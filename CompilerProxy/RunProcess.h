#pragma once

#include <optional>
#include <functional>
#include <tchar.h>
#include <strsafe.h>
#include "dependencies/subprocess/subprocess.h"

std::optional<int> RunProcess(const char* cmd[], const std::function<void(const char*)>& on_output)
{
    auto process_error = [&](const char* str){
        on_output(str);
        on_output("\nwith process command:");
        on_output(cmd[0]);
        return std::nullopt;
    };

    subprocess_s subprocess;
    auto flags = subprocess_option_inherit_environment | subprocess_option_combined_stdout_stderr | subprocess_option_search_user_path;
    if(subprocess_create(cmd, flags, &subprocess)){
        return process_error("failed to create subprocess");
    }

    FILE* p_stdout = subprocess_stdout(&subprocess);

    auto read_output = [&]() {
        char buffer[1024];
        while (true)
        {
            buffer[0] = 0;
            auto should_break = fgets(buffer, sizeof(buffer), p_stdout) == nullptr;
            on_output(buffer);
            if (should_break) {
                break;
            }
        }
    };

    read_output();
    int exit_code;
    if(subprocess_join(&subprocess, &exit_code)){
        return process_error("failed to join subprocess");
    }
    read_output();

    subprocess_destroy(&subprocess);

    return exit_code;
}