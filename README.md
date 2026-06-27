# Gen

Gen is a C++23 compile-time code generation library. You describe the code you want in ordinary C++—using constexpr builders for C++ modules, structs, interfaces, and GLSL shaders—and Gen emits source files during compilation, before your program links and runs.

The trick is a small **CompilerProxy** that sits in front of your real compiler, reads generation commands embedded in compiler diagnostics, and performs the side effects (writing files, running commands) as part of the build.

## How it works

```
┌─────────────┐     ┌────────────────┐     ┌──────────────────┐
│ Your C++    │───▶│ CompilerProxy  │────▶│ Real compiler    │
│ + Gen API   │     │ (intercepts    │     │ (MSVC, Clang, …) │
└─────────────┘     │  diagnostics)  │     └──────────────────┘
                    └───────┬────────┘
                            │
                            ▼
                    Write files, run commands
```

1. **Describe output at compile time** — Generation code runs inside `consteval` functions and writes to a fixed-size `StringWriter`.
2. **Encode commands in diagnostics** — `static_print()` turns the writer buffer into compiler messages (`#pragma message` on MSVC, deprecated template instantiation on Clang/GCC).
3. **CompilerProxy decodes and acts** — The proxy forwards invocations to the real compiler, parses the embedded data, and executes `WriteFile` and `System` commands (see `CompilerProxy/process_content.h`).

CMake wires this up automatically: on MSVC, CompilerProxy replaces `CMAKE_CXX_COMPILER`; on other toolchains it is used as `CMAKE_CXX_COMPILER_LAUNCHER`.

## Requirements

- **CMake** 3.31 or later
- A **C++23**-capable compiler

The `Example` target demonstrates both C++ module generation and GLSL shader generation. After a successful build, generated artifacts appear next to the source files that triggered them (for example `Example/shape.ixx` and `Example/mainImage.glsl`).

## Project layout

```
Gen/
├── Gen/                  # Header-only generation library
│   ├── Core/             # StringWriter, static_print, file/system commands
│   ├── Cpp/              # C++ module, interface, and struct builders
│   └── GLSL/             # GLSL shader AST and code emitter
├── CompilerProxy/        # Compiler wrapper that executes generation commands
└── Example/              # Sample project (C++ modules + GLSL shader)
```

## Examples

### C++ modules, interfaces, and structs

`Example/generate_example.ixx` defines a generator that emits a C++20 module:

```cpp
write_module<{}>(writer, "shape", "generate_example", [&] {
    gen::Interface("Shape")
        .fn(tp<int>, ID(area), Keywords::const_)
        .fn(tp<void>, ID(foo), tp<int> - ID(arg)).write(writer);

    gen::Struct("Material", {
        tp<int> - ID(texture),
        tp<float> - ID(opacity)
    }).write(writer);
});
```

Importing the generator module from `shape.ixx` triggers compilation of the generator, which writes `shape.ixx`. Application code then uses the generated types:

```cpp
import shape;

class Rectangle : Shape { /* … */ };

Material{42, 0.6f}.for_each([](auto name, auto&& value) {
    std::cout << name << ": " << value << '\n';
});
```

The `Struct` builder also emits a `for_each` member for simple field iteration.

### GLSL shaders

`Example/generate_shader.cpp` builds a Shadertoy-style `mainImage` shader using the GLSL AST in `Gen/GLSL/shader_ast.h`. Functions, scopes, control flow, and expressions are composed in C++ and written to `mainImage.glsl` at compile time. The AST supports post-generation transforms—for example, changing color constants before emission.

## API overview

| Component | Purpose |
|-----------|---------|
| `gen::StringWriter` | Fixed-capacity buffer for generated text |
| `gen::static_print()` | Encode buffer contents into compiler diagnostics |
| `gen::write_file()` | Queue a file write (path resolved from call site) |
| `gen::system()` | Queue a shell command |
| `gen::Interface` | Generate C++ abstract base classes |
| `gen::Struct` | Generate C++ structs with optional `for_each` |
| `gen::write_module()` | Generate C++20 module interface files |
| `gen::glsl::Program` | Build and emit GLSL source from an AST |


## License

MIT — see [LICENSE](LICENSE).
