# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

```bash
# Configure (Visual Studio 18 2026 generator, x64)
cmake -S . -B build -G "Visual Studio 18 2026" -A x64 -Wno-dev

# Build
cmake --build build --config Release

# Run
./build/Release/sdl_only.exe
```

When adding new `.c` files, add them to the `add_executable()` call in CMakeLists.txt.

## Architecture

Minimal SDL3 game template in C17. Currently only `src/main.c` exists as a bare entry point with SDL3 linked. The project is designed to grow into a module structure following the patterns documented in AGENTS.md.