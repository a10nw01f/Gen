set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

message(CMAKE_CXX_COMPILER="${CMAKE_CXX_COMPILER}")


add_executable (CompilerProxy "main.cpp"
        ../Gen/glsl/shader_ast.h)