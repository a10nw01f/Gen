#pragma once
#include "../Core/Utils.h"
#include "../Core/Path.h"
#include "../Core/Format.h"

namespace Gen
{
    namespace Impl
    {
        struct Empty{};

        template<class T>
        concept EnumValueType = std::is_same_v<T, Empty> || std::is_integral_v<T>;

        template<EnumValueType Type = Empty>
        struct EnumValueInfo
        {
            FixedString m_Name;
            Type m_Value;

            constexpr EnumValueInfo(const char* str, Type value = Empty{}):
                m_Name(StringToFixedStr(str)),
                m_Value(value)
            {
            }

            constexpr auto GetName() const { return std::string(m_Name.data()); }

            template<class T>
            constexpr T GetValue(T current) const 
            { 
                if constexpr (std::is_same_v<Type, Empty>)
                {
                    return current;
                }
                else
                {
                    return (T)m_Value;
                }
            }
        };
    }
}

namespace Gen
{
    template<NameAndFolder file_info, StaticString import, Impl::EnumValueInfo... values, class Type = int>
    constexpr auto EnumClass(TypeWrapper<Type> = TypeWrapper<int>{})
    {
        constexpr auto module_content = StringToArray_v<[]{
            auto content = Gen::ReserveString();
            Gen::Format(content, R"(module;
#include <compare>

export module ${{0}};

${{2}}

export class ${{0}} 
{
private:
using U = ${{1}};
U m_Value;
public:
auto operator<=>(const ${{0}}&) const = default;
constexpr explicit ${{0}}(U value) : m_Value(value) {}
constexpr explicit operator U() const { return m_Value; }
)", { file_info.m_Name.data(), GetTypeName<Type>(), import.String() });

            Type current = 0;
            ForEach([&content, &current](auto& value)
                {
                    current = value.GetValue(current);
                    Format(content, " static constexpr ${{0}} ${{1}}() { return ${{0}} { (U) ${{2}} }; }\n", 
                        { file_info.m_Name.data(), value.GetName(), ValueToString(current) });

                    current += (Type)1;
                }, values...);

            content += "};";
            return content;
        }>;

        WriteFile<PathWithExt<file_info, ".ixx">(), module_content>();
    }
}