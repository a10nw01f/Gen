#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <fstream>
#include <sstream>
#include <chrono>
#include <stdio.h>
#include "RunProcess.h"

std::string ReadFileToString(const char* file_path)
{
    std::ifstream file(file_path);
    std::string str(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());

    return str;
}

std::vector<std::string> ExtractStrings(std::string_view input)
{
    std::vector<std::string> outputs;

    constexpr auto start = std::string_view("StartStaticPrint");
    auto offset = input.find(start);
    if (offset == std::string::npos)
        return outputs;

    // prevent clang from processing the same print twice
    if (input.contains("note:")) {
        return outputs;
    }

    outputs.emplace_back();
    offset += start.length() + 3;
    auto parse_ws = [&] {
        while (input[offset] == ' ' && offset < input.size()) {
            offset++;
        }
    };

    auto parse_comma = [&] {
        if (input[offset] == ',' && offset < input.size()) {
            offset++;
        }
    };

    auto parse_int = [&] () -> int {
        auto ptr = &input[offset];
        auto c = strtol(ptr, const_cast<char**>(&ptr), 10);
        auto char_count = ptr - &input[offset];
        if (char_count == 0) {
            return -1;
        }
        offset += char_count;
        return c;
    };

    while (true)
    {
        parse_ws();
        parse_comma();
        parse_ws();
        auto v = parse_int();
        if (v == -1) {
            break;
        }
        else if (v == 0) {
            outputs.emplace_back();
        }
        else {
            outputs.back().push_back(static_cast<char>(v));
        }
    }

    if (outputs.back().empty()) {
        outputs.pop_back();
    }
    return outputs;
}

template<class F>
void SplitStr(std::string_view str, F&& func)
{
    size_t pos = 0;
    while (true)
    {
        auto endPos = str.find('\n', pos);
        if (endPos == std::string::npos)
        {
            return;
        }
        func(str.substr(pos, endPos - pos));
        pos = endPos + 1;
    }
}

int ProcessContent(const std::string_view& content, const std::function<void(std::string_view)>& output)
{
    int status = 0;
    SplitStr(content, [&output, &status](auto section)
    {
        auto strs = ExtractStrings(section);
        while (!strs.empty())
        {
            if (strs[0] == "WriteFile")
            {
                std::ofstream file(strs[1], std::ios_base::trunc);
                file << strs[2];
                strs.erase(strs.begin(), strs.begin() + 3);
            }
            else if (strs[0] == "System")
            {
                system(strs[1].c_str());
                strs.erase(strs.begin(), strs.begin() + 2);
            }
            else {
                break;
            }
        }
    });

    return status;
}
