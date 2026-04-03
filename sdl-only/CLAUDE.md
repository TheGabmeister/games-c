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

Dependencies (SDL3, SDL3_image, SDL3_mixer, SDL3_ttf) live in `vendor/` and are built via `add_subdirectory`.

When adding new `.c` files, add them to the `add_executable()` call in CMakeLists.txt.

## Architecture

SDL3 game template in C17 with a scalable module structure.

**platform.c/h** — Abstraction layer over SDL3:
- Window lifecycle: `init_window()`, `close_window()`, `is_window_ready()`, `window_should_close()`
- Frame timing: `get_deltatime()`, `get_fps()`
- Rendering access: `get_renderer()` returns the SDL_Renderer (no direct access to globals)
- Input polling with edge detection: `is_key_pressed()` (single-frame) vs `is_key_down()` (held), same for mouse
- Per-frame hooks: `platform_begin_frame()`, `platform_process_event()`
- Shared utility types and enums: `vector2`, `rectangle`, keyboard/mouse/gamepad constants
- All state in a single internal `Globals` struct — not exposed externally

**game.c/h** — Game bootstrap layer:
- `game_init()` registers states and sets the starting state
- `game_update(dt)` and `game_draw()` forward to the active game state
- `game_shutdown()` calls `game_state_shutdown()`
- Game-specific state implementations live here (or in separate `state_*.c` files as the project grows)

**game_state.c/h** — Scene/state machine:
- Register states with `game_state_register(STATE_ID, callbacks)`
- Each state has `init`, `update(dt)`, `draw`, `cleanup` callbacks (any may be NULL)
- Switch states with `game_state_switch()` — calls cleanup on old, init on new
- Default enum entries: `STATE_MENU`, `STATE_GAMEPLAY`, `STATE_PAUSE`; add new IDs to the `GameStateID` enum

**resources.c/h** — Asset cache (load once, lookup by path):
- `res_load_texture()`, `res_load_font()`, `res_load_sound()`, `res_load_music()`
- `res_free_all()` at shutdown — do not manually free cached assets
- `res_load_sound()` pre-decodes into memory (SFX); `res_load_music()` streams from disk (BGM)
- Fonts are keyed by path + size, so the same `.ttf` at different sizes gives separate entries
- Audio loading requires `audio_init()` first
- Assets are loaded relative to the executable (e.g. `build/Release/assets/`)

**drawing.c/h** — Rendering helpers using `get_renderer()`:
- `draw_texture()`, `draw_texture_region()`, `draw_texture_ex()`
- `draw_text()` — creates texture per call (fine for dynamic text)
- `draw_rect()`, `draw_rect_outline()`, `draw_line()`, `draw_circle()`

**audio.c/h** — SDL3_mixer 3.x wrappers:
- `audio_init()` / `audio_shutdown()` manage the MIX_Mixer
- `play_sound()` — fire-and-forget via `MIX_PlayAudio()`
- `play_music()` / `play_music_once()` / `stop_music()` / `pause_music()` / `resume_music()`
- `set_master_volume()`, `set_music_volume()` — 0.0 to 1.0
- `get_mixer()` for direct SDL3_mixer API access
- Supported formats: WAV, MP3, OGG Vorbis (all header-only decoders, no external deps)

**main.c** — Entry point wiring everything together. Game loop order:
1. `platform_begin_frame()` — snapshots input
2. `SDL_PollEvent()` + `platform_process_event()` — SDL events
3. `get_deltatime()` → `game_update(dt)`
4. Clear → `game_draw()` → `SDL_RenderPresent()`
5. Loop exits when `window_should_close()` returns true

**Startup/shutdown order**: `init_window()` → `audio_init()` → `game_init()` → main loop → `game_shutdown()` → `res_free_all()` → `audio_shutdown()` → `close_window()`

## Conventions

- Keep game-specific logic in separate state files (e.g. `state_menu.c`) rather than growing `main.c` or `game.c`.
- Route all asset loading through `resources.c` — don't call SDL3 load functions directly.
- Use the wrapper APIs (platform.h, drawing.h, audio.h) instead of calling SDL subsystems directly from game code.
