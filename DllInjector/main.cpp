#include <string>
#include <Windows.h>
#include <tlhelp32.h>
#include <iostream>

namespace
{
    DWORD FindProcessId(const std::wstring& process_name)
    {
        PROCESSENTRY32 process_entry;
        process_entry.dwSize = sizeof(process_entry);

        HANDLE processes_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
        if (processes_snapshot == INVALID_HANDLE_VALUE) {
            return 0;
        }

        Process32First(processes_snapshot, &process_entry);
        if (!process_name.compare(process_entry.szExeFile))
        {
            CloseHandle(processes_snapshot);
            return process_entry.th32ProcessID;
        }

        while (Process32Next(processes_snapshot, &process_entry))
        {
            if (!process_name.compare(process_entry.szExeFile))
            {
                CloseHandle(processes_snapshot);
                return process_entry.th32ProcessID;
            }
        }

        CloseHandle(processes_snapshot);
        return 0;
    }

    std::wstring ToWString(const char* str)
    {
        auto len = strlen(str);
        std::wstring wstr;
        wstr.resize(len);
        for (int i = 0; i < len; ++i)
        {
            wstr[i] = str[i];
        }

        return wstr;
    }
}

void InjectDLL(const char* process_name, const char* dll_path)
{
    auto dll_path_len = strlen(dll_path);
    auto pid = FindProcessId(ToWString(process_name));

    auto process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    auto addr = (LPVOID)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryA");
    auto arg = (LPVOID)VirtualAllocEx(process, NULL, dll_path_len, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    
    WriteProcessMemory(process, arg, dll_path, dll_path_len, NULL);
    CreateRemoteThread(process, NULL, 0, (LPTHREAD_START_ROUTINE)addr, arg, NULL, NULL);
}

int main(int argc, const char** argv)
{
    if (argc != 3)
    {
        std::cout << "usage DllInjector.exe <process_name> <dll_path>\n";
        return -1;
    }

    InjectDLL(argv[1], argv[2]);

    return 0;
}