# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

```bash
# Configure (Visual Studio 18 2026 generator, x64)
cmake -S . -B build -G "Visual Studio 18 2026" -A x64 -Wno-dev

# Build
cmake --build build --config Release

# Run
./build/Release/hello_sdl.exe
```

Dependencies (SDL3, SDL3_image, SDL3_mixer, SDL3_ttf) are fetched automatically via CMake FetchContent. First configure takes several minutes.

When adding new `.c` files, add them to the `add_executable()` call in CMakeLists.txt.

## Architecture

This is a minimal SDL3 game template in C17.

**platform.c/h** — Abstraction layer over SDL3 providing:
- Window lifecycle: `init_window()`, `close_window()`, `window_should_close()`
- Frame timing: `get_deltatime()`, `get_fps()`
- Rendering access: `get_renderer()` returns the SDL_Renderer (no direct access to globals)
- Input polling with edge detection: `is_key_pressed()` (single-frame) vs `is_key_down()` (held), same pattern for mouse buttons
- All state lives in a single internal `Globals` struct — not exposed externally

**Game loop pattern** — Call order matters:
1. `engine_begin_frame()` — snapshots keyboard/mouse state
2. `SDL_PollEvent()` + `engine_process_event()` — processes SDL events (wheel, quit)
3. `get_deltatime()` — computes frame delta
4. Render using `get_renderer()`
5. Loop exits when `window_should_close()` returns true

**main.c** — Entry point implementing the game loop above. Extend game logic here.
