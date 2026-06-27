#pragma once

#include <stdexcept>
#include <string_view>
#include <string>
#include "../Core/utils.h"

namespace gen {
    class Identifier{
        static constexpr bool is_alpha(char c) noexcept {
            return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
        }

        static constexpr bool is_alpha_nun(char c) noexcept {
            return is_alpha(c) || (c >= '0' && c <= '9');
        }
    public:
        constexpr Identifier(std::string_view name) : name_(name) {
            if (name.empty() || !(is_alpha(name[0]) || name[0] == '_') ) {
                throw std::invalid_argument("invalid identifier");
            }
            for (int i = 1; i < name.size(); ++i) {
                if (!(is_alpha_nun(name[i]) || name[i] == '_')) {
                    throw std::invalid_argument("invalid identifier");
                }
            }
        }

        constexpr Identifier(const char* name) : Identifier(std::string_view(name)) {}

        [[nodiscard]] constexpr std::string to_string() const noexcept {
            return name_;
        }
    private:
        std::string name_;
    };

    class Type {
    public:
        template<class T>
        constexpr Type(std::type_identity<T> type): name_(gen::get_type_name(type)) {}

        [[nodiscard]] constexpr std::string to_string() const noexcept {
            return name_;
        }

    private:
        std::string name_;
    };

    template<class T>
    inline constexpr auto tp = std::type_identity<T>{};

    struct TypeAndIdentifier {
        Type type;
        Identifier identifier;
    };

    constexpr auto operator-(Type type, Identifier identifier) {
        return TypeAndIdentifier{type, identifier};
    }
}

#define ID(name) gen::Identifier(#name)
