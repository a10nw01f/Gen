#pragma once

#include <string>
#include <vector>
#include <optional>
#include <variant>

#include "../core/utils.h"
#include "../core/static_print.h"

namespace gen::glsl {
    struct Exp;

#define GEN_GLSL_BINARY_OP(op) constexpr T operator op (this auto&& self, T&& other) {  \
    if constexpr(std::is_rvalue_reference_v<decltype(self)>) {     \
        return T("@@"#op, { T(std::move(self)), std::move(other) });\
    }\
    else { return T("@@"#op, { T(self), std::move(other) }); }\
}

    template<class T = Exp>
    struct OperatorHelper {
        GEN_GLSL_BINARY_OP(+)
        GEN_GLSL_BINARY_OP(-)
        GEN_GLSL_BINARY_OP(*)
        GEN_GLSL_BINARY_OP(>)
        GEN_GLSL_BINARY_OP(<)
        GEN_GLSL_BINARY_OP(==)
        constexpr T assign(this auto &&self, T &&other) {
            if constexpr (std::is_rvalue_reference_v<decltype(self)>) {
                return T("@@""=", {T(std::move(self)), std::move(other)});
            } else { return T("@@""=", {T(self), std::move(other)}); }
        }
    };

#undef GEN_GLSL_BINARY_OP

    inline constexpr std::string int_to_string(int value)
    {
        if (value == 0)
            return std::string("0");
        std::string result;
        bool negative = value < 0;
        if (negative)
        {
            value = -value;
        }

        while (value != 0)
        {
            result = (char)((value % 10) + '0') + result;
            value = value / 10;
        }

        if (negative)
        {
            result = '-' + result;
        }

        return result;
    }

    constexpr float constexpr_abs(float val) {
        return val < 0.0f ? -val : val;
    }

    constexpr std::string float_to_string(float value, int precision = 4) {
        std::string result = "";

        if (value < 0.0f) {
            result += '-';
            value = constexpr_abs(value);
        }

        long long integer_part = static_cast<long long>(value);
        float fractional_part = value - static_cast<float>(integer_part);

        std::string int_str = "";
        if (integer_part == 0) {
            int_str += '0';
        } else {
            while (integer_part > 0) {
                int_str += static_cast<char>('0' + (integer_part % 10));
                integer_part /= 10;
            }
            for (size_t i = 0; i < int_str.length() / 2; ++i) {
                std::swap(int_str[i], int_str[int_str.length() - 1 - i]);
            }
        }
        result += int_str;

        if (precision > 0) {
            result += '.';
            for (int i = 0; i < precision; ++i) {
                fractional_part *= 10.0f;
                int digit = static_cast<int>(fractional_part);
                result += static_cast<char>('0' + digit);
                fractional_part -= digit;
            }
        }

        return result;
    }

    struct Exp : OperatorHelper<Exp> {
        std::string op;
        std::vector<Exp> args;

        constexpr Exp(std::string op, std::vector<Exp> args = {}):
            op(std::move(op)), args(std::move(args))
        {}

        constexpr Exp(int literal):
            op("@literal"), args({ Exp("int(" + int_to_string(literal) + ")") })
        {}

        constexpr Exp(float literal):
            op("@literal"), args({ Exp("float(" + float_to_string(literal) + ")") })
        {}

        constexpr void visit(auto&& callback) {
            callback(*this);;
            for (auto& arg : args) {
                arg.visit(callback);
            }
        }

        template<auto I>
        constexpr void write(gen::StringWriter<I>& writer) const {
            if (op == "@literal" || op == "@ref") {
                writer.append(args[0].op);
            }
            else if (op.starts_with("@@")) {
                auto oper = op.substr(2);
                if (args.size() != 2) {
                    throw "invalid operator";
                }
                writer.append("(");
                args[0].write(writer);
                writer.append(oper);
                args[1].write(writer);
                writer.append(")");
            }
            else {
                writer.append(op);
                writer.append("(");
                const char* separator = "";
                for (auto& arg : args) {
                    writer.append(separator);
                    arg.write(writer);
                    separator = " ,";
                }
                writer.append(")");
            }
        }
    };

    struct Var {
        std::string type;
        std::string name;
        std::optional<Exp> init;

        constexpr Var(std::string type, std::string name, std::optional<Exp> init):
            type(std::move(type)), name(std::move(name)), init(std::move(init))
        {}

        constexpr operator Exp() const {
            return Exp("@ref", { Exp(name, {}) });
        }

        template<auto I>
        constexpr void write(gen::StringWriter<I>& writer) const {
            writer.format("${0} ${1}", { type, name });
            if (init) {
                writer.append(" = ");
                init->write(writer);
            }
        }

        constexpr void visit(auto&& callback) {
            callback(*this);
            if (init) {
                init->visit(callback);
            }
        }
    };

    struct Scope;

    template<class T = Scope>
    struct IIf {
        Exp condition;
        T then;

        constexpr IIf(Exp&& exp, T&& scope):
            condition(std::move(exp)), then(std::move(scope))
        {}

        template<auto I>
        constexpr void write(gen::StringWriter<I>& writer) const {
            writer.append("if (");
            condition.write(writer);
            writer.append(") {\n");
            then.write(writer);
            writer.append("}");
        }

        constexpr void visit(auto&& callback) {
            callback(*this);
            condition.visit(callback);
            then.visit(callback);
        }
    };

    struct Scope {
        std::vector<std::variant<Var, Exp, IIf<Scope>>> children;
        template<auto I>
        constexpr void write(gen::StringWriter<I>& writer) const {
            for (auto& child: children) {
                std::visit([&writer](auto&& arg) {
                    arg.write(writer);
                    writer.append(";\n");
                }, child);
            }
        }

        constexpr void visit(auto&& callback) {
            for (auto& child: children) {
                std::visit([&callback](auto&& arg) {
                    arg.visit(callback);
                }, child);
            }
        }

        constexpr Scope() = default;

        constexpr Scope(auto&& def) {
            def(*this);
        }

        constexpr void append(auto&& child) {
            children.emplace_back(std::forward<decltype(child)>(child));
        }

        constexpr Scope& operator+=(auto&& child) {
            append(std::forward<decltype(child)>(child));
            return *this;
        }

        constexpr void operator()(auto&&... child) {
            (append(std::forward<decltype(child)>(child)), ...);
        }

        constexpr auto var(std::string type, std::string name, std::optional<Exp> init = std::nullopt) {
            auto var = Var(std::move(type), std::move(name), std::move(init));
            auto ref = Exp(var);
            append(std::move(var));
            return ref;
        }
    };

    using If = IIf<Scope>;

    struct Arg {
        std::string type;
        std::string name;
        std::string modifier;

        constexpr operator Exp() const {
            return Exp("@ref", { Exp(name, {}) });
        }
    };

    template<int N>
    struct FixedString {
        constexpr FixedString(const char(&str)[N]) {
            for (auto i = 0; i < N; i++) {
                data[i] = str[i];
            }
        }

        char data[N] = {};

        [[nodiscard]] constexpr std::string_view view() const {
            return std::string_view{data};
        }
    };

    template<FixedString type, FixedString name, FixedString modifier = "">
    struct FnArg : OperatorHelper<Exp> {
        constexpr operator Exp() const {
            return Exp("@ref", { Exp(std::string(name.data), {}) });
        }

        constexpr Arg to_arg() const {
            return Arg{
                .type = std::string(type.data),
                .name = std::string(name.data),
                .modifier = std::string(modifier.data)
            };
        }
    };

    struct Function {
        std::string name;
        std::string return_type;
        std::vector<Arg> args;
        Scope body;

        template<auto I>
        constexpr void write(gen::StringWriter<I>& writer) const {
            writer.format("${0} ${1} (", { return_type,  name });
            const char* separator = "";
            for (auto& arg : args) {
                writer.append(separator);
                writer.format("${0} ${1} ${2}", { arg.modifier, arg.type, arg.name });
                separator = ", ";
            }
            writer.append(") {\n");
            body.write(writer);
            writer.append("}\n");
        }

        constexpr void visit(auto&& callback) {
            callback(*this);
            body.visit(callback);
        }
    };

    template<class... Ts>
    struct TypeList {
        template<class T, class... Us>
        constexpr auto pop_front(this TypeList<T, Us...> self) {
            return TypeList<Us...>{};
        }

        [[nodiscard]] constexpr auto size() const {
            return sizeof...(Ts);
        }

        constexpr auto expand(auto&& fn) {
            fn(Ts{}...);
        }
    };

    template<class R, class M, class... Ts>
    constexpr auto args_to_type_list(R(M::*)(Ts...) const) {
        return TypeList<Ts...>();
    }

    constexpr auto dot(Exp lhs, Exp rhs) {
        return Exp("dot", { std::move(lhs), std::move(rhs) });
    }

    constexpr auto clamp(Exp v, Exp min, Exp max) {
        return Exp("clamp", { std::move(v), std::move(min), std::move(max) });
    }

    constexpr auto vec3(Exp x, Exp y, Exp z) {
        return Exp("vec3", { std::move(x), std::move(y), std::move(z) });
    }

    constexpr auto vec2(Exp x, Exp y) {
        return Exp("vec2", { std::move(x), std::move(y) });
    }

    constexpr auto vec4(Exp x, Exp y) {
        return Exp("vec4", { std::move(x), std::move(y) });
    }

    struct Program {
        std::vector<Function> body;

        template<auto I>
        constexpr void write(gen::StringWriter<I>& writer) const {
            for (auto& fn : body) {
                fn.write(writer);
            }
        }

        constexpr auto fn(std::string name, const char* return_type, auto&& def) {
            Function function;
            function.name = name;
            function.return_type = return_type;
            constexpr auto ptr_to_member = &std::decay_t<decltype(def)>::operator();
            args_to_type_list(ptr_to_member).pop_front().expand([&](auto... args) {
                (function.args.emplace_back(args.to_arg()),...);
                def(function.body, args...);
            });

            body.emplace_back(std::move(function));

            return [name](auto&&... args) {
              return Exp(name, { Exp(std::forward<decltype(args)>(args))... });
            };
        }

        constexpr auto visit(auto&& callback) {
            for (auto& fn : body) {
                fn.visit(callback);
            }
        }
    };
}
