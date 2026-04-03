# AGENTS.md

This file provides guidance to coding agents working in this repository.

## Purpose

This repository is a small raylib game template built with CMake. It currently
uses the stock multi-screen flow from the raylib advanced game template:

- `LOGO -> TITLE -> OPTIONS -> GAMEPLAY -> ENDING`

Most screen modules are still template-level placeholders. Agents should favor
small, clear changes that preserve the existing screen system unless the user
asks for a larger redesign.

## Build And Run

Preferred commands from the repo root:

```powershell
cmake -S . -B build-codex -G "Visual Studio 18 2026" -A x64 -Wno-dev
cmake --build build-codex --config Debug
.\build-codex\template\Debug\template.exe
```

The default local build directory `build/` also exists in this repo, but
`build-codex/` is a safer choice for agent work to avoid clobbering the user's
own build outputs.

Resources under `src/resources/` are copied into the executable output
directory by `CMakeLists.txt`.

## Project Layout

- `CMakeLists.txt`: top-level build, raylib dependency, resource-copy steps
- `src/game.c`: app entry point, global shared resources, screen transitions,
  main loop
- `src/screens.h`: shared screen declarations and shared globals
- `src/screen_logo.c`: animated raylib logo intro
- `src/screen_title.c`: title screen
- `src/screen_options.c`: placeholder options screen
- `src/screen_gameplay.c`: placeholder gameplay screen
- `src/screen_ending.c`: ending screen
- `src/resources/`: runtime assets currently loaded by the app
- `vendor/raylib/`: vendored raylib source; treat as third-party code

## Current Architecture

### `src/game.c`

`game.c` owns:

- window and audio initialization
- loading shared assets into globals declared in `screens.h`
- the current screen state machine
- fade transitions between screens
- the main update/draw loop
- shutdown and resource unloading

Keep `game.c` focused on app-level flow. If a feature belongs to one screen,
prefer putting its state and behavior in that screen module instead of adding
more cross-screen globals.

### `src/screens.h`

`screens.h` is the contract between `game.c` and the screen modules.

It currently exposes:

- `GameScreen`
- shared globals: `currentScreen`, `font`, `music`, `fxCoin`
- `Init*`, `Update*`, `Draw*`, `Unload*`, and `Finish*` functions for each
  screen

If you add another screen, update both `screens.h` and the switch statements in
`src/game.c`.

### Screen Modules

Each screen follows the same structure:

- `Init...Screen()`
- `Update...Screen()`
- `Draw...Screen()`
- `Unload...Screen()`
- `Finish...Screen()`

Keep per-screen state in `static` file-local variables whenever possible.

## Agent Guardrails

- Prefer editing files under `src/` and `CMakeLists.txt`.
- Do not modify `vendor/raylib/` unless the user explicitly asks for dependency
  changes.
- Keep resource paths relative to the executable output, for example
  `resources/mecha.png`.
- When adding new source files under `src/`, the current `file(GLOB_RECURSE ...)`
  setup will include them automatically.
- Preserve the existing transition-based screen flow unless a change requires a
  deliberate restructuring.
- Keep code C-style and straightforward; avoid introducing unnecessary
  abstraction for small gameplay or UI tasks.

## Common Pitfalls

- `music` is declared and configured in `src/game.c` but no music file is
  currently loaded. Be careful not to extend that path without loading a valid
  music asset first.
- Post-build resource copying means runtime asset issues are often build-output
  issues, not source-path issues.
- Several screens are placeholders. If you implement real gameplay or menu
  behavior, move beyond the stub text instead of layering more temporary logic
  on top of it.

## Workflow

Before substantial changes:

1. Read the module you are changing and `src/game.c`.
2. Check whether the change affects shared globals or screen transitions.
3. Build when code or CMake changes are involved.

When finishing:

1. Summarize what changed.
2. Report build status.
3. Call out any remaining placeholders, risks, or follow-up work.
