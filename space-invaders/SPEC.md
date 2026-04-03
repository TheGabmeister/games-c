# Space Invaders - Game Specification

## Overview

A modernized Space Invaders built with raylib using only primitive shapes. Same classic mechanics, but with a neon/glow aesthetic, particle effects, screen shake, and smooth animations. No sprites, no textures, no audio.

**Window**: 800x600 at 60 FPS. Title: "Space Invaders". Dark navy background (`(10, 10, 30)`).

---

## Visual Style

- **Color palette**: Deep navy background (`(10, 10, 30)`), neon cyan (player), neon green/yellow/magenta (aliens), orange/red (enemy bullets/explosions), white/cyan (player bullets, UI text).
- **Glow effect**: Key elements (player, aliens, bullets) drawn twice - once solid, once slightly larger and semi-transparent (~0.3 alpha) for a bloom look.
- **Starfield**: ~100 small dots across the background at 3 depth layers (slow/medium/fast), scrolling downward slowly. Varying brightness (white at different alpha levels).
- **Screen shake**: Random offset applied to all draw positions. Decays linearly over its duration.
- **Font**: Use raylib's default font (`DrawText` / `GetFontDefault()`). The existing `mecha.png` bitmap font is a texture and will not be used.

---

## Screen Layout

```
+------------------------------------------+
|  SCORE: 00000    WAVE: 1    LIVES: < < < |  <- HUD (y: 8-28)
+------------------------------------------+
|              [Mystery Ship]              |  <- UFO lane (y: 40)
|                                          |
|   x x x x x x x x x x x                |  <- Alien formation
|   x x x x x x x x x x x                |     starts at y: 70
|   x x x x x x x x x x x                |
|   x x x x x x x x x x x                |
|   x x x x x x x x x x x                |
|                                          |
|                                          |
|                                          |
|     [==]    [==]    [==]    [==]          |  <- Shields (y: 460)
|                                          |
|                  /\                       |  <- Player (y: 550)
+------------------------------------------+
```

- **Play area**: Full window width with 20px padding on each side.
- **HUD**: Top bar, y: 8. Score left-aligned, wave centered, lives right-aligned (drawn as small ship icons).
- **UFO lane**: y: 40.
- **Alien formation start**: Top-left of grid at approximately x: 80, y: 70. Aliens spaced 40px apart horizontally, 36px vertically.
- **Shields**: y: 460, evenly distributed across the play area.
- **Player**: y: 550, starts centered horizontally.
- **Ground line**: Thin glowing line at y: 580 (purely decorative).

---

## Game Entities

### Player
- **Shape**: Chevron built from a triangle (pointing up) with a small rectangle base. ~30px wide, ~20px tall.
- **Movement**: Left/right with arrow keys or A/D. Speed: 300px/s. Clamped to play area bounds (20px padding).
- **Shooting**: Spacebar. One player bullet on screen at a time. If a bullet is already active, input is ignored (no cooldown indicator needed).
- **Lives**: 3. Shown as small ship icons in the HUD.
- **Death animation**: Explosion particle burst, then 2s invulnerability. During invulnerability, the ship blinks (toggle visible every 8 frames). Player can still move and shoot while invulnerable.
- **Respawn**: Same X position where they died (no reset to center).

### Aliens (Invaders)
- **Grid**: 5 rows x 11 columns = 55 aliens.
- **Shapes by row** (all primitives):
  - **Rows 0-1 (top, 22 aliens)**: Small diamond/rhombus (~16px). Worth 30 points. Color: magenta `(255, 0, 180)`.
  - **Rows 2-3 (middle, 22 aliens)**: Circle (~8px radius) with two short antenna lines angled outward. Worth 20 points. Color: cyan `(0, 220, 255)`.
  - **Row 4 (bottom, 11 aliens)**: Rectangle (~24x12px) with two small triangles on the sides ("wings"). Worth 10 points. Color: green `(0, 255, 100)`.
- **Animation**: Two-frame toggle every time the formation takes a horizontal step. Frame A/B have slight shape differences (e.g., antenna angle flips, diamond squashes/stretches).
- **Movement**: The entire formation moves as one unit.
  - **Horizontal step**: 4px per step.
  - **Step timing**: Timer-based. Interval = `0.02s + (aliveCount / 55.0) * 0.48s`. At 55 alive: ~0.5s per step. At 1 alive: ~0.03s per step.
  - **Edge detection**: When any alive alien in the formation would exceed the play area bounds (20px padding) on the next step, the formation drops 12px and reverses direction.
