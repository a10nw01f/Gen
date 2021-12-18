#include <windows.h>
#include <string>
#include <psapi.h>
#include <fstream>
#include <array>
#include <vector>
#include "detour/detours.h"

bool EndsWith(std::wstring_view const& fullString, std::wstring_view const& ending) {
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
    }
    else {
        return false;
    }
}

std::vector<std::string> ExtractStrings(std::string_view str)
{
    std::string_view delimiter = "{char{";
    std::vector<std::string> outputs;
    size_t offset = 0;
    while(true)
    {
        auto idx = str.find(delimiter, offset);
        if (idx == std::string::npos)
            break;
        auto& output = outputs.emplace_back();
        auto ptr = &str[idx + delimiter.size()];
        while (true)
        {
            auto c = strtol(ptr, const_cast<char**>(&ptr), 10);
            if (!c)
                break;
            output += (char)c;
            ptr++;
        }
        offset = ptr - str.data();
    }

    return outputs;
}

std::string ReadFileToString(const char* file_path)
{
    std::ifstream file(file_path);
    std::string str(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());

    return str;
}

void Inject(const std::vector<std::string>& values)
{
    auto& input_file = values[1];
    auto& output_file = values[2];

    auto file_content = ReadFileToString(input_file.c_str());
    for (int i = 3; i < values.size(); i += 2)
    {
        auto& marker = values[i];
        auto& value = values[i + 1];
        auto pos = file_content.find(marker);
        if (pos != std::string::npos)
        {
            file_content.insert(pos + marker.length() + 1, value);
        }
    }

    std::ofstream file(output_file.c_str(), std::ios_base::trunc);
    file << file_content;
}

BOOL (*g_OriginalWriteFile)(
    HANDLE hFile,
    LPCVOID lpBuffer,
    DWORD nNumberOfBytesToWrite,
    LPDWORD lpNumberOfBytesWritten,
    LPOVERLAPPED lpOverlapped);

BOOL WINAPI ModifiedWriteFile(
    HANDLE hFile,
    LPCVOID lpBuffer,
    DWORD nNumberOfBytesToWrite,
    LPDWORD lpNumberOfBytesWritten,
    LPOVERLAPPED lpOverlapped
)
{
    TCHAR path[MAX_PATH];
    auto dwRet = GetFinalPathNameByHandle(hFile, path, MAX_PATH, VOLUME_NAME_NT);
    if (dwRet < MAX_PATH)
    {
        //std::wstring str(path);
        if (EndsWith(std::wstring_view(path), std::wstring_view(L"_GENERATE.log")))
        {
            std::string_view content((char*)lpBuffer, (char*)lpBuffer + nNumberOfBytesToWrite);
            if (content.find("void __cdecl StaticPrint<class std::array<char") != std::string::npos)
            {
                auto strs = ExtractStrings(content);
                if (!strs.empty())
                {
                    if (strs[0] == "WriteFile")
                    {
                        std::ofstream file(strs[1].c_str(), std::ios_base::trunc);
                        file << strs[2];
                    }
                    else if (strs[0] == "System")
                    {
                        system(strs[1].c_str());
                    }
                    else if (strs[0] == "Inject")
                    {
                        Inject(strs);
                    }
                }
            }
        }
    }

    return g_OriginalWriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
}

void SetHook()
{
    auto module = GetModuleHandleA("kernelbase.dll");
    auto write_file_func = GetProcAddress(module, "WriteFile");
    
    DetourRestoreAfterWith();
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&(PVOID&)write_file_func, ModifiedWriteFile);
    DetourTransactionCommit();

    g_OriginalWriteFile = (decltype(g_OriginalWriteFile))write_file_func;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    HANDLE file;
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        SetHook();
        break;
    }
    return TRUE;
}

