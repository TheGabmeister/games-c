# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build

```bash
cmake -S . -B build
cmake --build build
```

The executable is emitted to `build/raylib-game-template/`. Resources from `src/resources/` are automatically copied next to the executable at build time. `CMAKE_EXPORT_COMPILE_COMMANDS` is on.

New `.c`/`.h` files added to `src/` are picked up automatically by CMake glob.

## Architecture

Raylib C game using a **screen state machine**: `LOGO → TITLE → OPTIONS/GAMEPLAY → ENDING`.

- `src/raylib_game.c` — main entry point, window/audio init, screen transition loop with fade effects
- `src/screens.h` — shared globals (`currentScreen`, `font`, `music`, `fxCoin`) and screen lifecycle declarations
- `src/screen_*.c` — one module per screen, each implementing: `Init`, `Update`, `Draw`, `Unload`, `Finish`
- `vendor/raylib/` — vendored raylib source, statically linked. Treat as read-only third-party code.

Screen transitions and fade logic are centralized in `raylib_game.c`. Each screen module uses file-local `static` state. Resource paths are relative to the executable (e.g. `LoadFont("resources/mecha.png")`).

## Conventions

- Match existing C style and naming
- New assets go in `src/resources/`
- New game modules go in `src/` (auto-discovered by CMake)
- Do not modify `vendor/raylib/` unless explicitly required