- **Shooting**: Each frame, there is a chance that a random bottom-most alien in a random column fires. Fire chance per frame: `0.005 + (1.0 - aliveCount/55.0) * 0.015`. Max 3 enemy bullets on screen at once. Enemy bullets move downward at 250px/s.

### Bullets
- **Player bullet**: Rectangle 3x10px, white with cyan glow. Moves up at 500px/s. Destroyed when it hits an alien, shield block, mystery ship, or exits the top of the screen.
- **Enemy bullet**: Rectangle 3x10px, orange-red with glow. Moves down at 250px/s. Destroyed on hitting the player, a shield block, or exiting the bottom of the screen.
- **Trails**: Each bullet stores its last 3 positions. Draw fading copies at those positions (alpha: 0.5, 0.25, 0.1) for a comet-tail effect.
- **No bullet-bullet collision**: Player and enemy bullets pass through each other (classic behavior).

### Shields (Barriers)
- **Count**: 4, evenly spaced across the play area.
- **Construction**: Each shield is built from a grid of small blocks (6px wide, 6px tall). The shape is an arch, approximately:
  ```
  Row 0:   . # # # # # # .    (6 blocks, centered)
  Row 1:   # # # # # # # #    (8 blocks, full width)
  Row 2:   # # # # # # # #    (8 blocks, full width)
  Row 3:   # # # . . # # #    (6 blocks, center gap)
  Row 4:   # # . . . . # #    (4 blocks, wider gap)
  ```
  Total: ~32 blocks per shield. Color: green `(0, 200, 80)`.
- **Destruction**: Any bullet (player or enemy) that hits a shield block destroys that block. The block flashes white for 2 frames, then disappears with a small particle burst (5-8 particles). Bullets are also destroyed on shield contact.

### Mystery Ship (UFO)
- **Shape**: Ellipse (~30x12px) with a half-circle dome on top (~12x8px). Color: red `(255, 50, 50)` with a pulsing glow (glow alpha oscillates between 0.2 and 0.5 using a sine wave).
- **Spawn**: Random interval between 15-25 seconds. Direction alternates each appearance (first left-to-right, then right-to-left, etc.). Spawns just off-screen on the entry side.
- **Movement**: 150px/s horizontally. Despawns when it exits the opposite side.
- **Points**: Random value from {50, 100, 150, 200, 300}, chosen at spawn time. On hit, the score value appears as floating text at the hit position.

---

## Game Flow

### Screens

The existing screen system (LOGO -> TITLE -> GAMEPLAY -> ENDING) is preserved.

1. **LOGO**: Already implemented. Keep as-is (raylib branding, uses only primitives).
2. **TITLE**: Dark background matching the game. "SPACE INVADERS" in large white text with glow effect. Row of alien shapes displayed below the title as decoration (one of each type with point values). "PRESS ENTER TO START" blinking text (toggle every 30 frames). High score shown at bottom.
3. **GAMEPLAY**: The main game (see Gameplay Loop below).
4. **ENDING**: Dark background. "GAME OVER" in large text with red glow. Final score and high score displayed. "PRESS ENTER TO PLAY AGAIN" blinking text.
5. **OPTIONS**: Remove this screen. Delete `screen_options.c` and remove all OPTIONS references from `game.c`, `screens.h`, and the screen transition logic.

### Gameplay Loop

1. Wave starts. Player has 3 lives (or remaining lives from previous wave), score carries over.
2. 55 aliens spawn in formation. Shields are rebuilt fresh each wave.
3. Player moves and shoots. Aliens march and shoot back.
4. **Wave clear**: When all 55 aliens are destroyed, brief 2-second pause with "WAVE COMPLETE" floating text, then next wave starts. Each new wave, aliens start one row (36px) lower than the previous wave's starting position (capped so they don't start below the shields). Alien step timing is 10% faster per wave (the base formula is multiplied by `max(0.3, 1.0 - wave * 0.1)`).
5. **Player death**: On hit, lose a life. Explosion particles. 2s invulnerability. If lives reach 0, transition to ENDING screen after a 1.5s delay.
6. **Game over by invasion**: If any alive alien's Y position reaches the shield row (y >= 450), immediate game over. Transition to ENDING screen.
7. **Score passing**: The final score and high score must be accessible from the ENDING screen. Use a global or a shared struct in `screens.h`.

### Scoring
- Bottom aliens (row 4): 10 pts
- Middle aliens (rows 2-3): 20 pts
- Top aliens (rows 0-1): 30 pts
- Mystery ship: 50-300 pts (random)
- High score tracked in memory across screens (reset on app close, not saved to disk).

---

## Visual Effects

