module;

#include <Gen/Core/System.h>
#include <Gen/Core/Inject.h>
#include <Gen/Cpp/Enum.h>
#include <Gen/Cpp/Interface.h>
#include <Gen/GLSL/Uniform.h>
#include <Gen/Cpp/Cpp2.h>

export module GenerateExamples;

import Empty;

export namespace Test 
{
    struct MyStruct {};
}

using namespace Gen;

constexpr void Generate()
{
    // open the calculator
    System<"calc.exe">();

    constexpr auto imports = StaticString("import GenerateExamples;");

    // generate interface class
    Interface<{"Shape"},
        imports,
        InterfaceFunc<double, "area", {"const", "noexcept"}>,
        InterfaceFunc<void, "scale_by", { "noexcept"}, {Tw<double>, "factor"}>,
        InterfaceFunc<Test::MyStruct, "GetMyStruct", {}>>();

    // generate a class module file that behaves like an enum class
    EnumClass<{"MyEnum"}, imports, {"a"}, {"b", 1}, {"c", 5}, {"d"}>(Tw<long>);

    // inject content into a file
    Inject<{"Example.template.h"}, {"MyClass.ixx"},
        "/*GENERATE_EXPORT_HERE*/", "export module MyClass;",
        "/*GENERATE_MY_CLASS_HERE*/", "class MyClass{};",
        "/*GENERATE_MY_ARG_HERE*/", "export struct MyArg{int x;};",
        "/*GENERATE_MY_IMPORTS_HERE*/", imports>();

    // generate c++ and glsl struct for uniform buffer
    Uniform<{"Material"},
        imports,
        { Tw<float>, "m_Roughness" },
        { Tw<float>, "m_Metallic" },
        { Tw<float>, "m_Specular" },
        { Tw<int>, "m_TextureID" },
        { Tw<Vec<float, 3>>, "m_Diffuse" } >();
#ifdef CPP2EXAMPLE
    Cpp2<{"HelloCpp2.h"}, R"(
HelloCpp2: () -> int = {
    s: std::string = "Gen";
    myfile := fopen("xyzzy", "w");
    myfile.fprintf("Hello with UFCS from Cpp2 and %s!", s.c_str() );
    myfile.fclose();
    return 0;
})", "-cl">();
#endif
}
