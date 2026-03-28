# AGENTS.md

This repository is a small raylib-based C game scaffold. Use this file as the working agreement for future agents making changes here.

## Scope and Ownership

- Make gameplay and app changes in `src/`.
- Treat `vendor/raylib/` as third-party code. Do not edit it unless the task explicitly requires a raylib vendor patch.
- Keep changes tightly scoped. This workspace appears to sit inside a larger parent repo, so avoid touching sibling projects outside `c:\dev\games-c\raylib`.

## Repo Map

- `CMakeLists.txt`: top-level build, links vendored raylib, copies `src/resources/` into the runtime output.
- `src/CMakeLists.txt`: glob-loads all `*.c` and `*.h` under `src/`, so new source files are picked up automatically.
- `src/raylib_game.c`: main entry point, window/audio setup, shared asset loading, and screen transition loop.
- `src/screens.h`: shared globals plus the screen lifecycle declarations.
- `src/screen_*.c`: one module per screen (`logo`, `title`, `options`, `gameplay`, `ending`).
- `src/resources/`: runtime assets loaded by relative path such as `resources/mecha.png`.
- `src/raylib_game.rc` and `src/Info.plist`: Windows/macOS app metadata.

## Build and Run

Preferred local build flow:

```powershell
cmake -S . -B build
cmake --build build
```

Notes:

- The executable is emitted under `build/raylib-game-template/`.
- The top-level CMake file copies `src/resources/` next to the built executable after build, so asset loads should work from the build output directory.
- `CMAKE_EXPORT_COMPILE_COMMANDS` is enabled at the top level.

## Architecture Notes

- The game uses a screen-state flow: `LOGO -> TITLE -> OPTIONS/GAMEPLAY -> ENDING`.
- `currentScreen`, `font`, `music`, and `fxCoin` are shared globals declared in `src/screens.h`.
- Each screen module follows the same lifecycle shape:
  - `Init...Screen`
  - `Update...Screen`
  - `Draw...Screen`
  - `Unload...Screen`
  - `Finish...Screen`
- Screen transitions and fade logic live centrally in `src/raylib_game.c`. Prefer keeping transition behavior there instead of re-implementing it in individual screens.

## Coding Conventions

- Match the existing C style and naming before introducing new patterns.
- Prefer small file-local `static` state inside each `screen_*.c` module.
- Keep resource paths relative to the executable, consistent with existing calls like `LoadFont("resources/mecha.png")`.
- Preserve template comments only when they still help; otherwise, favor concise updates over large comment rewrites.

## Safe Change Guidelines

- Do not assume deleted-but-tracked files should be restored. Check the current working tree first and work with what is present.
- Do not rename the top-level CMake project or executable unless the task explicitly asks for rebranding.
- If you add assets, place them under `src/resources/` so the existing copy step continues to work.
- If you add new gameplay modules, include them under `src/`; CMake will pick them up automatically.

## Verification

When you make code changes:

1. Rebuild with CMake.
2. Run the produced executable when the environment supports a windowed app.
3. If you cannot run the game locally, say so clearly and at least verify the build succeeds.

## Known Template Leftovers

- `README.md` is still the upstream template and may not describe the current game accurately.
- Several source files still contain raylib template comments and placeholder text.
- `music` is declared and managed globally, but the actual music load/update path is still commented out in `src/raylib_game.c`.

When changing behavior, preserve working code first and clean up template leftovers only if that cleanup is part of the task.
