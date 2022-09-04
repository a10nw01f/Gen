#pragma once

#include "std.h"

namespace Gen
{
    struct FixedString
    {
        constexpr static int Size = 260;
        char m_Data[Size] = {};
        constexpr FixedString() = default;
        constexpr FixedString(const char* str)
        {
            for (int i = 0; i < Size; i++)
            {
                m_Data[i] = str[i];
                if (str[i] == '\0')
                {
                    break;
                }
            }
        }
        constexpr auto data() const { return m_Data; }
        constexpr auto& operator[](int i) const { return m_Data[i]; }
        constexpr auto& operator[](int i) { return m_Data[i]; }
        constexpr auto String() const { return std::string(m_Data); }
    };

    constexpr FixedString StringToFixedStr(const char* str)
    {
        FixedString arr{};
        for (auto i = 0ul; i < FixedString::Size; i++)
        {
            arr[i] = str[i];
            if (str[i] == '\0')
            {
                break;
            }
        }

        return arr;
    }
}