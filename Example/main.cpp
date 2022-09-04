#include <Gen/Core/System.h>

#ifndef __clang__

#include <iostream>

import GenerateExamples;
import MyEnum;
import Shape;
import MyClass;
import Material;

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

#endif


int main(int argc, char** argv)
{

#ifndef __clang__

    Rect rect;

    auto enum_value = MyEnum::a();

    Material mat;
    mat.ForEach([](auto& member, const char* name)
    {
       std::cout << name << std::endl; 
    });

    auto obj = MyFunc(MyArg{});

#else
    Gen::System<"calc.exe">();
#endif
    return 0;
}






