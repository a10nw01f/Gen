module;
#include <array>
#include <string>
#include <source_location>

export module GenerateExamples;

import Gen;
using namespace Gen;

export namespace Test {
    struct MyStruct {
    };
}

constexpr void Generate()
{
    // generate interface module file
    Interface<{"Shape"},
        "import GenerateExamples;",
        InterfaceFunc<void, "area", {"const"} > ,
        InterfaceFunc<double, "scale_by", {"const", "noexcept"}, {Tw<double>, "factor"}>,
        InterfaceFunc<Test::MyStruct, "GetMyStruct">
    >();

    // generate a class module file that behaves like an enum class
    EnumClass<{"MyEnum"}, {"a"}, {"b", 1}, {"c", 5}, {"d"}>(Tw<long>);

    // inject content into a file
    Inject<{"Example.template.h"}, {"MyClass.ixx"},
        "/*GENERATE_EXPORT_HERE*/", "export module MyClass;",
        "/*GENERATE_MY_CLASS_HERE*/", "class MyClass{};",
        "/*GENERATE_MY_ARG_HERE*/", "export struct MyArg{int x;};">();

    // open the calculator during compilation
    System<"calc.exe">();
}