#include <iostream>
#include <vector>

import MyEnum;
import Shape;
import MyClass;
import PostGenerate;

struct D : Shape 
{
	virtual void area() const override {};
	virtual double scale_by(double facto) const noexcept override { return 0.0; };
	virtual struct Test::MyStruct GetMyStruct() override { return Test::MyStruct{}; }
};

int main()
{
	auto e = MyEnum::a();
	auto b = MyFunc(MyArg{ 42 });
	C c;
	c.Foo();
	D d;

	std::cout << "hi!";
	return 0;
}