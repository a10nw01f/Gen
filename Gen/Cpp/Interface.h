#pragma once

#include "../Core/std.h"
#include "../Core/StaticString.h"
#include "../Core/Utils.h"
#include "../Core/Identifier.h"

namespace Gen
{
    template<class T, int I>
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
    struct VirtualFuncKeywards : std::array<FixedString, I>
    {
        template<class... Ts>
        consteval VirtualFuncKeywards(Ts&&... args) : std::array<FixedString, I>{StringToFixedStr(std::forward<Ts>(args))...} {
            for (auto i = 0ul; i < this->size(); ++i)
            {
                const auto& keyward = std::string_view((*this)[i].m_Data);
                if (keyward != "const" && keyward != "noexcept")
                {
                    throw("Invalid keyward");
                }
            }
        }
    };

    template<class... Ts>
    VirtualFuncKeywards(Ts&&... args)->VirtualFuncKeywards<sizeof...(Ts)>;
    
    template<class Ret, StaticString name, VirtualFuncKeywards keywardlist, FuncArg... Args>
    struct InterfaceFunc
    {
        constexpr static bool IsInterfaceFunction = true;
        constexpr auto Stringiy() const
        {
            static_assert(Idetifier<name>, "name must be a valid identifier");

            return StringToArray_v<[]{
                std::string str = std::string(GetTypeName<Ret>()) + ' ' + name.m_Array + "(";
                ForEach([&str](auto arg) {
                    str += std::string(arg.m_Type.GetName()) + ' ' + arg.m_Name.m_Array + ',';
                    }, Args...);

                constexpr int kNumArgs = sizeof...(Args);
                if constexpr (0 < kNumArgs)
                {
                    str.back() = ')';
                }
                else
                {
                    str += ')';
                }

                for (auto i = 0ul; i < keywardlist.size(); ++i)
                {
                    str += ' ';
                    str += keywardlist[i].m_Data;
                }

                return str;
            } > ;
        }
    };

    template<class T>
    concept InterfaceFunctionsConcept = requires(T x) { 
        { InterfaceFunc{x} } -> std::same_as<T>;
    };

    template<NameAndFolder file_info, StaticString imports, InterfaceFunctionsConcept... functions>
    constexpr void Interface()
    {
        constexpr auto name = StringToArray_v<[]{
            return file_info.m_Name.String();
        }>;

        static_assert(Idetifier<name>, "name must be a valid identifier");

        constexpr auto module_content = StringToArray_v<[]{
            auto content = std::string("export module ") + file_info.m_Name.data() + ";\n" + imports.m_Array + "\nexport class " + file_info.m_Name.data() + "\n{\npublic:\n";

            ForEach([&content](auto& func) {
                content += std::string("\tvirtual ") + func.m_Array + " = 0;\n";
            }, functions{}.Stringiy()...);

            content += std::string("\tvirtual ~") + file_info.m_Name.data() + "() noexcept{}\n};";

            return content;
        }>;

        WriteFile<PathWithExt<file_info, ".ixx">(), module_content>();
    }
}