module;

#include <array>
#include <string>
#include <ranges>
#include <algorithm>
#include <utility>
#include <source_location>
#include <vector>
#include <windows.h>

const static int kMaxPath = MAX_PATH;

export module Gen.Utils;

template<class T, int N>
constexpr auto ToArray(T(&arg)[N])
{
    std::array<std::decay_t<T>, N - 1> arr{};
    std::copy(arg, arg + N - 1, std::begin(arr));
    return arr;
}

constexpr int PraseInt(const char* str)
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

export template<auto...>
struct StaticPrint
{
    constexpr static void Print()
    {
#pragma message(__FUNCSIG__)
    }
};

export 
{
    namespace Gen
    {
        template<std::size_t N>
        struct StaticString
        {
            constexpr static int Size = N;
            char m_Array[N]{};

            constexpr StaticString(char const(&arg)[N])
            {
                std::ranges::copy(arg, m_Array);
            };

            constexpr int GetSize() const { return N; }

            constexpr StaticString() = default;

            constexpr auto Array() const {
                return ::ToArray(m_Array);
            }

            constexpr auto String() const {
                return std::string(m_Array);
            }
        };

        template<StaticString file, StaticString content>
        constexpr void WriteModule()
        {
            StaticPrint<StaticString("WriteFile").Array(), file.Array(), content.Array()>::Print();
        };

        template<auto func>
        constexpr auto StringToArray()
        {
            constexpr int x = (int)func().length() + 1;
            StaticString<x> arr{};
            auto str = func();
            std::copy(std::begin(str), std::end(str), std::begin(arr.m_Array));

            return arr;
        }

        template<auto func>
        constexpr auto StringToArray_v = StringToArray<func>();

        template<class T>
        constexpr std::string GetTypeName()
        {
            constexpr auto start_sig_len = std::string_view("class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> > __cdecl Gen::GetTypeName<").length();
            constexpr auto end_sig_len = std::string_view(">(void)").length();
            constexpr auto sig_len = std::string_view(__FUNCSIG__).length();

            return std::string(__FUNCSIG__).substr(start_sig_len, sig_len - start_sig_len - end_sig_len);
        }

        template<class F, class... Ts>
        constexpr void ForEach(F&& func, Ts&&... args)
        {
            (func(args), ...);
        }

        struct FixedString
        {
            constexpr static int Size = kMaxPath;
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
            for (int i = 0; i < FixedString::Size; i++)
            {
                arr[i] = str[i];
                if (str[i] == '\0')
                {
                    break;
                }
            }

            return arr;
        }

        constexpr FixedString GetCurrentDir(const char* path)
        {
            std::string str(path);
            return StringToFixedStr(str.substr(0, str.find_last_of('\\')).c_str());
        }

        template<class T>
        struct TypeWrapper 
        {
            using Type = T;
            constexpr auto GetName() const { return GetTypeName<T>(); }
        };

        template<class T>
        constexpr TypeWrapper<T> Tw = TypeWrapper<T>{};

        constexpr std::string Format(std::string format, std::vector<std::string> args)
        {
            int offset = 0;
            while (true)
            {
                auto start = format.find("${{", offset);
                if (start == std::string::npos)
                    return format;
                auto num = PraseInt(format.data() + start + 3);
                auto end = format.find("}}", start);
                format = format.substr(0, start) + args[num] + format.substr(end + 2);
                offset = start + args[num].length();
            }
        }

        template<class T>
        constexpr std::string ValueToString(T value)
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

        struct NameAndFolder
        {
            FixedString m_Name;
            FixedString m_FolderPath;
            constexpr NameAndFolder(const char* name, const char* folder_path = GetCurrentDir(std::source_location::current().file_name()).data()) :
                m_Name(StringToFixedStr(name)),
                m_FolderPath(StringToFixedStr(folder_path))
            {
            }
        };
    }
}
