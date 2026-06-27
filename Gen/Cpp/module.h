#pragma once

#include <complex>

#include "interface.h"
#include "../core/commands.h"

namespace gen {
    template<Directory dir, auto I>
    constexpr auto& write_module(
        StringWriter<I>& writer, std::string module_name, std::string_view first_import, std::string_view global_fragment, auto&& callback) {
        gen::write_file<dir>(writer, module_name + ".ixx", [&] {
        writer.format(R"(module;

${0}

export module ${1};
import ${2};
)", { global_fragment, module_name, first_import });
            callback();
    });
        return writer;
    }

    template<Directory dir, auto I>
    constexpr auto& write_module(
        StringWriter<I>& writer, std::string module_name, std::string_view first_import, auto&& callback) {
        gen::write_file<dir>(writer, module_name + ".ixx", [&] {
        writer.format(R"(export module ${0};
import ${1};
)", { module_name, first_import });
            callback();
    });
        return writer;
    }
}