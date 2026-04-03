# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

```bash
# Configure (Windows / Visual Studio 2026)
cmake -S . -B build -G "Visual Studio 18 2026" -A x64 -Wno-dev

# Build
cmake --build build --config Release

# Executable
build/Release/sdl_only.exe
```

New `.c` files must be added to the `add_executable()` list in `CMakeLists.txt` manually.

## Architecture

Pure C17 project using SDL3 (vendored in `vendor/sdl/`). No sprites, textures, or audio — rendering uses SDL's built-in debug font and primitive drawing only.

### Module dependency graph

```
main.c → platform, input, game, scene
game.c → scene, scenes/*
scenes/* → platform, input, draw, scene (+ other scenes for transitions)
draw → platform
input → platform (for types only)
platform → SDL3
scene → nothing
```

### Modules

- **platform** (`platform.h/c`) — Window lifecycle, timing (nanosecond delta), renderer access. Owns the SDL_Window/SDL_Renderer. Defines shared types: `rectangle`, `color`, `vector2-4`, `camera`.
- **input** (`input.h/c`) — Action-based input with rebindable keys. Actions (`ACTION_UP`, `ACTION_DOWN`, etc.) map to up to `MAX_BINDINGS` scancodes each. Raw `is_key_down`/`is_key_pressed` and mouse functions also live here.
- **draw** (`draw.h/c`) — Rendering primitives: `draw_rect`, `draw_rect_filled`, `draw_line`, `draw_circle`, `draw_circle_filled`, `draw_text`. Color presets as macros (`COLOR_WHITE`, `COLOR_RED`, etc.). Text uses `SDL_RenderDebugText` (8×8 fixed font), scaled via `SDL_SetRenderScale`.
- **scene** (`scene.h/c`) — Scene manager. A `Scene` is a struct of 4 function pointers: `init`, `update`, `draw`, `cleanup`. `scene_set()` auto-cleans the previous scene before initing the new one.
- **game** (`game.c`) — Glue: sets the starting scene and handles shutdown.
- **scenes/** (`src/scenes/`) — One file per scene. Each exports a factory function returning a `Scene` struct (e.g., `Scene title_scene(void)`).

### Adding a new scene

1. Create `src/scenes/my_scene.h` and `src/scenes/my_scene.c`
2. Define static `init`/`update`/`draw`/`cleanup` callbacks, export a `Scene my_scene(void)` factory
3. Add the `.c` to `CMakeLists.txt`
4. Transition from any scene with `scene_set(my_scene())`

### Adding a new input action

Append to the `Action` enum in `input.h` (before `ACTION_COUNT`), then bind default keys in `input_init()` in `input.c`.
