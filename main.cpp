
#include <functional>  
#include <iostream> 
#include <memory>    
#include <string>  

#include "ftxui/component/captured_mouse.hpp"  
#include "ftxui/component/component.hpp"  
#include "ftxui/component/component_base.hpp"      
#include "ftxui/component/component_options.hpp"   
#include "ftxui/component/screen_interactive.hpp"  
#include "ftxui/dom/elements.hpp"  
#include "ftxui/screen/color.hpp" 
#include <vector>
#include <windows.h>
#include <memory>
#include "processes.h"
#include <string>
#include "injector.h"
#include <sys/stat.h>

using namespace ftxui;


MenuEntryOption Colored(ftxui::Color c) {
    MenuEntryOption option;
    option.transform = [c](EntryState state) {
        state.label = (state.active ? "> " : "  ") + state.label;
        Element e = text(state.label) | color(c);
        if (state.focused)
            e = e | inverted;
        if (state.active)
            e = e | bold;
        return e;
        };
    return option;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Usage: inject <dll_path>" << std::endl;
        return 0;
    }

    struct stat sb;

    const char* dll_path = argv[1];

    if (stat(dll_path, &sb) == 0 && !(sb.st_mode & S_IFDIR))
        std::cout << "DLL Path is valid";
    else {
        std::cout << "Cannot Find Provided DLL, Is the path definitely correct?";
        return 0;
    }
        

    g_Plist = std::make_unique<ProcessesList>();
    g_Plist->GetProcesses();

    std::vector<ProcessesList::Process> full_processes = g_Plist->processes;
    std::vector<ProcessesList::Process> filtered_processes = full_processes;
    int selected = 3;
    int value = 50;
    std::string message = "";

    auto buttons = Container::Horizontal({
        Button(
            "Inject", [&] {
                message = std::format("Injecting Into {}", filtered_processes[selected].name);
                Injector::Inject(filtered_processes[selected].pid, dll_path);
            }, ButtonOption::Animated(Color::Red)),
        });

    std::vector<Component> entries;

    auto update_entries = [&] {
        entries.clear();
        for (auto& p : filtered_processes) {
            entries.push_back(MenuEntry(p.GetFormattedName()));
        }
    };

    auto menu = Container::Vertical(entries, &selected);

    std::string search_query;
    auto search_input = Input(&search_query, "Search...");


    auto main_container = Container::Vertical({
        search_input,
        menu,
        buttons
    });

    auto on_search_change = [&] {
        g_Plist->GetProcesses();
        full_processes = g_Plist->processes;
        filtered_processes.clear();
        for (const auto& p : full_processes) {
            if (p.name.find(search_query) != std::string::npos) {
                filtered_processes.push_back(p);
            }
        }

        if (search_query.empty()) {
            filtered_processes = full_processes;
        }

        update_entries();

        menu->DetachAllChildren();
        for (auto& entry : entries) {
            menu->Add(entry);
        }
        
        if (selected >= entries.size()) {
            selected = 0;
        }

    };

    auto component = Renderer(main_container, [&] {
        on_search_change();
        return vbox({
            search_input->Render(),
            separator(),
            hbox(text("selected = "), text(filtered_processes[selected].GetFormattedName())),
            separator(),
            menu->Render() | frame | size(HEIGHT, LESS_THAN, 10),
            separator(),
            buttons->Render() | frame | center | size(HEIGHT, LESS_THAN, 10),
            text(message) | center,
            }) | border;
        });

    auto screen = ScreenInteractive::TerminalOutput();
    screen.Loop(component);
    return 0;
}
