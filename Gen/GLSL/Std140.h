#pragma once

#include <utility>
#include "../Core/Utils.h"

namespace Gen
{
    template<class T, int Align>
    class alignas(Align) AlignWrapper
    {
    private:
        T m_Value;
    public:
        template<class... Ts>
        AlignWrapper(Ts&&... args) :m_Value(std::forward<Ts>(args)...) {}
        operator T& () { return m_Value; }
        operator const T& () const { return m_Value; }
    };

    template<class T, int I>
    struct Vec
    {
    private:
        void Assert()
        {
            static_assert(std::is_fundamental_v<T> && (std::is_integral_v<T> || std::is_floating_point_v<T>), "");
            static_assert(I >= 2 && I <= 4, "");
        }
    public:
        T m_Data[I];
    };

    template<class T>
    constexpr int GetAlignment(Gen::TypeWrapper<T>)
    {
        if constexpr (std::is_fundamental_v<T> && (std::is_integral_v<T> || std::is_floating_point_v<T>))
        {
            return alignof(T) < 4 ? 4 : alignof(T);
        }
        else
        {
            static_assert(std::is_same_v<T, struct Err>, "");
            return 0;
        }
    }

    template<class T, int I>
    constexpr int GetAlignment(Gen::TypeWrapper<Vec<T, I>>)
    {
        return GetAlignment(Gen::TypeWrapper<T>{}) * (I == 3 ? 4 : I);
    }

    template<class T>
    using Std140 = AlignWrapper<T, GetAlignment(Gen::TypeWrapper<T>{})>;
}