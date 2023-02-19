#pragma once
#include "std.h"
#include "StaticPrint.h"
#include "StaticString.h"

template<class T>
inline constexpr auto GetTypeName()
{
    #ifdef _MSC_VER

    constexpr auto start_sig_len = std::string_view("auto __cdecl GetTypeName<").length();
    constexpr auto end_sig_len = std::string_view(">(void)").length();
    constexpr auto sig_len = std::string_view(__FUNCSIG__).length();

    return std::string(__FUNCSIG__).substr(start_sig_len, sig_len - start_sig_len - end_sig_len);

    #elif __clang__

    constexpr auto start_sig_len = std::string_view("auto GetTypeName() [T = ").length();
    constexpr auto end_sig_len = std::string_view("]").length();
    constexpr auto sig_len = std::string_view(__PRETTY_FUNCTION__).length();

    return std::string(__PRETTY_FUNCTION__).substr(start_sig_len, sig_len - start_sig_len - end_sig_len);

    #elif __GNUC__

    constexpr auto start_sig_len = std::string_view("constexpr auto GetTypeName() [with T = ").length();
    constexpr auto end_sig_len = std::string_view("]").length();
    constexpr auto sig_len = std::string_view(__PRETTY_FUNCTION__).length();

    return std::string(__PRETTY_FUNCTION__).substr(start_sig_len, sig_len - start_sig_len - end_sig_len);

    #endif
}

namespace Gen
{
    template<class F, auto... Is>
    inline constexpr auto ForEachIndex(F&& func, std::index_sequence<Is...>)
    {
        return func(Is...);
    }

    template<class F, class... Ts>
    inline constexpr void ForEach(F&& func, Ts&&... args)
    {
        (func(args), ...);
    }

    template<StaticString file, StaticString content>
    inline constexpr void WriteFile()
    {
        StaticPrint<StaticString("WriteFile").Array(), file.Array(), content.Array()>::Print();
    }

    template<auto func>
    inline constexpr auto StringToArray()
    {
        constexpr auto size = func().length() + 1ul;
        StaticString<size> arr{};
        auto str = func();
        for(auto i = 0ul; i < size; ++i)
        {
            arr.m_Array[i] = str[i];
        }

        return arr;
    }

    template<auto func>
    constexpr auto StringToArray_v = StringToArray<func>();

    template<class T>
    struct TypeWrapper 
    {
        using Type = T;
        constexpr auto GetName() const { return GetTypeName<T>(); }
        T GetType() const;
    };

    template<class T>
    constexpr TypeWrapper<T> Tw = TypeWrapper<T>{};

    template<class T>
    inline constexpr std::string ValueToString(T value)
    {
        if (value == 0)
            return std::string("0");
        std::string result;
        bool negative = value < 0;
        if (negative)
        {
            value = -value;
        }

        while (value != 0)
        {
            result = (char)((value % 10) + '0') + result;
            value = value / 10;
        }

        if (negative)
        {
            result = '-' + result;
        }

        return result;
    }

    template<class... Ts>
    struct TypeList{};

    template<class T, class S>
    constexpr auto GetDataMemberType(T S::*)
    {
        return Tw<T>;
    }

    template<class T, class S>
    constexpr auto GetStructType(T S::*)
    {
        return Tw<S>;
    }

    template<class F, class... Ts>
    constexpr auto Expand(TypeList<Ts...>, F&& func)
    {
        return func(Ts{}...);
    }

    template<class... Ts, class... Us>
    constexpr auto Concat(TypeList<Ts...>,TypeList<Us...>){return TypeList<Ts...,Us...>();}

    template<class F>
    constexpr auto Filter(TypeList<>, F&& func){return TypeList<>();}

    template<class F, class T, class... Ts>
    constexpr auto Filter(TypeList<T, Ts...>, F func)
    {
        std::integral_constant<bool, func(T())> passed;
        if constexpr(passed)
        {
            return Concat(TypeList<T>(),Filter(TypeList<Ts...>(), func)); 
        }
        else
        {
            return Filter(TypeList<Ts...>(), func);
        }
    }

    template<class F, class... Ts>
    constexpr auto Transform(TypeList<Ts...>, F&& func)
    {
        return TypeList<decltype(func(Ts()))...>();
    }

    template<auto V>
    struct ValueWrapper
    {
        constexpr auto Value() const { return V; }
    };
}