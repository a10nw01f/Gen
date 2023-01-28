#include "../Core/Path.h"
#include "../Core/StaticString.h"
#include "../Core/StaticPrint.h"

namespace Gen
{
    template<Path output_file, StaticString cpp2src, StaticString... flags>
    constexpr void Cpp2()
    {
        StaticPrint<StaticString("Cpp2").Array(), FullPath<output_file>().Array(), cpp2src.Array(), flags.Array()...>::Print();
    }
}