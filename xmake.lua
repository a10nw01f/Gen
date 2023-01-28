add_rules("mode.debug", "mode.release")
set_policy("build.ccache", false)
set_policy("build.warning", true)

add_includedirs("./")
set_languages("c++20")

if not has_config("toolchain") then
    set_toolchains("msvc")
    add_cxxflags("/constexpr:steps10485760") 
end

if is_config("toolchain", "msvc") then
    add_cxxflags("/constexpr:steps10485760") 
end

target("CompilerProxy")
    add_defines("UNICODE")
    add_defines("_UNICODE")
    set_kind("binary")
    add_files("CompilerProxy/*.cpp")

target("Example")
    add_includedirs("./Example/dependencies/cppfront/include")
    if is_config("toolchain", "clang") then
        add_files("Example/main.cpp")
    else
        add_files("Example/*.ixx")
        add_files("Example/*.cpp")
    end
    
    --add_defines("CPP2EXAMPLE")

    before_build_files(function(target)
        tmp = target._MEMCACHE._DATA.toolchains[1]._CACHE._DATA
        for k,v in pairs(tmp) do
            if v.toolname == "cl" then
                v.program = "cl.exe"
            end
        end
    end)

    set_kind("binary")


    