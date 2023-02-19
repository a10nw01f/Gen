#pragma once

#include "../Core/StaticString.h"
#include "../Core/StaticPrint.h"
#include "../Core/Path.h"
#include "../Core/Format.h"
#include "../Core/Identifier.h"
#include "../Core/NameAndType.h"
#include <utility>

namespace Gen
{
    template<NameAndFolder file_info, StaticString import, NameAndType... members>
    constexpr void Struct() requires (... && IsIdentifierAndType<members>)
    {
        constexpr auto module_content = StringToArray_v<[]{
            auto content = Gen::ReserveString();
            Gen::Format(content, R"(module;
#include <Gen/Cpp/Struct.h>
export module ${{0}};

${{1}}

export struct ${{0}} 
{
    using Self = ${{0}};
)", { file_info.m_Name.Data(), import.String() });

                ForEach([&content](auto& member)
                {
                    Format(content, "    ${{0}} ${{1}};\n", { member.GetTypeName(), member.m_Name.Data() });
                }, members...);

                content += "    using Members = Gen::TypeList< ";
                ForEach([&content](auto& member)
                {
                    Format(content, "Gen::MemberAndName<&Self::${{0}}, \"${{0}}\">,", { member.m_Name.Data() });
                }, members...);

                content.back() = '>';
                content += ";\n};";
                return content;
            }>;

            WriteFile<PathWithExt<file_info, ".ixx">(), module_content>();
    }

    template<auto TMember, StaticString TName> 
    struct MemberAndName{
        static constexpr auto Member = TMember;

        template<class T>
        constexpr auto& GetMember(T&& value) const { return value.*TMember; }
        constexpr auto Name() const { return TName;}
        constexpr auto MemberType() const { return GetDataMemberType(Member); }
        constexpr auto StructType() const { return GetStructType(Member); }
    };

    template<class T>
    constexpr auto ToTuple(T&& value)
    {
        return Expand(std::decay_t<T>::Members(), [&](auto... members){
            return std::tuple((members.GetMember(value))...);
        });
    }

    template<class T>
    concept MemberAndNameConcept = requires(T x) { 
        { MemberAndName{x} } -> std::same_as<T>;
    };

    template<class T, MemberAndNameConcept... Ts>
    constexpr std::true_type IsValidMembersList(TypeWrapper<T>, TypeList<Ts...>) 
        requires (... && std::is_same_v<T,decltype(Ts().StructType().GetType())>)
    {
        return std::true_type();
    }

    template<class T>
    concept StructMemberListConcept = requires(T) {
        {IsValidMembersList(Tw<T>,typename T::Members())} -> std::same_as<std::true_type>;
    };

    template<class T>
    concept NameAndTypeConcept = requires(T x) { 
        { NameAndType{x} } -> std::same_as<T>;
    };

    template<auto Func>
    concept MapFuncConcept = NameAndTypeConcept<decltype(Func(Tw<int>, StaticString("")))>;

    template<NameAndFolder file_info, StaticString import, StructMemberListConcept T, auto Func>
    constexpr void MappedStruct() requires MapFuncConcept<Func>
    {
        auto transformed = Transform(T::Members(), [](auto member){
            constexpr auto val = Func(member.MemberType(), member.Name());
            return ValueWrapper<val>();
        });

        auto filtered = Filter(transformed, [](auto val){
            if(val.Value().m_Name.Data() == std::string_view("")){
                return false;    
            }
            return true;
        });

        Expand(filtered, [](auto... members){
            Struct<file_info, import, members.Value()...>();
        });
    }
}
