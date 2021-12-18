module;
#include <array>
#include <string>
#include <source_location>

export module Gen.Inject;

import Gen.Utils;
using namespace Gen;

template<int I>
struct Path {
    StaticString<I> m_RelativePath;
    FixedString m_Dir;

    constexpr Path(const char(&relative_path)[I], FixedString dir = GetCurrentDir(std::source_location::current().file_name())) :
        m_RelativePath(relative_path), m_Dir(dir)
    {}
};

template<Path path>
constexpr auto FullPath() {
    return StringToArray_v < []() {
        return path.m_Dir.m_Data + std::string("\\") + path.m_RelativePath.String();
    } > ;
}

export namespace Gen
{
    export template<Path input_file, Path output_file, StaticString... strs>
    constexpr void Inject()
    {
        StaticPrint<StaticString("Inject").Array(), FullPath<input_file>().Array(), FullPath<output_file>().Array(), strs.Array()...>::Print();
    }
}