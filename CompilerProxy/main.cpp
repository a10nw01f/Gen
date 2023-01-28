#include <windows.h>
#include <iostream>
#include <string_view>
#include <string.h>

#include <fstream>
#include <filesystem>
#include <stdio.h>
#include "RunProcess.h"
#include "ProcessContent.h"

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#include <strsafe.h>
#include <tchar.h>

std::wstring GetFileNameFromHandle(HANDLE hFile)
{
    BOOL bSuccess = FALSE;
    TCHAR pszFilename[MAX_PATH + 1];
    pszFilename[0] = 0;
    HANDLE hFileMap;

    // Get the file size.
    DWORD dwFileSizeHi = 0;
    DWORD dwFileSizeLo = GetFileSize(hFile, &dwFileSizeHi);

    if (dwFileSizeLo == 0 && dwFileSizeHi == 0)
    {
        return std::wstring();
    }

    // Create a file mapping object.
    hFileMap = CreateFileMapping(hFile,
        NULL,
        PAGE_READONLY,
        0,
        1,
        NULL);

    if (hFileMap)
    {
        // Create a file mapping to get the file name.
        void* pMem = MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 1);

        if (pMem)
        {
            if (GetMappedFileName(GetCurrentProcess(),
                pMem,
                pszFilename,
                MAX_PATH))
            {

                // Translate path with device name to drive letters.
                constexpr int kBufferSize = 512;
                TCHAR szTemp[kBufferSize];
                szTemp[0] = '\0';

                if (GetLogicalDriveStrings(kBufferSize - 1, szTemp))
                {
                    TCHAR szName[MAX_PATH];
                    TCHAR szDrive[3] = TEXT(" :");
                    BOOL bFound = FALSE;
                    TCHAR* p = szTemp;

                    do
                    {
                        // Copy the drive letter to the template string
                        *szDrive = *p;

                        // Look up each device name
                        if (QueryDosDevice(szDrive, szName, MAX_PATH))
                        {
                            size_t uNameLen = _tcslen(szName);

                            if (uNameLen < MAX_PATH)
                            {
                                bFound = _tcsnicmp(pszFilename, szName, uNameLen) == 0
                                    && *(pszFilename + uNameLen) == _T('\\');

                                if (bFound)
                                {
                                    // Reconstruct pszFilename using szTempFile
                                    // Replace device path with DOS path
                                    TCHAR szTempFile[MAX_PATH];
                                    StringCchPrintf(szTempFile,
                                        MAX_PATH,
                                        TEXT("%s%s"),
                                        szDrive,
                                        pszFilename + uNameLen);
                                    StringCchCopyN(pszFilename, MAX_PATH + 1, szTempFile, _tcslen(szTempFile));
                                }
                            }
                        }

                        // Go to the next NULL character.
                        while (*p++);
                    } while (!bFound && *p); // end of string
                }
            }
            bSuccess = TRUE;
            UnmapViewOfFile(pMem);
        }

        CloseHandle(hFileMap);
    }

    return std::wstring(pszFilename);
}
#endif

void RunCompilerProcess(const char* command[], const CompilerInfo& compiler_info)
{
    int exit_status = 0;

    std::stringstream compiler_output;
    auto result = RunProcess(command, [&](const char* str){
        auto content = std::string_view(str);
        std::cout << content;
        compiler_output << content;
    });

    if(!result.has_value())
    {
        exit(-1);
    }

    exit_status |= ProcessContent(compiler_output.str(), compiler_info, [](const char* str) {
        std::cout << str;
    });
    
    exit_status |= *result;

#ifdef _WIN32
    char buffer[MAX_PATH];
    if (GetEnvironmentVariableA("VS_UNICODE_OUTPUT", buffer, MAX_PATH) != 0)
    {
        auto handle = strtoll(buffer, nullptr, 10);
        DWORD bytes_read = 0;
        auto success = ReadFile((HANDLE)handle, buffer, MAX_PATH, &bytes_read, nullptr);
            
        auto filepath = GetFileNameFromHandle((HANDLE)handle);
        size_t length;
        auto err = wcstombs_s(&length, buffer, filepath.c_str(), sizeof(buffer));
        if (err) {
            std::cout << "wcstombs_s failed";
            exit(-1);
        }
        auto content = ReadFileToStringW(buffer);
        
        std::ofstream output(buffer, std::ios_base::app | std::ios::binary);
        
        exit_status |= ProcessContent(content, compiler_info, [&output](const char* str){
            for(; *str; str++){
                output.put(*str);
                output.put(0);
            }
        });
    }
#endif

    exit(exit_status);
    return;
}

std::filesystem::path GetExePath()
{
#ifdef _WIN32
    wchar_t path[MAX_PATH] = { 0 };
    GetModuleFileNameW(NULL, path, MAX_PATH);
    return path;
#else
    char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    return std::string(result, (count > 0) ? count : 0);
#endif
}

auto FindOriginalPath(std::filesystem::path path)
{
    std::ifstream config(path.parent_path().append("config.txt").string());
    std::string line;
    while (std::getline(config, line))
    {
        if (line == path.filename())
        {
            std::getline(config, line);
            return line;
        }
    }

    path.replace_extension(".original.exe");
    if(std::filesystem::exists(path))
    {
        return path.string();
    }

    return std::string();
}

int main(int argc, char** argv)
{
    auto exe_path = GetExePath();
    auto compiler_path = FindOriginalPath(exe_path);
    if (compiler_path.empty())
    {
        return 0;
    }

    auto compiler_info = GetCompilerInfo(exe_path.filename().string());
    if (!compiler_info)
    {
        return 0;
    }

    std::vector<const char*> command = {compiler_path.c_str()};
    for (int i = 1; i < argc; ++i)
    {
        command.emplace_back(argv[i]);
    }
    command.emplace_back(nullptr);
    RunCompilerProcess(command.data(), *compiler_info);

    return 0;
}
