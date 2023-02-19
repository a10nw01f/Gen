#pragma once

#include "std.h"

namespace Gen
{
    template<int I = 128>
    struct FixedStringImpl
    {
        constexpr static int Size = I;
        char m_Data[Size] = {};
        constexpr FixedStringImpl() = default;
        constexpr FixedStringImpl(const char* str)
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

        constexpr auto& operator[](int i) const { return m_Data[i]; }
        constexpr auto& operator[](int i) { return m_Data[i]; }
        constexpr auto String() const { return std::string(m_Data); }
        constexpr auto Data() const { return m_Data; }
    };

    using FixedString = FixedStringImpl<>;

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