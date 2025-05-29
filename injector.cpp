#include "injector.h"
#include <Windows.h>
#include <iostream>

#define CREATE_THREAD_ACCESS (PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ)

namespace Injector {

	typedef HMODULE(WINAPI* loadlib)(LPCSTR);
	bool Inject(DWORD pid, const char* DLL_PATH) {

		loadlib LoadLibrary = (loadlib)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");

		if (!LoadLibrary) {
			std::cout << "Failed to get Load Library Function" << std::endl;
			return false;
		}

		HANDLE hProcess = OpenProcess(CREATE_THREAD_ACCESS, 0, pid);
		if (!hProcess) {
			std::cout << "Failed to open process" << std::endl;
			return false;
		}

		LPVOID dllStartAddress = VirtualAllocEx(hProcess, NULL, strlen(DLL_PATH) + 1, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE);
		if (!dllStartAddress) {
			CloseHandle(hProcess);
			return false;
		}

		if (!WriteProcessMemory(hProcess, dllStartAddress, DLL_PATH, strlen(DLL_PATH) + 1, NULL)) {
			std::cout << "Failed to write process memory" << std::endl;
			VirtualFreeEx(hProcess, dllStartAddress, strlen(DLL_PATH) + 1, MEM_RELEASE);
			CloseHandle(hProcess);
			return false;
		}

		HANDLE thread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibrary, dllStartAddress, 0, NULL);
		if (!thread) {
			VirtualFreeEx(hProcess, dllStartAddress, strlen(DLL_PATH) + 1, MEM_RELEASE);
			CloseHandle(hProcess);
			return false;
		}

		WaitForSingleObject(thread, INFINITE);

		VirtualFreeEx(hProcess, dllStartAddress, strlen(DLL_PATH) + 1, MEM_RELEASE);
		CloseHandle(thread);
		CloseHandle(hProcess);


		return true;
	}
}