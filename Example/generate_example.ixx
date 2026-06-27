module;

#include <Gen/core/static_print.h>
#include <Gen/cpp/interface.h>
#include <Gen/cpp/module.h>
#include <Gen/cpp/struct.h>

export module generate_example;

static consteval void generate_example() {
    constexpr auto writer = [] {
        using namespace gen;
        StringWriter writer;

        system(writer, "gnome-calculator");
        system(writer, "calc.exe");


        write_module<{}>(writer, "shape", "generate_example", [&] {
            gen::Interface("Shape")
                    .fn(tp<int>, ID(area), Keywords::const_)
                    .fn(tp<void>, ID(foo), tp<int> - ID(arg)).write(writer);

            gen::Struct("Material", {
                tp<int> - ID(texture),
                tp<float> - ID(opacity)
            }).write(writer);
        });

        return writer;
    }();
    gen::static_print<writer>();
}
