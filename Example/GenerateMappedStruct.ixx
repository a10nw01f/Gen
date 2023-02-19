module;
#include <Gen/Cpp/Struct.h>
#include <iostream>
export module GenerateMappedStruct;

#ifndef __GNUC__

import TestStruct;

constexpr void Generate()
{
    constexpr auto func = [](auto type, auto name){
        using T = decltype(type.GetType());
        return Gen::NameAndType(Gen::Tw<T*>, (name.String() + "Mapped").c_str());
    };

    Gen::MappedStruct<{"MappedStructExample"},"import GenerateMappedStruct;",TestStruct, func>();
}

#endif