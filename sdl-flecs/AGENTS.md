# AGENTS.md

## Project Summary

This repository is a small C11 game project built with CMake on top of SDL3 and Flecs. The top-level executable target is `pacman`, and the current runtime flow is:

`main.c` -> `game_manager_init()` -> splash scene -> title scene -> level scene

The game is structured as an ECS application:

- `src/components/`: Flecs component types and enums
- `src/managers/`: initialization, resource ownership, singleton setup, entity helpers
- `src/systems/`: Flecs systems executed every frame
- `src/scenes/`: scene-specific spawn/init/update/fini logic
- `res/`: runtime assets copied next to the executable by CMake

## Build And Run

Use the repo root as the working directory.

Configure:

```powershell
cmake -S . -B build
```

Build:

```powershell
cmake --build build --config Release
```

With the current Visual Studio generator, the executable lands at:

```text
build/Release/pacman.exe
```

The game expects relative asset paths like `./res/gfx/...`, `./res/sfx/...`, and `./res/data/...`, so run it from the repo root or from a build output folder that already has the copied `res/` directory beside the executable.

There is no automated test suite in this repo right now. The minimum verification step is a successful CMake build. Manual runtime checks are required for gameplay, UI, rendering, audio, and scene flow changes.

## Repo-Specific Guidance

- Do not treat `vendor/engine/` as part of the active app path unless you explicitly wire it in. The top-level `CMakeLists.txt` does not add that library, and the game uses `src/platform.[ch]`.
- Prefer changing code under `src/` and assets under `res/`. Avoid editing vendored libraries under `vendor/` unless the task is explicitly about a dependency.
- `res/` is copied by the `copy_resources` target that the executable depends on. If you add files under `res/`, rebuilding the app is enough to refresh them next to the executable.
- Settings are persisted to `storage.data` in the process working directory, not under a platform-specific config folder.

## Architecture Notes

### ECS Registration

When adding a new component:

1. Add its header under `src/components/`.
2. Include it in [`src/managers/component.c`](./src/managers/component.c).
3. Register it with `ECS_COMPONENT_DEFINE`.
4. If it is a singleton, also mark it with `ecs_add_id(world, ecs_id(...), EcsSingleton)`.

If you skip step 2 or 3, the component will compile but not be properly registered in the world.

### Systems

System scheduling is centralized in [`src/managers/system.c`](./src/managers/system.c). New systems should be added there with an explicit Flecs phase. The current frame pipeline includes:

- input and time progression
- GUI input/update
- scene update
- physics/audio
- rendering/compositing

### Scenes

Scene dispatch is centralized in [`src/systems/scene.c`](./src/systems/scene.c). To add a new scene:

1. Extend `SceneName` in [`src/components/scene.h`](./src/components/scene.h).
2. Implement `spawn_*`, `init_*`, `update_*`, and `fini_*` in `src/scenes/`.
3. Register the scene in the `_targets` table in [`src/systems/scene.c`](./src/systems/scene.c).

Current scene flow is:

- splash shows logo and sound
- title builds the Nuklear menu
- level spawns a movable sheep entity

### Assets And Registries

Assets are not discovered dynamically. They are loaded through fixed enums and tables:

- textures: [`src/managers/texture.h`](./src/managers/texture.h) and [`src/managers/texture.c`](./src/managers/texture.c)
- sounds: [`src/managers/sound.h`](./src/managers/sound.h) and [`src/managers/sound.c`](./src/managers/sound.c)
- music: [`src/managers/music.h`](./src/managers/music.h) and [`src/managers/music.c`](./src/managers/music.c)
- fonts: [`src/managers/font.h`](./src/managers/font.h) and [`src/managers/font.c`](./src/managers/font.c)
- JSON gameplay data: [`src/managers/data.h`](./src/managers/data.h) and [`src/managers/data.c`](./src/managers/data.c)

When adding a new asset type entry, update both the enum and the load table.

## Known Project Quirks

- `data_manager_init()` tries to load `level1` through `level9`, but this repo currently only contains `res/data/level1.json` and `res/data/pac-man.json`. Missing files will log load failures unless you add them or change the loader.
- The repo currently builds successfully in `Release`, but MSVC emits warnings from vendored Nuklear headers during `src/nuklear_impl.c`. Treat those as vendor warnings unless the task is specifically about the GUI integration.
- Naming is inconsistent in a few places (`pacman`, `PAC-MAN`, `StarterKit`). Preserve behavior unless the task is explicitly a naming cleanup.

## Input And Runtime Behavior

Current keyboard/mouse input lives in [`src/systems/input.c`](./src/systems/input.c):

- `Ctrl+Q` or `Ctrl+W`: quit
- `Ctrl+F`: toggle fullscreen
- `P`: pause
- `Enter`, `Space`, or left mouse click: select
- `WASD`, `IJKL`, arrows, keypad: movement

If you change input behavior, verify both scene navigation and in-level movement.

## Preferred Verification

For most code changes:

1. Build with `cmake --build build --config Release`.
2. If the change affects runtime behavior, launch `build/Release/pacman.exe`.
3. Check that `res/` was copied next to the executable and that audio/images/fonts still load.

If you cannot run the game, say so explicitly and report only the build status.
