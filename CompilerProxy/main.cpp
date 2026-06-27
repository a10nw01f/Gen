#include <iostream>
#include <string_view>
#include <filesystem>
#include "run_process.h"
#include "process_content.h"

void RunCompilerProcess(const char* command[])
{
    int exit_status = 0;

    std::stringstream compiler_output;
    auto result = RunProcess(command, [&](std::string_view str) {
      std::cout << str;
      compiler_output << str;
    });

    if(!result.has_value())
    {
        exit(-1);
    }

    exit_status |= ProcessContent(compiler_output.str(), [](std::string_view str) {
        std::cout << str;
    });
    
    exit_status |= *result;

    exit(exit_status);
}


int main(int argc, char** argv)
{
    std::string compiler_path;
    std::string_view compiler_path_arg = "-DGEN_COMPILER_PATH=";
    for (int i = 1; i < argc; ++i) {
      auto arg = std::string_view(argv[i]);
      if (arg.starts_with(compiler_path_arg)) {
        compiler_path = arg.substr(compiler_path_arg.length());
        break;
      }
    }

    std::cout << "running compiler proxy\n";

    std::vector<const char*> command = {compiler_path.c_str()};
    for (int i = 1; i < argc; ++i)
    {
        command.emplace_back(argv[i]);
    }

    command.emplace_back(nullptr);
    RunCompilerProcess(command.data());

    return 0;
}
