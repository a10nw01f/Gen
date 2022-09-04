#pragma once

#include <variant>
#include "../Core/std.h"
#include "../Core/Identifier.h"

namespace Gen
{
    struct Var
    {
        std::string m_Type;
        std::string m_Identifier;
    };

    struct Exp
    {
        struct Var
        {
            std::string m_Identifier;
            std::vector<Exp> m_Initialize;
        };

        struct Op
        {
            std::string m_Op;
            std::vector<Exp> m_Operands;

            constexpr Op() = default;

            constexpr Op(const std::string& t, const std::vector<Exp>& u): m_Op(t),m_Operands(u)
            {}
        };

        struct Scope
        {
            std::vector<Exp> m_Statements;
            constexpr Gen::Var Var(const std::string& name, const Exp& exp)
            {
                m_Statements.emplace_back(Exp("void", Exp::Var{ name, { exp } }));
                return Gen::Var{exp.m_Type, name };
            }

            constexpr std::string ToString() const
            {
                std::string str = "{\n";
                for(auto& exp : m_Statements)
                {
                    str += exp.ToString() + ";\n";
                }
                str += "}";
                return str;
            }

            constexpr void operator+=(Exp&& exp)
            {
                m_Statements.emplace_back(std::move(exp));
            }
        };

        struct If
        {
            std::vector<Exp> m_Condition;
            Scope m_Scope;
        };

        struct Literal
        {
            std::string m_Value;
        };

        struct Ref
        {
            std::string m_Identifier;
        };

        using Variant = std::variant<Var, Op, If, Literal, Ref>;

        constexpr Exp() = default;

        template<class U, class T>
        constexpr Exp(U&& type, T&& var): m_Type(std::forward<U>(type)), m_Variant(std::forward<T>(var)){}

        constexpr Exp(const Gen::Var& var)
        {
            this->m_Type = var.m_Type;
            this->m_Variant = Exp::Ref(var.m_Identifier);
        }

        constexpr std::string ToString() const
        {
            return std::visit([this](auto& val){
                using T = std::decay_t<decltype(val)>;
                if constexpr(std::is_same_v<T, Ref>)
                {
                    return val.m_Identifier;
                }
                else if constexpr(std::is_same_v<T, Literal>)
                {
                    return val.m_Value;
                }
                else if constexpr(std::is_same_v<T, Var>)
                {
                    return val.m_Initialize[0].m_Type + " " + val.m_Identifier + " = " + val.m_Initialize[0].ToString();
                }
                else if constexpr(std::is_same_v<T, Op>)
                {
                    const Op& op = val;
                    if(IsValidIndentifierChar(op.m_Op[0]))
                    {
                        std::string str = op.m_Op + '(';
                        for(auto& opr : op.m_Operands)
                        {
                            str += opr.ToString() + ',';
                        }
                        str.back() = ')';
                        return str;
                    }
                    else
                    {
                        std::string str;
                        bool appendOp = true;
                        for(auto& opr : op.m_Operands)
                        {
                            str += opr.ToString();
                            if(appendOp)
                            {
                                str += op.m_Op;
                                appendOp = false;
                            }
                        }
                        return str;
                    }
                }
                else if constexpr(std::is_same_v<T, If>)
                {
                    const If& iff = val;
                    return "if(" + iff.m_Condition[0].ToString() + ")\n" + iff.m_Scope.ToString();
                }
                return std::string("exp");
            }, m_Variant);
        }

        std::string m_Type;
        Variant m_Variant;
    };

    constexpr void CheckSameType(const Exp& a, const Exp& b)
    {
        if (a.m_Type != b.m_Type)
        {
            throw "types dont match";
        }
    }

    struct Arg : Var 
    {
        std::string m_Modifier;
    };

    struct Function
    {
        std::string m_Identifier;
        std::string m_RetType;
        std::vector<Arg> m_Args;
        Exp::Scope m_Scope;

