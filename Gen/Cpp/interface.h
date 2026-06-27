#pragma once

#include <string>
#include "identifier.h"
#include <vector>
#include "../Core/static_print.h"

namespace gen {
    enum class Keywords {
    none = 0,
    const_ = 1,
    noexcept_ = 2
};

constexpr auto operator|(Keywords a, Keywords b) noexcept {
    return static_cast<Keywords>(static_cast<int>(a) | static_cast<int>(b));
}

constexpr bool operator&(Keywords a, Keywords b) noexcept {
    return static_cast<int>(a) & static_cast<int>(b);
}

constexpr std::string to_string(Keywords keywords) {
    std::string result;
    if (keywords & Keywords::const_) {
        result += "const ";
    }
    if (keywords & Keywords::noexcept_) {
        result += "noexcept ";
    }

    return result;
}

struct Interface {
    struct Fn {
        Type ret_;
        Identifier name_;
        std::vector<TypeAndIdentifier> arguments_;
        Keywords keyword_ = Keywords::none;
    };

    Identifier name_;
    std::vector<Fn> functions_;

    constexpr Interface(Identifier name): name_(name) {}

    constexpr Interface& fn(Type ret, Identifier identifier, auto... args) {
        functions_.emplace_back(Fn{ret, identifier, { args...} });
        return *this;
    }

    constexpr Interface& fn(Type ret, Identifier identifier, Keywords keywords, auto... args) {
        functions_.emplace_back(Fn{ret, identifier, { args...}, keywords });
        return *this;
    }

    template<auto I>
    constexpr auto& write(gen::StringWriter<I>& writer) {
        writer.format(R"(
export class ${0} {
public:
)", { name_.to_string() });

        for (auto& fn : functions_) {
            writer.format("virtual ${0} ${1}", { fn.ret_.to_string(), fn.name_.to_string() });
            const char* separator = "(";
            if (fn.arguments_.empty()) {
                writer.append(separator);
            }
            else {
                for (auto& arg : fn.arguments_) {
                    writer.append(separator);
                    writer.format("${0} ${1}", { arg.type.to_string(), arg.identifier.to_string() });
                    separator = ", ";
                }
            }
            writer.format(") ${0} = 0;\n", { to_string(fn.keyword_) });
        }

        writer.format(R"(
virtual ~${0} () = default;
};
)", { name_.to_string() });
        return writer;
    }

    template<auto I>
    friend constexpr auto& operator>>(gen::StringWriter<I>& writer, Interface interface) {
        return interface.write(writer);
    }
};
}
