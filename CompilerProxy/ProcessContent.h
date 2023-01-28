#pragma once
#include <windows.h>

#include <string>
#include <string_view>
#include <vector>
#include <fstream>
#include <sstream>
#include <chrono>
#include <stdio.h>
#include "RunProcess.h"

std::vector<std::string> ExtractStrings(std::string_view str, std::string_view delimiter, int advance)
{
    std::vector<std::string> outputs;
    size_t offset = 0;
    while (true)
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
            ptr += advance;
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

std::string ReadFileToStringW(const char* file_path)
{
    auto str = ReadFileToString(file_path);
    auto ptr = (wchar_t*)str.c_str();

    return std::string(ptr, ptr + str.length() / 2);
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

struct CompilerInfo 
{
    const char* m_ExeName;
    const char* m_StartPos;
    const char* m_EndPos;
    const char* m_Delimiter;
    int m_Advance;
};

template<class T, int N>
constexpr int ArraySize(T(&)[N])
{
    return N;
}

CompilerInfo g_CompilerInfos[3] = { {
        "cl.exe",
        "void __cdecl StaticPrint<class std::array<int",
        "::Print",
        "{int{",
        1
    },{
        "gcc.exe",
        "static constexpr void StaticPrint<<anonymous> >::Print() [with auto ...<anonymous> =",
        "}]",
        ">::_Type{",
        2
    }, {
        "clang.exe",
        "warning: 'StaticPrintImpl<{{",
        "' is deprecated [-Wdeprecated-declarations]",
        "{{",
        2
    } };

const CompilerInfo* GetCompilerInfo(const std::string& exe_file_name)
{
    for (int i = 0; i < ArraySize(g_CompilerInfos); ++i)
    {
        if (exe_file_name == g_CompilerInfos[i].m_ExeName)
        {
            return &g_CompilerInfos[i];
        }
    }

    return nullptr;
}

template<class F>
void SplitStr(std::string_view str, const char* startPosStr, const char* endPosStr, F&& func)
{
    size_t pos = 0;
    while (true)
    {
        auto startPos = str.find(startPosStr, pos);
        if (startPos == std::string::npos)
        {
            return;
        }
        auto endPos = str.find(endPosStr, startPos);
        func(str.substr(startPos, endPos - startPos));
        pos = endPos;
    }
}

template<class T>
class ItrPair {
    T m_Begin, m_End;
public:
    ItrPair(T begin, T end) : m_Begin(begin), m_End(end) {}
    auto begin() { return m_Begin; }
    auto end() { return m_End; }
};

int OutputCppFromCpp2(const std::string& output_file_path, const std::string& cpp2src, ItrPair<std::string*> flags, const std::function<void(const char*)>& output)
{
    auto output_error = [&](const char* err) {
        output(err);
        return -1;
    };

    char buffer[L_tmpnam_s];
    if (auto err = tmpnam_s(buffer, sizeof(buffer)); err) 
    {
        return output_error("tmpnam_s failed");
    }

    auto input_file_path = buffer + std::string(".cpp2");
    {
        std::ofstream file(input_file_path);
        if(!file.is_open())
        {
            return output_error("failed to open temporary file");
        }
        file << cpp2src;
    }

    std::vector<const char*> command;
    command.emplace_back("cppfront");
    for(auto& flag : flags)
    {
        command.emplace_back(flag.c_str());
    }
    command.emplace_back("-o");
    command.emplace_back(output_file_path.c_str());
    command.emplace_back(input_file_path.c_str());
    command.emplace_back(nullptr);

    int exit_code = RunProcess(command.data(), output).value_or(-1);

    std::filesystem::remove(input_file_path);
    return exit_code;
}

int ProcessContent(const std::string_view& content, const CompilerInfo& compiler_info, const std::function<void(const char*)>& output)
{
    int status = 0;
    SplitStr(content, compiler_info.m_StartPos, compiler_info.m_EndPos, [&compiler_info, &output, &status](auto section)
    {
        auto strs = ExtractStrings(section, compiler_info.m_Delimiter, compiler_info.m_Advance);
        if (!strs.empty())
        {
            if (strs[0] == "WriteFile")
            {
                std::ofstream file(strs[1], std::ios_base::trunc);
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
            else if(strs[0] == "Cpp2")
            {
                status |= OutputCppFromCpp2(strs[1], strs[2], ItrPair(&strs[3], strs.data() + strs.size()), output);
            }
        }
    });

    return status;
}
