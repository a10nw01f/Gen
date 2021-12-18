module;

#include <array>
#include <string>
#include <utility>

export module Gen.Enum;

import Gen.Utils;
using namespace Gen;

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


export namespace Gen 
{
    template<NameAndFolder file_info, EnumValueInfo... values, class Type = int>
    constexpr auto EnumClass(TypeWrapper<Type> = TypeWrapper<int>{})
    {
        constexpr auto module_path = StringToArray_v < []() {
            return file_info.m_FolderPath.String() + '\\' + file_info.m_Name.data() + ".ixx";
        } > ;

        constexpr auto module_content = StringToArray_v < []() {
            auto content = Format(R"(module;
#include <compare>

export module ${{0}};

export class ${{0}} 
{
private:
using U = ${{1}};
U m_Value;
public:
auto operator<=>(const ${{0}}&) const = default;
constexpr explicit ${{0}}(U value) : m_Value(value) {}
constexpr explicit operator U() const { return m_Value; }
)", { file_info.m_Name.data(), GetTypeName<Type>() });

            Type current = 0;
            ForEach([&content, &current](auto& value)
                {
                    current = value.GetValue(current);
                    content += Format(R"(	static constexpr ${{0}} ${{1}}() { return ${{0}} { (U) ${{2}} }; }
)", { file_info.m_Name.data(), value.GetName(), ValueToString(current) });

                    current += (Type)1;
                }, values...);

            content += "};";

            return content;
        } > ;

        WriteModule<module_path, module_content>();
    }
}