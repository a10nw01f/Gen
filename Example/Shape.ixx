export module shape;
import generate_example;

export class Shape {
public:
virtual int area() const  = 0;
virtual void foo(int arg)  = 0;

virtual ~Shape () = default;
};
export struct Material {
  int texture;
  float opacity;

  constexpr void for_each(auto&& callback) {
    callback("texture", texture);
    callback("opacity", opacity);
  }
};