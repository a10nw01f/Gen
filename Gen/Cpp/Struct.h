#pragma once

#include "identifier.h"
#include "../core/static_print.h"
#include <vector>

namespace gen {
    class Struct {
    public:
        constexpr Struct(Identifier identifier, std::vector<TypeAndIdentifier> members): identifier_(identifier), members_(members) {}

        template<auto I>
        constexpr auto& write(StringWriter<I>& writer) {
            writer.format("export struct ${0} {\n", { identifier_.to_string() });
            for (auto& member : members_) {
                writer.format("  ${0} ${1};\n", { member.type.to_string(), member.identifier.to_string() });
            }
            writer.append("\n  constexpr void for_each(auto&& callback) {\n");
            for (auto& member : members_) {
                writer.format("    callback(\"${0}\", ${0});\n", { member.identifier.to_string() });
            }
            writer.append("  }\n};");
            return writer;
        }
    private:
        Identifier identifier_;
        std::vector<TypeAndIdentifier> members_;
    };
}