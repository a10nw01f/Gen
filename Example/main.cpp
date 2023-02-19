

#include <Gen/Core/System.h>

#ifndef __clang__

#include <iostream>
#include <Gen/Cpp/Struct.h>

import GenerateExamples;
import MyEnum;
import Shape;
import MyClass;
import Material;
import MappedStructExample;

class Rect : Shape
{
public:
    int m_Width;
    int m_Height;

	double area() const noexcept override 
    { 
        return m_Width * m_Height; 
    }

	void scale_by(double factor) noexcept override
    {
        m_Width *= factor;
        m_Height *= factor;
    }

	Test::MyStruct GetMyStruct() override { return Test::MyStruct{}; }
};

#ifdef CPP2EXAMPLE
#include "HelloCpp2.h"
#endif

#endif


int main(int argc, char** argv)
{
    
#ifndef __clang__

#ifdef CPP2EXAMPLE
    auto ret = HelloCpp2();
#endif
    Rect rect;

    auto enum_value = MyEnum::a();

    Material mat;
    mat.ForEach([](auto& member, const char* name)
    {
       std::cout << name << std::endl; 
    });

    auto obj = MyFunc(MyArg{});

#ifndef __GNUC__
    MappedStructExample mapped{nullptr, nullptr};
    auto tup = Gen::ToTuple(mapped);
#endif
#else
    Gen::System<"calc.exe">();
#endif
    return 0;
}






