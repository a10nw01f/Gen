#include <Gen/GLSL/ShaderAST.h>
#include <Gen/Core/Utils.h>
#include <Gen/Core/Path.h>
#include <iostream>

using namespace Gen;

constexpr void ModifyColorInAST(GLSLFile& glsl_file, const char* identifier, const Exp& color_exp)
{
    for(Function& func : glsl_file.m_Functions)
    {
        if(func.m_Identifier == "mainImage")
        {
            for(auto& statement : func.m_Scope.m_Statements)
            {
                bool ret = false;
                std::visit([&](auto& val){
                    using T = std::decay_t<decltype(val)>;
                    if constexpr(std::is_same_v<T, Exp::Var>)
                    {
                        Exp::Var& var = val;
                        if(var.m_Identifier == identifier)
                        {
                            var.m_Initialize[0] = color_exp;
                            ret = true;
                        }
                    }
                }, statement.m_Variant);
                if(ret)
                {
                    return;
                }
            }
        }
    }
}

constexpr void GenerateMainImage()
{
    constexpr auto content = StringToArray_v<[]{
        GLSLFile glsl_file;
        auto IsPointInCircle = glsl_file.Func<5>("void", "IsPointInCircle", { 
            { "vec2", "point" },
            { "vec2", "center" },
            { "float", "radius" },
            { "vec3", "color", "inout"},
            { "vec3", "newColor"}
            }, [](auto& scope, auto& point, auto& center, auto& radius, auto& color, auto& new_color) {
                auto dist = scope.Var("dist", point - center);
                scope += If(dot(dist, dist) < radius * radius, [&](auto& statements) {
                    statements += Assign(color, new_color);
                });
            });

        auto IsPointInRect = glsl_file.Func<5>("void", "IsPointInRect", {
                { "vec2", "point" },
                { "vec2", "rectMin"},
                { "vec2", "rectMax"},
                { "vec3", "color", "inout"},
                { "vec3", "newColor"}
            }, [](auto& scope, auto& point, auto& rect_min, auto& rect_max, auto& color, auto& new_color) {
                auto clampedPoint = scope.Var("clampedPoint", clamp(point, rect_min, rect_max));
                scope += If(clampedPoint == point, [&](auto& statements){
                        statements += Assign(color, new_color);
                    });
        });

        glsl_file.Func<2>("void", "mainImage", {
            {"vec4", "fragColor", "out"},
            {"vec2", "fragCoord", "in" }
            }, [&](auto& scope, auto& fragColor, auto& fragCoord) 
            {
                auto primaryColor = scope.Var("primaryColor", Vec3(1_il, 1_il, 1_il));
                auto bgColor = scope.Var("bgColor", Vec3(0_il, 0_il, 0_il));
                auto color = scope.Var("color", bgColor);

                scope += IsPointInCircle( fragCoord, Vec2(200_il, 225_il), 150_fl, color, primaryColor);
                scope += IsPointInCircle( fragCoord, Vec2(200_il, 225_il), 100_fl, color, bgColor );
                scope += IsPointInRect( fragCoord, Vec2(225_il, 0_il),   Vec2(350_il, 450_il), color, bgColor );
                scope += IsPointInRect(fragCoord, Vec2(250_il, 200_il), Vec2(400_il, 250_il), color, primaryColor);
                scope += IsPointInRect(fragCoord, Vec2(300_il, 150_il), Vec2(350_il, 300_il), color, primaryColor);
                scope += IsPointInRect(fragCoord, Vec2(450_il, 200_il), Vec2(600_il, 250_il), color, primaryColor);
                scope += IsPointInRect(fragCoord, Vec2(500_il, 150_il), Vec2(550_il, 300_il), color, primaryColor);

                scope += Assign(fragColor, Vec4(color, 1_il));
        });

        //ModifyColorInAST(glsl_file, "primaryColor", Vec3(0_il, 1_il, 0_il));
        //ModifyColorInAST(glsl_file, "bgColor", Vec3(0_il, 0_il, 1_il));
        return glsl_file.ToString();
    }>;
    WriteFile<PathWithExt<{"MainImage"}, ".glsl">(), content>();
}