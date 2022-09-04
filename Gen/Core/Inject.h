#pragma once
#include "std.h"
#include "StaticPrint.h"
#include "StaticString.h"
#include "Path.h"

namespace Gen
{
    template<Path input_file, Path output_file, StaticString... strs>
    constexpr void Inject()
    {
        StaticPrint<StaticString("Inject").Array(), FullPath<input_file>().Array(), FullPath<output_file>().Array(), 
            StaticString(strs).Array()...>::Print();
    }
}