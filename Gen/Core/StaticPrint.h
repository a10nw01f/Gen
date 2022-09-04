#pragma once

#ifdef _MSC_VER 

template<auto...>
struct StaticPrint
{
    constexpr static void Print()
    {
#pragma message(__FUNCSIG__)
    }
};

#elif __clang__

template<auto...>
[[deprecated]] constexpr void StaticPrintImpl(){}

template<auto... Vs>
struct StaticPrint
{
    constexpr static void Print()
    {
        StaticPrintImpl<Vs...>();
    }

};

#elif __GNUC__

template<auto...>
struct StaticPrint
{
    constexpr static void Print()
    {
#pragma GCC diagnostic push
#pragma GCC diagnostic warning "-Wunused"
    int unused = 0;
#pragma GCC diagnostic pop
    }

};

#endif