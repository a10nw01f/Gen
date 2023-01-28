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

    constexpr void Format(std::string& output, const std::string_view& format, std::initializer_list<std::string_view> args)
    {
        std::size_t offset = 0;
        while (true)
        {
            auto start = format.find("${{", offset);
            if (start == std::string::npos)
            {
                break;
            }
            auto num = PraseInt(format.data() + start + 3);
            auto end = format.find("}}", start);
            output.append(format.substr(offset, start - offset));
            output.append(std::data(args)[num]);
            offset = end + 2;
        }

        output.append(format.substr(offset));
    }

    constexpr int kStringReserveSize = 4096;

    inline constexpr std::string ReserveString(int reserve_size = kStringReserveSize)
    {
        std::string str;
        str.reserve(reserve_size);
        return str;
    }
    
    inline constexpr std::string Format(const std::string_view& format, std::initializer_list<std::string_view> args)
    {
        std::string output;
        Format(output, format, args);
        return output;
    }
}