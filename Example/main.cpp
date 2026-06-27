#include <iostream>

import shape;

class Rectangle : Shape {
public:
    int width = 0;
    int height = 0;

    int area() const noexcept override {
        return width * height;
    }

    void foo(int arg1) noexcept override {

    }
};

int main() {
    Material{42, 0.6f}.for_each([](auto name, auto&& value) {
        std::cout << name << ": " << value << '\n';
    });
    return 0;
}