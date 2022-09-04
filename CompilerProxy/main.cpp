#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <string_view>
#include <string.h>
#include <windows.h>
#include <thread>
#include <fstream>
#include <string>
#include <memory.h>
#include <filesystem>
#include <stdio.h>
#include <tchar.h>
#include <psapi.h>
#include <strsafe.h>
#include "ProcessContent.h"

std::wstring GetFileNameFromHandle(HANDLE hFile)
{
    BOOL bSuccess = FALSE;
    TCHAR pszFilename[MAX_PATH + 1];
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

DWORD __stdcall ReadDataFromExtProgram(HANDLE read_handle, const CompilerInfo& compiler_info)
{
    DWORD read;
    constexpr int kBufferSize = 1024 * 1024;
    static char buffer[kBufferSize];

    while(true)
    {
        auto success = ReadFile(read_handle, buffer, kBufferSize, &read, NULL);
        if (!success || read == 0) continue;

        auto content = std::string_view(buffer, buffer + read);

        ProcessContent(content, compiler_info);

        std::cerr << content << std::endl;
        if (!success) break;
    }
    return 0;
}

HRESULT RunCompilerProcess(const std::string& commandLine, const CompilerInfo& compiler_info)
{
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    SECURITY_ATTRIBUTES saAttr;

    ZeroMemory(&saAttr, sizeof(saAttr));
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    HANDLE read_handle = NULL;
    HANDLE write_handle = NULL;

    if (!CreatePipe(&read_handle, &write_handle, &saAttr, 0))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    if (!SetHandleInformation(read_handle, HANDLE_FLAG_INHERIT, 0))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.hStdError = write_handle;
    si.hStdOutput = write_handle;
    si.dwFlags |= STARTF_USESTDHANDLES;

    ZeroMemory(&pi, sizeof(pi));

    if (!CreateProcessA(NULL,
        (LPSTR)commandLine.c_str(),
        NULL,
        NULL,
        TRUE,
        0,
        NULL,
        NULL,
        &si,
        &pi))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    else
    {
        std::thread([read_handle, &compiler_info]{
            ReadDataFromExtProgram(read_handle, compiler_info);
        }).detach();
        WaitForSingleObject(pi.hProcess, INFINITE);

        Sleep(100);
        DWORD exitCode;
        GetExitCodeProcess(pi.hProcess, &exitCode);

        constexpr int kBufferSize = 512;
        char buffer[kBufferSize];
        if (GetEnvironmentVariableA("VS_UNICODE_OUTPUT", buffer, kBufferSize) != 0)
        {
            auto handle = strtol(buffer, nullptr, 10);
            DWORD bytes_read = 0;
            auto success = ReadFile((HANDLE)handle, buffer, kBufferSize, &bytes_read, nullptr);
                
            auto filepath = GetFileNameFromHandle((HANDLE)handle);
            auto length = std::wcstombs(buffer, filepath.c_str(), kBufferSize);
            auto content = ReadFileToStringW(buffer);
            ProcessContent(content, compiler_info);
        }

        exit(exitCode);
    }
    return S_OK;
}

auto GetExePath()
{
    char buffer[1024];
    GetModuleFileNameA(GetModuleHandleA(NULL), buffer, sizeof(buffer));

    return std::filesystem::path(buffer);
}

auto FindOriginalPath(std::filesystem::path path)
{
    std::ifstream config(path.parent_path().string() + "\\config.txt");
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
    auto cmd = FindOriginalPath(exe_path);
    if (cmd.empty())
    {
        return 0;
    }

    auto compiler_info = GetCompilerInfo(exe_path.filename().string());
    if (!compiler_info)
    {
        return 0;
    }

    if (!cmd.empty())
    {
        
        for (int i = 1; i < argc; ++i)
        {
            cmd += std::string(" \"") + argv[i] + '"';
        }
        RunCompilerProcess(cmd, *compiler_info);
    }

    return 0;
}
