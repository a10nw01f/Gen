message(CMAKE_CXX_COMPILER="${CMAKE_CXX_COMPILER}")
add_definitions(-DGEN_COMPILER_PATH=${CMAKE_CXX_COMPILER})

#set(CMAKE_CXX_COMPILER ${CMAKE_BINARY_DIR}/CompilerProxy/CompilerProxy${CMAKE_EXECUTABLE_SUFFIX} CACHE PATH "" FORCE)
#set(CMAKE_CXX_COMPILER ${CMAKE_BINARY_DIR}/CompilerProxy/CompilerProxy${CMAKE_EXECUTABLE_SUFFIX})
if(MSVC)
    set(CMAKE_CXX_COMPILER "${CMAKE_BINARY_DIR}/CompilerProxy/CompilerProxy${CMAKE_EXECUTABLE_SUFFIX}")
else()
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CMAKE_BINARY_DIR}/CompilerProxy/CompilerProxy${CMAKE_EXECUTABLE_SUFFIX}")
endif ()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(Example main.cpp
        generate_shader.cpp)

target_sources(Example
        PUBLIC
        FILE_SET cxx_modules TYPE CXX_MODULES FILES
        generate_example.ixx
        shape.ixx)

target_include_directories(Example PRIVATE ../)