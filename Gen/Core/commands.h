#pragma once

#include "static_print.h"
#include <iostream>
#include <source_location>
#include "path.h"

namespace gen {

    template<auto I>
    constexpr auto& system(StringWriter<I>& writer, std::string_view command) {
        writer.add_string("System");
        writer.add_string(command);
        return writer;
    }

    template<Directory dir, auto I>
    constexpr auto& write_file(StringWriter<I>& writer, std::string_view file, auto&& callback) {
        writer.add_string("WriteFile");
        writer.add_string(dir.get_file_path(file));
        callback();
        writer.end_string();
        return writer;
    }

}