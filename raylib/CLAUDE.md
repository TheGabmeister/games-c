# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build and Run

```powershell
# Configure (use build-codex to avoid clobbering the user's build/ directory)
cmake -S . -B build-codex -G "Visual Studio 18 2026" -A x64 -Wno-dev

# Build
cmake --build build-codex --config Debug

# Run
.\build-codex\template\Debug\template.exe
```

The Ninja generator requires running from a VS Developer Command Prompt (vcvarsall.bat). The Visual Studio generator handles toolchain resolution automatically and is the safer choice.

There are no tests in this project.

## Architecture

This is a raylib game template using a **screen state machine** pattern. The game flows through: `LOGO → TITLE → OPTIONS → GAMEPLAY → ENDING`.

**game.c** is the hub — it owns the window, audio init, main loop, screen transitions (with fade effects), and shared resource lifecycle. Each screen is a separate `.c` file implementing five lifecycle functions (`Init`, `Update`, `Draw`, `Unload`, `Finish`). `Finish*Screen()` returns an int signaling which screen to transition to next.

**screens.h** is the contract between game.c and screen modules. It declares the `GameScreen` enum, shared globals (`currentScreen`, `font`, `music`, `fxCoin`), and all screen lifecycle function prototypes. Adding a new screen requires updating both screens.h and the switch statements in game.c.

Per-screen state should be `static` file-local variables. Shared state goes through screens.h externs — but prefer keeping things local to screens when possible.

## Key Details

- `vendor/raylib/` is a vendored copy (not a submodule) — treat as read-only third-party code.
- `file(GLOB_RECURSE ...)` in CMakeLists.txt auto-discovers new `.c`/`.h` files under `src/`.
- Resources in `src/resources/` are copied post-build to the executable directory. Runtime paths are relative (e.g., `"resources/mecha.png"`).
- `music` is declared but no music file is loaded — don't call music functions without loading a valid asset first.
- Most screens are still placeholder stubs.
