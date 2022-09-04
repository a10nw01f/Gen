#pragma once
#include "std.h"

namespace Gen
{
    inline constexpr int PraseInt(const char* str)
    {
        int result = 0;
        while (true)
        {
            auto c = *str;
            if (c < '0' || c > '9')
            {
                return result;
            }
            result = (result * 10) + (c - '0');
            str++;
        }
    }

    inline constexpr std::string Format(std::string formatArg, std::vector<std::string> args)
    {
        auto format = formatArg;
        std::size_t offset = 0;
        while (true)
        {
            auto start = format.find("${{", offset);
            if (start == std::string::npos)
            {
                return format;
            }
            auto num = PraseInt(format.data() + start + 3);
            auto end = format.find("}}", start);
            format = format.substr(0, start) + args[num] + format.substr(end + 2);
            offset = start + args[num].length();
        }
    }
}