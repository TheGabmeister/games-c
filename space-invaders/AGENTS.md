# AGENTS.md

## Purpose

This repository is a modernized `Space Invaders` remake in C using raylib.

Agents working here should optimize for:

- preserving the classic Space Invaders mechanics
- keeping the implementation simple and readable
- using primitive-only rendering
- avoiding unnecessary dependencies, assets, and architectural sprawl

Read [SPEC.md](C:/dev/games-c/space-invaders/SPEC.md) before making gameplay or presentation changes. It is the source of truth for design decisions.

---

## Project Layout

- `src/game.c`: app entry point, window setup, screen transitions
- `src/screens.h`: shared screen declarations and shared app/session state
- `src/screen_logo.c`: logo screen
- `src/screen_title.c`: title screen
- `src/screen_gameplay.c`: gameplay screen wrapper
- `src/screen_ending.c`: ending screen
- `src/game_types.h`: gameplay constants, enums, and structs
- `src/gameplay.c`, `src/gameplay.h`: core gameplay state and update logic
- `src/particles.c`, `src/particles.h`: particle system
- `src/drawing.c`, `src/drawing.h`: primitive drawing helpers
- `vendor/raylib`: vendored raylib dependency

---

## Core Constraints

- Use raylib primitives only for visuals.
- Do not add textures, sprite sheets, custom fonts, or audio.
- Keep the gameplay loop faithful to classic Space Invaders.
- Prefer fixed-size arrays/pools over heap allocation.
- Keep desktop keyboard controls as the primary target.
- Keep screen flow as `LOGO -> TITLE -> GAMEPLAY -> ENDING`.
- Do not reintroduce an `OPTIONS` screen unless the user explicitly asks for it.

If you change gameplay behavior, update `SPEC.md` when needed so the implementation and spec stay aligned.

---

## Build And Verify

Preferred configure/build commands from the repo root:

```powershell
cmake -S . -B build
cmake --build build --config Release
```

If a change is code-related, try to build before finishing. If you cannot build, say so clearly in your handoff.

Executable output is placed under:

- `build/space-invaders/`

---

## Coding Guidance

- Favor small, direct C functions over clever abstractions.
- Keep gameplay rules centralized in `gameplay.c` and shared definitions in `game_types.h`.
- Keep drawing logic in `drawing.c` rather than mixing rendering deeply into gameplay state updates.
- Use named constants instead of scattering magic numbers.
- Add brief comments only where intent would otherwise be unclear.
- Match the existing C style in surrounding files.

For gameplay code, prefer this separation:

- state and rules in `gameplay.*`
- particles in `particles.*`
- rendering helpers in `drawing.*`
- screen transitions and high-level app flow in screen files / `game.c`

---

## Design Guardrails

When extending the game, preserve these priorities:

1. Gameplay readability over visual noise.
2. Classic mechanics over feature creep.
3. Deterministic, debuggable logic over flashy but brittle behavior.
4. Simple data ownership over cross-file global sprawl.

Modernization should come from polish:

- glow
- particles
- shake
- trails
- UI clarity

It should not come from changing the fundamental rules unless requested.

---

## Common Pitfalls

- Do not make timing frame-dependent when it should be time-based.
- Do not mix HUD and world-space shake unless intentionally desired.
- Do not add dead resources or asset-loading code for primitive-only features.
- Do not scatter score, wave, and lives state across unrelated globals.
- Do not bypass the screen system unless there is a strong reason.

---

## Agent Workflow

Before substantial edits:

1. Read the relevant section of `SPEC.md`.
2. Inspect the local module you are changing.
3. Prefer focused patches over broad refactors.
4. Build if the change affects code.

When finishing:

1. Summarize what changed.
2. Mention any spec updates made.
3. Report build/test status.
4. Call out follow-up risks if something remains incomplete.
