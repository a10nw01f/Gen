#pragma once

#include <source_location>
#include <string>

namespace gen {
    constexpr inline char path_separator = '/';

    constexpr auto file_path(const char* file_name, std::source_location source_location = std::source_location::current()) {
        std::string str(source_location.file_name());
        return str.substr(0, str.find_last_of("/\\")) + path_separator + file_name;
    }


    struct Directory
    {
        char data[1024] = {};
        constexpr Directory(std::source_location source_location = std::source_location::current())
        {
            std::string str(source_location.file_name());
            auto dir = str.substr(0, str.find_last_of("/\\"));
            for (int i = 0; i < dir.length(); ++i) {
                data[i] = dir[i];
            }
        }

        constexpr auto get_file_path(std::string_view file_name) const
        {
            return (std::string(data) + '/').append(file_name);
        }
    };
}