        constexpr auto ToString() const
        {
            std::string str = m_RetType + ' ' + m_Identifier + '(';
            for(auto& arg : m_Args)
            {
                str += arg.m_Modifier + ' ' + arg.m_Type + ' ' + arg.m_Identifier + ',';
            }
            str.back() = ')';
            str += '\n';
            str += m_Scope.ToString();
            return str;
        }
    };

    struct GLSLFile
    {
        std::vector<Function> m_Functions;

        template<int I, class F>
        constexpr auto Func(std::string ret_type, std::string identifier, std::vector<Arg> args, F&& body) 
        {
            auto& func = m_Functions.emplace_back(Function{identifier, ret_type, args});

            if (args.size() != I)
                throw "wrong vector size";

            ForEachIndex([&](auto... is) 
            {
                body(func.m_Scope, func.m_Args[is]...);
            }, std::make_index_sequence<I>());

            return [ret_type, identifier](auto&&... args){
                return Gen::Exp(ret_type, Gen::Exp::Op(identifier, std::vector<Gen::Exp>{std::forward<decltype(args)>(args)...}));
            };
        }

        constexpr auto ToString()
        {
            std::string str;
            for(auto& func : m_Functions)
            {
                str += func.ToString() + "\n\n";
            }

            return str;
        }
    };

    constexpr auto IntLiteral(int value) 
    {
        return Exp("int", Exp::Literal(ValueToString(value)));
    }

    constexpr auto FloatLiteral(int value) 
    {
        return Exp("float", Exp::Literal(ValueToString(value) + ".0"));;
    }

    constexpr auto operator "" _il(unsigned long long x)
    {
        return IntLiteral(x);
    }

    constexpr auto operator "" _fl(unsigned long long x)
    {
        return FloatLiteral(x);
    }

    constexpr auto operator-(const Exp& a, const Exp& b)
    {
        CheckSameType(a, b);
        return Exp(a.m_Type, Exp::Op( "-", {a, b}));
    }

    constexpr auto operator*(const Exp& a, const Exp& b)
    {
        CheckSameType(a, b);
        return Exp(a.m_Type, Exp::Op( "*", {a, b}));
    }

    constexpr auto operator==(const Exp& a, const Exp& b)
    {
        CheckSameType(a, b);
        return Exp("bool", Exp::Op( "==", {a,b}));
    }

    constexpr auto operator<(const Exp& a, const Exp& b)
    {
        CheckSameType(a, b);
        return Exp("bool", Exp::Op( "<", {a, b}));
    }

    constexpr auto dot(Exp&& a, Exp&& b)
    {
        CheckSameType(a, b);
        return Exp("float", Exp::Op( "dot", {std::move(a),std::move(b)}));
    }

    constexpr auto clamp(const Exp& v, const Exp& min, const Exp& max)
    {
        CheckSameType(v, min);
        CheckSameType(v, max);
        return Exp(v.m_Type, Exp::Op( "clamp", {v,min,max}));
    }

    template<class F>
    constexpr auto If(const Exp& condition, F&& scope)
    {
        Exp exp;
        exp.m_Type = "void";
        auto& iff = exp.m_Variant.emplace<Exp::If>();
        iff.m_Condition.emplace_back(condition);
        scope(iff.m_Scope);

        return exp;
    }

    constexpr auto Assign(const Var& a, const Exp& b)
    {
        CheckSameType(a, b);
        return Exp(a.m_Type, Exp::Op( "=", {a,b}));
    }

    constexpr auto Vec4(const Exp& vec3, const Exp& w)
    {
        return Exp("vec4", Exp::Op( "vec4", {vec3,w}));
    }

    constexpr auto Vec3(const Exp& x, const Exp& y, const Exp& z)
    {
        return Exp("vec3", Exp::Op( "vec3", {x,y,z}));
    }

    constexpr auto Vec2(const Exp& x, const Exp& y)
    {
        return Exp("vec2", Exp::Op( "vec2", {x,y}));
    }
}