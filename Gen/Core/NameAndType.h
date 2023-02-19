#pragma once

#include "Identifier.h"
#include "FixedString.h"
#include "Utils.h"

namespace Gen
{
    template<class T>
    struct NameAndType
    {
        Gen::FixedString m_Name;

        constexpr NameAndType(TypeWrapper<T>, const char* str) :
            m_Name(Gen::StringToFixedStr(str))
        {
        }

        constexpr auto GetName() const { return std::string(m_Name.Data()); }
        constexpr auto GetTypeName() const { return ::GetTypeName<T>(); }
        constexpr auto GetType() const { return Gen::Tw<T>; }
    };

    template<NameAndType value>
    concept IsIdentifierAndType = IsIdentifier(value.m_Name.Data());

    template<NameAndType value>
    concept IsIdentifierAndTypeNotVoid = IsIdentifierAndType<value> && !std::is_same_v<void, decltype(value.GetType().GetType())>;
}