### Particle System
A pool of particles (max 512) reused across all effects. Each particle has: position, velocity, color, lifetime, max_lifetime, size. Updated each frame: position += velocity * dt, lifetime counts down, size and alpha shrink proportionally.

- **Alien explosion**: 15-25 particles burst outward in the alien's color. Initial speed: 80-200px/s in random directions. Lifetime: 0.3-0.6s. Size: 2-4px circles.
- **Player death**: 30-40 particles, white/cyan. Initial speed: 100-300px/s. Lifetime: 0.4-0.8s. Size: 3-5px.
- **Shield block hit**: 5-8 particles in shield green. Speed: 40-100px/s. Lifetime: 0.2-0.4s. Size: 2-3px.
- **Bullet impact**: 8-12 tiny sparks in the bullet's color. Speed: 60-150px/s. Lifetime: 0.15-0.3s. Size: 1-2px.

### Bullet Trails
Each bullet stores its last 3 positions (updated each frame). At draw time, render fading copies at those positions with decreasing alpha and slightly smaller size.

### Screen Shake
- **Player death**: amplitude 4px, duration 0.3s.
- **Shield hit**: amplitude 2px, duration 0.1s.
- Implementation: a shake timer and amplitude. Each frame while active, generate random x/y offset in [-amplitude, +amplitude], scaled by remaining time ratio. Apply as offset to all draw calls.

### Floating Score Text
When an alien or mystery ship is destroyed, spawn floating text showing the point value. Floats upward at 40px/s, fades out over 1s. Color matches the destroyed entity. Max 8 floating texts at once.

### HUD Glow
Score and wave text drawn twice: once in white, once slightly offset and larger in cyan at low alpha.

### Ground Line
A thin horizontal line at y: 580, drawn in dim cyan with a subtle glow. Decorative only.

---

## Controls

| Input          | Action                    |
|----------------|--------------------------|
| Left / A       | Move left                |
| Right / D      | Move right               |
| Space          | Shoot                    |
| Enter          | Start / Restart (menus)  |
| ESC            | Quit (closes window)     |

---

## Audio

All audio code will be removed from the project:
- Remove `InitAudioDevice()` / `CloseAudioDevice()` from `game.c`.
- Remove `LoadSound()`, `LoadMusicStream()`, `PlaySound()`, `PlayMusicStream()`, `UpdateMusicStream()`, `UnloadSound()`, `UnloadMusicStream()` calls from all files.
- Remove `extern Sound fxCoin` and `extern Music music` from `screens.h`.
- Remove the `Sound fxCoin` and `Music music` global variables from `game.c`.
- Remove `PlaySound(fxCoin)` calls from `screen_title.c`, `screen_ending.c`, and `screen_gameplay.c`.

---

## File Structure

All game logic goes in `src/`. New files to create:

| File | Purpose |
|------|---------|
| `src/game_types.h` | All structs (Player, Alien, Bullet, Shield, Particle, FloatingText, Star), constants, and shared game state |
| `src/gameplay.c` | Core game logic: init, spawning, movement, collision, wave management, game state |
| `src/particles.c` | Particle system: pool, spawn, update, draw |
| `src/drawing.c` | All entity drawing functions (player shape, alien shapes, shields, bullets, HUD, starfield, effects) |

Files to modify:
| File | Changes |
|------|---------|
| `src/game.c` | Remove audio. Remove OPTIONS screen. Remove `font` loading. Update window title/size. |
| `src/screens.h` | Remove `extern Music music`, `extern Sound fxCoin`, `extern Font font`. Remove OPTIONS screen declarations. Add gameplay state globals (score, high score, lives). |
| `src/screen_gameplay.c` | Rewrite: delegate to `gameplay.c`, `particles.c`, `drawing.c`. |
| `src/screen_title.c` | Rewrite: neon-styled title screen with alien decoration. |
| `src/screen_ending.c` | Rewrite: game over screen with score display. |

Files to delete:
| File | Reason |
|------|--------|
| `src/screen_options.c` | OPTIONS screen removed (no settings needed). |

---

## Technical Notes

- **Collision detection**: AABB via `CheckCollisionRecs()` from raylib.
- **Random**: `GetRandomValue()` from raylib.
- **Delta time**: `GetFrameTime()` for frame-rate-independent movement.
- **Drawing primitives only**: `DrawRectangle`, `DrawTriangle`, `DrawCircle`, `DrawCircleV`, `DrawLine`, `DrawLineEx`, `DrawPoly`, `DrawText`, and their color/alpha variants. No `LoadTexture`, `LoadImage`, or `LoadFont`.
- **No audio**: All audio initialization, loading, playing, and cleanup code removed entirely.
