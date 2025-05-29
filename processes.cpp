#include "processes.h"
#include <Windows.h>
#include <TlHelp32.h> 
#include <iostream>

void ProcessesList::GetProcesses() {
	processes.clear();
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (Process32First(snapshot, &entry) == TRUE) {
		do {
			if (entry.th32ProcessID != 0) {
				processes.emplace_back(
					ProcessesList::Process(entry.th32ProcessID, entry.szExeFile)
				);
			}
		} while (Process32Next(snapshot, &entry) == TRUE);
	}

	if (CloseHandle(snapshot) == 0) {
		std::cout << "Failed to close snapshot" << std::endl;
	}
}
