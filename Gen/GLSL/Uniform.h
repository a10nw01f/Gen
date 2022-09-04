#pragma once

#include "../Core/std.h"
#include "../Core/Path.h"
#include "../Core/Utils.h"
#include "../Core/Format.h"
#include "Std140.h"

namespace Gen
{
    namespace Impl
    {
        template<class T, int I>
        constexpr std::string GetGLSLTypeName(TypeWrapper<Vec<T,I>>)
        {
            std::string name;
            if constexpr(std::is_same_v<T, int>)
            {
                name += 'i';
            }
            name += "vec";
            name += ('0' + I);
            return name;
        }

        template<class T>
        constexpr std::string GetGLSLTypeName(TypeWrapper<T>)
        {
            return GetTypeName<T>();
        }

        template<class T>
        struct NameAndType
        {
            Gen::FixedString m_Name;

            constexpr NameAndType(TypeWrapper<T>, const char* str) :
                m_Name(Gen::StringToFixedStr(str))
            {
            }

            constexpr auto GetName() const { return std::string(m_Name.data()); }
            constexpr auto GetTypeName() const { return ::GetTypeName<T>(); }
            constexpr auto GetType() const { return Gen::Tw<T>; }
        };
    }

    template<NameAndFolder file_info, StaticString imports, Impl::NameAndType... members>
    constexpr void Uniform()
    {    
        constexpr auto glsl_content = StringToArray_v<[]{
            std::string members_str;
            ForEach([&members_str](auto member) {
                members_str += Format("\n   ${{0}} ${{1}};", { Gen::Impl::GetGLSLTypeName(member.GetType()), member.m_Name.String() });
                }, members...);
            auto content = Format(R"(
struct ${{0}}
{${{1}}
};
)", { file_info.m_Name.String(), members_str });
            return content;
        }>;

        constexpr auto module_content = StringToArray_v<[]{
            std::string members_str;
            std::string for_each_str;
            ForEach([&](auto member) {
                members_str += Format("\n   Gen::Std140<${{0}}> ${{1}};", { member.GetTypeName(), member.m_Name.String() });
                for_each_str += Format("\n      func(${{0}},\"${{0}}\");", { member.m_Name.String() });
            }, members...);

            return Format(R"(module;
#include <Gen/GLSL/Std140.h>

export module ${{1}};

${{0}}

export struct ${{1}}
{${{2}}

    template<class F>
    void ForEach(F&& func)
    {${{3}}
    }
};
)", { imports.String(), file_info.m_Name.String(), members_str, for_each_str});
        }>;

        WriteFile<PathWithExt<file_info,".ixx">(), module_content>();
        WriteFile<PathWithExt<file_info,".glsl">(), glsl_content>();
    }
}