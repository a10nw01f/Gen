#include <Gen/GLSL/shader_ast.h>
#include <Gen/Core/commands.h>

static void consteval generate_shader() {
    constexpr auto writer = [] {
        using namespace gen::glsl;
        Program program;
        auto IsPointInCircle = program.fn("IsPointInCircle", "void", [](
            Scope& scope,
            FnArg<"vec2", "point"> point,
            FnArg<"vec2", "center"> center,
            FnArg<"float", "radius"> radius,
            FnArg<"vec3", "color", "inout"> color,
            FnArg<"vec3", "newColor"> newColor) {
            auto dist = scope.var("vec2",  "dist", point - center);
            scope += If(dot(dist, dist) < radius * radius, [&](Scope& scope) {
                scope(color.assign(newColor));
            });
        });

        auto IsPointInRect = program.fn("IsPointInRect", "void", [](
            Scope& scope,
            FnArg<"vec2", "point"> point,
            FnArg<"vec2", "rectMin"> rectMin,
            FnArg<"vec2", "rectMax"> rectMax,
            FnArg<"vec3", "color", "inout"> color,
            FnArg<"vec3", "newColor"> newColor) {
            auto clamped = scope.var("vec2",  "clamped", clamp(point, rectMin, rectMax));
            scope += If(clamped == point, [&](Scope& scope) {
                scope(color.assign(newColor));
            });
        });

        program.fn("mainImage", "void", [&](
            Scope& scope,
            FnArg<"vec4", "fragColor", "out"> fragColor,
            FnArg<"vec2", "fragCoord", "in"> fragCoord) {
                auto primaryColor = scope.var("vec3", "primaryColor", vec3(1,1,1));
                auto bgColor = scope.var("vec3", "bgColor", vec3(0,0,0));
                auto color = scope.var("vec3", "color", bgColor);

                scope(
                    IsPointInCircle(fragCoord, vec2(200,225), 150.f, color, primaryColor),
                    IsPointInCircle(fragCoord, vec2(200,225), 100.f, color, bgColor),
                    IsPointInRect(fragCoord,vec2(225,0),vec2(350,450),color,bgColor),
                    IsPointInRect(fragCoord,vec2(250,200),vec2(400,250),color,primaryColor),
                    IsPointInRect(fragCoord,vec2(300,150),vec2(350,300),color,primaryColor),
                    IsPointInRect(fragCoord,vec2(450,200),vec2(600,250),color,primaryColor),
                    IsPointInRect(fragCoord,vec2(500,150),vec2(550,300),color,primaryColor),
                    fragColor.assign(vec4(color, 1))
                );
        });

        program.visit([](auto&& node) {
            using T = std::decay_t<decltype(node)>;
            if constexpr (std::is_same_v<T, Var>) {
                Var& var = node;
                if (var.name == "bgColor") {
                    var.init = vec3(0,0,1);
                }
                else if (var.name == "primaryColor") {
                    var.init = vec3(0,1,0);
                }
            }
        });

        gen::StringWriter writer;
        gen::write_file<{}>(writer, "mainImage.glsl", [&] {
            program.write(writer);
        });
        return writer;
    }();

    gen::static_print<writer>();
}