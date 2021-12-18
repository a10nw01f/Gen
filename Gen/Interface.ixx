module;
#include <string>
#include <array>
#include <utility>

export module Gen.Interface;

import Gen.Utils;
using namespace Gen;

template<class F, int... Is>
constexpr auto ForEachIndex(F&& func, std::index_sequence<Is...>)
{
	return func(Is...);
}

constexpr bool IsDigit(const char c)
{
    return (c >= '0' && c <= '9');
}

constexpr bool IsValidIndentifierChar(char c)
{
	return IsDigit(c) || c == '_' || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

constexpr bool IsIdentifier(std::string_view str)
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
constexpr bool IsIdentifier()
{
    return !IsDigit(ident.m_Array[0]) && ForEachIndex([](auto... is) {
			return (... && IsValidIndentifierChar(ident.m_Array[is]));
		}, std::make_index_sequence<decltype(ident)::Size - 1>());
}

template<StaticString str>
concept Idetifier = IsIdentifier<str>();

template<StaticString current_dir, StaticString module_name, StaticString imports, StaticString name, StaticString... functions>
constexpr void InterfaceImpl()
{
    constexpr auto module_path = StringToArray_v<[]() {
        return current_dir.String() + '\\' + module_name.m_Array;
    }>;

    constexpr auto module_content = StringToArray_v < []() {
        auto content = "export module " + name.String() + ";\n" + imports.String() + "\nexport class " + name.String() + "\n{\npublic:\n";

        ForEach([&content](auto& func) {
            content += "\tvirtual " + func.String() + " = 0;\n";
            }, functions...);

        content += "\tvirtual ~" + name.String() + "() noexcept{}\n};";

        return content;
    }>;

    WriteModule<module_path, module_content>();
}

export template<class T, int I>
struct FuncArg{
    StaticString<I> m_Name;
    TypeWrapper<T> m_Type;

    consteval FuncArg(TypeWrapper<T> type, const char(&name)[I]):
        m_Name(name)
    {
        if (!IsIdentifier(name))
        {
            throw "Invalid identifier";
        }
    }
};

template<int I>
struct VirtualFuncKeywards : std::array<const char*, I>
{
    template<class... Ts>
    consteval VirtualFuncKeywards(Ts&&... args) : std::array<const char*, I>{std::forward<Ts>(args)...} {
        for (int i = 0; i < this->size(); ++i)
        {
            const auto& keyward = std::string_view((*this)[i]);
            if (keyward != "const" && keyward != "noexcept")
            {
                throw("Invalid keyward");
            }
        }
    }
};

template<class... Ts>
VirtualFuncKeywards(Ts&&... args)->VirtualFuncKeywards<sizeof...(Ts)>;

export namespace Gen
{
    template<class Ret, StaticString name, VirtualFuncKeywards keywardlist = {}, FuncArg... Args >
    struct InterfaceFunc
    {
        constexpr static bool IsInterfaceFunction = true;
        constexpr auto Stringiy() const
        {
            static_assert(Idetifier<name>, "name must be a valid identifier");

            return StringToArray_v < []() {
                auto str = GetTypeName<Ret>() + ' ' + name.m_Array + "(";
                ForEach([&str](auto arg) {
                    str += arg.m_Type.GetName() + ' ' + arg.m_Name.String() + ',';
                    }, Args...);
                if constexpr (sizeof...(Args) > 0)
                {
                    str.back() = ')';
                }
                else
                {
                    str += ')';
                }

                for (int i = 0; i < keywardlist.size(); ++i)
                {
                    str += ' ';
                    str += keywardlist[i];
                }

                return str;
            } > ;
        }
    };
}

template<class T>
concept InterfaceFunctionsConcept = T::IsInterfaceFunction;

export namespace Gen
{
    template<NameAndFolder file_info, StaticString imports, InterfaceFunctionsConcept... functions>
    constexpr void Interface()
    {
        constexpr auto current_dir = StringToArray_v < []() {
            return file_info.m_FolderPath.String();
        } > ;

        constexpr auto module_name = StringToArray_v < []() {
            return file_info.m_Name.String() + ".ixx";
        } > ;

        constexpr auto name = StringToArray_v < []() {
            return file_info.m_Name.String();
        } > ;

        static_assert(Idetifier<name>, "name must be a valid identifier");

        ::InterfaceImpl<current_dir, module_name, imports, name, functions{}.Stringiy()...>();
    }
    
}