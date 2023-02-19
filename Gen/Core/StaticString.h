#pragma once
#include "std.h"

namespace Gen
{
    template<std::size_t N>
    struct StaticString
    {
        constexpr static int Size = N;
        char m_Array[N]{};

        constexpr StaticString(char const(&arg)[N])
        {
            for(auto i = 0ul; i < N; ++i)
            {
                m_Array[i] = arg[i];
            }
        };

        constexpr int GetSize() const { return N; }

        constexpr StaticString() = default;

        constexpr auto Array() const {
            std::array<int, N - 1> arr{};
            for(auto i = 0ul; i < N - 1; ++i)
            {
                arr[i] = m_Array[i];
            }
            return arr;
        }

        constexpr auto Data() {
            return m_Array;
        }

        constexpr auto String() const {
            return std::string(m_Array);
        }
    };
}