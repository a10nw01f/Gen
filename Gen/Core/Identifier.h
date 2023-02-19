#pragma once

#include "std.h"
#include "StaticString.h"
#include "Utils.h"

namespace Gen
{
    inline constexpr bool IsDigit(const char c)
    {
        return (c >= '0' && c <= '9');
    }

    inline constexpr bool IsValidIndentifierChar(char c)
    {
        return IsDigit(c) || c == '_' || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
    }

    inline constexpr bool IsIdentifier(std::string_view str)
    {
        if (IsDigit(str[0]))
            return false;

        for (auto c : str)
        {
            if (!IsValidIndentifierChar(c))
                return false;
        }

        return true;
    }

    template<StaticString ident>
    inline constexpr bool IsIdentifier()
    {
        return !IsDigit(ident.m_Array[0]) && ForEachIndex([](auto... is) {
                return (... && IsValidIndentifierChar(ident.m_Array[is]));
            }, std::make_index_sequence<decltype(ident)::Size - 1>());
    }

    template<StaticString str>
    concept Identifier = IsIdentifier<str>();
}