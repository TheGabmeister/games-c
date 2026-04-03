# AGENTS.md

This file provides guidance to coding agents working in this repository.

## Build Commands

```powershell
# Configure (Visual Studio 18 2026 generator, x64)
cmake -S . -B build -G "Visual Studio 18 2026" -A x64 -Wno-dev

# Build
cmake --build build --config Debug

# Run
.\build\Debug\sdl_only.exe
```

Notes:

- The CMake project name is `sdl_only`, so the executable is `sdl_only.exe`.
- On Windows, `CMakeLists.txt` copies the SDL3 runtime DLL next to the built executable after build.

## Current Project Layout

This repository is currently a minimal SDL3 C17 starter, not the larger engine-style template described in older docs.

### `CMakeLists.txt`

- Sets `CMAKE_C_STANDARD` to `17`
- Builds SDL3 from `vendor/sdl` via `add_subdirectory()`
- Builds a single executable from `src/main.c`
- Links only `SDL3::SDL3`

When adding new `.c` files, add them to the `add_executable()` source list in `CMakeLists.txt`.

### `src/main.c`

Current responsibilities:

- Includes `SDL3/SDL.h` and `SDL3/SDL_main.h`
- Defines the application entry point
- Is currently a minimal stub that returns immediately

### `vendor/sdl`

- Vendored SDL3 source used by the build
- Treat this as third-party code unless the task explicitly requires patching SDL itself

## Agent Guidance

- Keep `src/main.c` small. If the project grows, prefer introducing new modules in `src/` instead of turning `main.c` into a catch-all file.
- Do not assume helper files from older templates exist. Create them only when the task actually needs them.
- Preserve the current plain SDL3 setup unless the user asks for a larger abstraction layer or engine structure.
- Prefer updating documentation when architecture changes so `AGENTS.md` stays aligned with the real codebase.
- Avoid editing `vendor/sdl` unless the user explicitly asks for dependency-level changes.
