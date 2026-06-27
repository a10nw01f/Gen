#pragma once

#include <utility>
#include <string_view>
#include <string>

namespace gen {

#ifdef _MSC_VER

template<auto...>
struct StaticPrint
{
    constexpr static void print()
    {
#pragma message(__FUNCSIG__)
    }
};

#elif __clang__ || __GNUC__

struct StaticPrint
{
    template<auto... Vs>
    [[deprecated]] constexpr static void print(){}
};

#endif


    struct StartStaticPrint{};

    inline constexpr int parse_int(const char* str)
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

    template<int Capacity = 4096>
    struct StringWriter {
        char data[Capacity] = {};
        int size = 0;

        constexpr auto& append(std::string_view str) {
            for (int i = 0; i < str.size(); ++i) {
                data[size++] = str[i];
            }
            return *this;
        }

        constexpr auto& end_string() {
            data[size++] = 0;
            return *this;
        }

        constexpr StringWriter& add_string(std::string_view str) {
            append(str);
            end_string();
            return *this;
        }

        constexpr void format(const std::string_view& format, std::initializer_list<std::string_view> args)
        {
            std::size_t offset = 0;
            while (true)
            {
                auto start = format.find("${", offset);
                if (start == std::string::npos)
                {
                    break;
                }
                auto num = parse_int(format.data() + start + 2);
                auto end = format.find("}", start);
                append(format.substr(offset, start - offset));
                append(std::data(args)[num]);
                offset = end + 1;
            }

            append(format.substr(offset));
        }
    };

    template<auto writer, auto eval = []{}>
    constexpr void static_print() {
        []<auto... Is>(std::index_sequence<Is...>){
            constexpr auto start = StartStaticPrint{};
#ifdef _MSC_VER
            StaticPrint<start, int(writer.data[Is])...>::print();
#else
            StaticPrint::print<start, int(writer.data[Is])...>();
#endif
        }(std::make_index_sequence<writer.size>());
    }
}