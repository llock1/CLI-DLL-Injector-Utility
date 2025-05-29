#pragma once
#include <Windows.h>
#include <vector>
#include <memory>
#include <string>

class ProcessesList {
public:
	struct Process {
		DWORD pid;
		std::string name;

		std::string GetFormattedName() {
			return name + "  (PID: " + std::to_string(pid) + ")";
		}

		Process(DWORD pid, const char* name) {
			this->pid = pid;
			this->name = name;
		};
	};       


	std::vector<Process> processes;
	void GetProcesses();
};
inline std::unique_ptr<ProcessesList> g_Plist;