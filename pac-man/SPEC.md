# SPEC.md — Pac-Man (Modernized)

## 1. Window & Rendering

- Logical grid: 28 columns x 36 rows. Tile size: 24px. Maze area: 672x864 pixels, centered in the window.
- Window: 750x1000. The maze is centered with empty space around it for HUD elements.
- Target framerate: 60 FPS
- Coordinate system: tile (0,0) at top-left. Pixel position = tile × tile_size. Entity positions tracked as floating-point pixel coords.
- Rendering order (back to front): background → maze walls → dots/pellets → fruit → ghosts → Pac-Man → UI overlay
- **Modern visual style:** smooth rounded walls (not blocky pixel art), glow effects on pellets and power-ups, subtle particle effects on dot consumption and ghost eating. The game should look like a polished modern take on Pac-Man, not a pixel-perfect retro recreation.

## 2. Maze

- Single maze layout for all levels, stored as a 28x36 `const int` array in `maze.c`/`maze.h`
- Tile types: `WALL`, `DOT`, `POWER_PELLET`, `EMPTY`, `GHOST_DOOR`, `TUNNEL`
- 240 dots + 4 power pellets = 244 consumables
- Power pellet positions: near the four corners of the playfield
- Tunnel tiles: row 17, columns 0–4 (left) and 23–27 (right). Entities wrap at column boundaries.
- Ghost house: centered at columns 11–16, rows 15–17. Door at tiles (13–14, 15). Interior is 3 tiles tall.

## 3. Pac-Man

- Starting position: tile (14, 26), facing left
- Moves continuously in current direction until hitting a wall
- **Input:** arrow keys, WASD, and gamepad left stick / D-pad. All input methods work simultaneously — no mode switching needed.
- **Gamepad:** use raylib's gamepad API. Left stick with a deadzone (~0.3) maps to 4 directions. D-pad also supported. Any face button starts the game from title/game-over. Start button pauses.
- **Direction queuing:** when a direction is pressed, store it as pending. At each tile boundary, if the pending direction is valid (no wall), turn. Otherwise continue in current direction. This allows pre-turning before reaching an intersection.
- **Cornering:** Pac-Man can cut corners slightly (~half a tile of tolerance when turning at intersections)
- **Animation:** 3 frames cycling (closed, half-open, fully-open mouth). Speed tied to movement. Static frame when stopped.
- **Death animation:** Pac-Man opens wide then deflates in a circular wipe (~11 frames over ~1.5 seconds)

## 4. Collision

All collision is tile-based: two entities collide when they occupy the same tile (i.e. their tile coordinates match).

- **Dots & Power Pellets:** consumed when Pac-Man enters the tile
- **Fruit:** collected when Pac-Man enters the fruit's tile
- **Ghosts:** collision checked each frame. If Pac-Man and a ghost share the same tile:
  - Ghost in CHASE/SCATTER → Pac-Man dies
  - Ghost in FRIGHTENED → ghost is eaten
  - Ghost in EATEN/IN_HOUSE/EXITING → no collision
- **Pass-through prevention:** if Pac-Man and a ghost swap tiles in a single frame (moving toward each other), that also counts as a collision. Check both current-tile overlap and whether they crossed paths since last frame.

## 5. Dots & Power Pellets

- Regular dot: 10 points. Eating pauses Pac-Man for ~16ms (1 frame at 60 FPS).
- Power pellet: 50 points. Eating pauses Pac-Man for ~50ms (3 frames at 60 FPS). Pellets flash on/off at ~0.2s intervals.
- Track remaining dot count. Level complete when count reaches 0.
- Dot state stored as a parallel 28x36 boolean array (eaten or not). Reset each new level.

## 6. Ghosts

Four ghosts, each with a name, color, scatter corner, and starting position:

| Ghost | Color | Scatter Corner | Start Position |
|-------|-------|---------------|----------------|
| Blinky | Red (#FF0000) | Top-right | Outside ghost house |
| Pinky | Pink (#FFB8FF) | Top-left | Inside ghost house, center |
| Inky | Cyan (#00FFFF) | Bottom-right | Inside ghost house, left |
| Clyde | Orange (#FFB852) | Bottom-left | Inside ghost house, right |

**Ghost state machine (per ghost):**

| State | Behavior |
|-------|----------|
| IN_HOUSE | Waiting inside ghost house, bobbing up and down. Exits when dot counter or timer threshold is met (see Section 10). |
| EXITING | Moving upward through the ghost door to the starting position above the house. Transitions to current global mode (CHASE or SCATTER). |
| CHASE | Use unique targeting algorithm |
| SCATTER | Target assigned corner tile |
| FRIGHTENED | Move randomly at reduced speed, can be eaten |
| EATEN | Eyes only, return to ghost house at high speed, then regenerate and re-enter EXITING |

**Reversal rules:** Ghosts cannot voluntarily reverse. Forced reversal when:
- Global mode switches between scatter ↔ chase
- A power pellet is eaten (entering frightened mode)

Ghosts move at reduced speed through tunnel tiles.

## 7. Ghost AI

**Pathfinding at intersections:** At each intersection, evaluate all valid directions (excluding reverse). For each candidate, compute Euclidean distance from the resulting next tile to the target tile. Choose the smallest distance. Tie-breaking priority: Up > Left > Down > Right.

**Blinky (Red) — direct chase:**
- Target = Pac-Man's current tile
- **Cruise Elroy:** when remaining dots fall below a threshold, Blinky speeds up and ignores scatter mode (see Section 14)

**Pinky (Pink) — ambush:**
- Target = 4 tiles ahead of Pac-Man's facing direction
- **Overflow bug:** when Pac-Man faces up, target is 4 tiles up AND 4 tiles left. Implement this for authenticity.

**Inky (Cyan) — unpredictable:**
1. Find the tile 2 tiles ahead of Pac-Man's facing direction
2. Draw a vector from Blinky's current tile to that reference tile
3. Double that vector. The endpoint is Inky's target.

**Clyde (Orange) — shy:**
- If more than 8 tiles from Pac-Man (Euclidean): target = Pac-Man's tile (same as Blinky)
- If 8 or fewer tiles away: target = Clyde's scatter corner (retreats)

## 8. Scatter/Chase Timing

Ghosts alternate between scatter and chase on a global timer:

| Phase | Level 1 | Levels 2–4 | Levels 5+ |
|-------|---------|-----------|----------|
| Scatter 1 | 7s | 7s | 5s |
| Chase 1 | 20s | 20s | 20s |
| Scatter 2 | 7s | 7s | 5s |
| Chase 2 | 20s | 20s | 20s |
| Scatter 3 | 5s | 5s | 5s |
| Chase 3 | 20s | 1033s | 1037s |
| Scatter 4 | 5s | 1/60s | 1/60s |
| Chase 4+ | indefinite | indefinite | indefinite |

- Timer pauses during frightened mode, resumes when it ends
- Each mode transition forces all non-eaten ghosts to immediately reverse direction
- Blinky in Cruise Elroy mode stays in chase even during scatter phases

## 9. Frightened Mode

Triggered when Pac-Man eats a power pellet:
- All ghosts in chase/scatter switch to frightened (reverse direction, turn blue, move randomly)
- Ghosts already in eaten state are unaffected
- Random movement: at each intersection, choose a random valid direction (excluding reverse)

**Duration by level:**

| Levels | Duration | Flashes |
|--------|----------|---------|
| 1 | 6s | 5 |
| 2 | 5s | 5 |
| 3 | 4s | 5 |
| 4 | 3s | 5 |
| 5 | 2s | 5 |
| 6 | 5s | 5 |
| 7 | 2s | 5 |
| 8 | 2s | 5 |
| 9 | 1s | 3 |
| 10 | 5s | 5 |
| 11 | 2s | 5 |
| 12 | 1s | 3 |
| 13 | 1s | 3 |
| 14 | 3s | 5 |
| 15 | 1s | 3 |
| 16 | 1s | 3 |
| 17 | 0s | 0 |
| 18 | 1s | 3 |
| 19+ | 0s | 0 |

- 0s duration: ghosts reverse direction but do not turn blue or become vulnerable
- Flashing: ghosts alternate between blue and white sprites before reverting. Flashing occupies the final 2 seconds of frightened duration (or the entire duration if under 2s). Each flash cycle is ~0.28s (0.14s blue, 0.14s white).
- **Ghost eating combo** (per power pellet): 200 → 400 → 800 → 1600 for 1st through 4th ghost. Resets with each new power pellet.
- When a ghost is eaten: game freezes ~1 second showing the point value, then ghost enters eaten state.

## 10. Ghost House

**Initial exit order (start of level):**
- Blinky starts outside, always active immediately
- Other ghosts exit based on a personal dot counter (dots eaten by Pac-Man since level start):

| Level | Pinky | Inky | Clyde |
|-------|-------|------|-------|
| 1 | 0 (immediate) | 30 dots | 60 dots |
| 2 | 0 (immediate) | 0 (immediate) | 50 dots |
| 3+ | 0 (immediate) | 0 (immediate) | 0 (immediate) |

**After losing a life:** Personal dot counters are replaced by a global dot counter. Ghosts exit in order:
- Pinky: 7 dots
- Inky: 17 dots
- Clyde: 32 dots

The global counter deactivates (reverts to personal counters) once Clyde exits using it.

**Global timer fallback:** If Pac-Man does not eat any dots for 4 seconds (levels 1–4) or 3 seconds (levels 5+), the next waiting ghost is forced out. Timer resets each time a dot is eaten or a ghost is released.

**Exit animation:** Ghost floats upward to the door, passes through, then moves to the starting position above the house.

## 11. Fruit

- Appears at the center-bottom of the maze, just below the ghost house
- Appears twice per level: when 70 dots eaten and when 170 dots eaten
- Disappears after ~9–10 seconds if not eaten
- Only one fruit on screen at a time

| Level | Fruit | Points |
|-------|-------|--------|
| 1 | Cherry | 100 |
| 2 | Strawberry | 300 |
| 3–4 | Orange | 500 |
| 5–6 | Apple | 700 |
| 7–8 | Melon | 1000 |
| 9–10 | Galaxian | 2000 |
| 11–12 | Bell | 3000 |
| 13+ | Key | 5000 |

When eaten, the point value is shown briefly at the fruit location (~2 seconds). The bottom HUD displays fruit icons for the current and recent levels.

## 12. Scoring & Lives

- Starting lives: 3 (displayed as Pac-Man icons in the bottom-left)
- Extra life awarded once at 10,000 points (play a sound)
- Score: top-left. "HIGH SCORE": centered at top.
- High score persisted to a file (single integer)
- **On death:** lives decrement. If lives > 0, reset positions (Pac-Man and all ghosts to starting positions), dots remain as-is, continue level. If lives = 0, game over.
- **Game Over:** display "GAME OVER" in the center of the maze for ~3 seconds, then return to title screen.

## 13. Movement Speeds

Base speed (100%) = 9.47 tiles/second (75.75 pixels/second in the original 8px-tile game). Movement = `speed_percentage × BASE_SPEED × tile_size × delta_time`.

| Entity/Context | Level 1 | Levels 2–4 | Levels 5–20 | Levels 21+ |
|----------------|---------|-----------|------------|-----------|
| Pac-Man normal | 80% | 90% | 100% | 90% |
| Pac-Man eating dots | 71% | 79% | 87% | 79% |
| Pac-Man frightened | 90% | 95% | 100% | 90% |
| Ghost normal | 75% | 85% | 95% | 95% |
| Ghost frightened | 50% | 55% | 60% | 60% |
| Ghost tunnel | 40% | 45% | 50% | 50% |
| Ghost eaten (eyes) | 150% | 150% | 150% | 150% |

- Pac-Man uses "eating" speed for 1 frame when consuming a dot, otherwise "normal" or "frightened"
- Ghosts in the tunnel zone use tunnel speed regardless of mode
- Eaten ghosts (eyes) move at ~150%

## 14. Level Progression

Each new level:
- Refill all 244 dots/pellets
- Reset all entity positions
- "READY!" pause (~2 seconds) before play begins
- Update speeds, frightened duration, ghost house thresholds, fruit type, and Cruise Elroy thresholds

**Level tiers:** Level 1 | Levels 2–4 | Levels 5–20 | Levels 21+ (loop forever, no kill screen)

**Cruise Elroy thresholds (Blinky speed-up):**

| Level | Elroy 1 (dots remaining) | Elroy 2 (dots remaining) |
|-------|--------------------------|--------------------------|
| 1 | 20 | 10 |
| 2 | 30 | 15 |
| 3–5 | 40 | 20 |
| 6–8 | 50 | 25 |
| 9–11 | 60 | 30 |
| 12–14 | 80 | 40 |
| 15–18 | 100 | 50 |
| 19+ | 120 | 60 |

## 15. Game States

```
TITLE → READY → PLAYING → DYING → (READY or GAME_OVER)
                   |                       |
                   +→ LEVEL_COMPLETE ──────+→ READY (next level)
                   |
                   +→ PAUSED (toggle with ESC or P)
```

| State | Behavior |
|-------|----------|
| TITLE | Game title, "PRESS ENTER OR START", high score |
| READY | Maze with all dots, entities at starting positions, "READY!" in yellow for ~2s, no input |
| PLAYING | Normal gameplay: dot/pellet consumption, ghost collisions, fruit, level completion, death |
| GHOST_EATEN_PAUSE | Sub-state of PLAYING: freeze ~1s, show score at eaten ghost's position |
| DYING | Death animation ~1.5s, ghosts hidden. If lives > 0 → READY (same level). If 0 → GAME_OVER |
| LEVEL_COMPLETE | Maze flashes (walls blue/white, ~4 flashes over ~2s). Then READY for next level |
| GAME_OVER | "GAME OVER" for ~3s. Check/update high score. Return to TITLE |
| PAUSED | Freeze all timers/movement. "PAUSED" overlay. Resume with ESC, P, or gamepad Start |

## 16. Sound

Generate with rfxgen. Store WAV files in `src/resources/`.

| Sound Event | rfxgen Preset | Filename |
|-------------|---------------|----------|
| Dot eaten (variant A) | `coin` | `dot_a.wav` |
| Dot eaten (variant B) | `coin` | `dot_b.wav` |
| Power pellet eaten | `powerup` | `power_pellet.wav` |
| Ghost eaten | `laser` | `ghost_eaten.wav` |
| Fruit eaten | `coin` | `fruit.wav` |
| Pac-Man death | `explosion` | `death.wav` |
| Extra life | `powerup` | `extra_life.wav` |
| Level start | `blip` | `ready.wav` |
| Menu select | `blip` | `menu_select.wav` |

- Two dot-eaten variants alternate for the classic "waka-waka" effect
- Load all sounds at startup with raylib's `LoadSound()`

## 17. Sprites

Sprites authored as SVG, exported to PNG via Inkscape. Stored in `src/resources/`. Use a higher resolution than the classic 16x16 (e.g. 32x32 or 64x64) for a cleaner modern look.

**Pac-Man:**
- 3 animation frames × 4 directions = 12 directional frames
- Death animation: ~11 frames
- Smooth, clean circle with a subtle gradient or shading — not flat pixel art

**Ghosts:**
- 2 body frames × 4 directions = 8 frames per ghost
- Frightened (shared): 2 blue frames + 2 white frames (for flashing)
- Eaten (shared): 4 eye frames (one per direction)
- Smooth rounded body shapes with expressive eyes

**Maze:** Render walls procedurally from tile data using raylib draw functions. Use rounded corners and a subtle glow/neon effect on wall edges.

**Dots/Pellets:** Draw procedurally — small filled circle for dots, larger glowing circle for power pellets with a pulsing animation.

**Fruit:** 8 sprites: cherry, strawberry, orange, apple, melon, galaxian, bell, key

**Visual effects:**
- Dot consumption: small particle burst or fade-out
- Power pellet: pulsing glow aura
- Ghost eaten: score popup with a brief flash
- Frightened mode: screen-wide subtle color shift or vignette
- Level complete: wall color flash animation
- Dark background with the maze as the main light source (neon aesthetic)

**Color palette:**

| Element | Color |
|---------|-------|
| Background | #0A0A1A (near-black) |
| Pac-Man | #FFFF00 |
| Blinky | #FF0000 |
| Pinky | #FFB8FF |
| Inky | #00FFFF |
| Clyde | #FFB852 |
| Maze walls | #2121DE (with glow) |
| Dots | #FFCC99 |
| Power pellet glow | #FFFFFF with soft bloom |
| Frightened ghost | #2121FF |
| Frightened flash | #FFFFFF |
| Ghost door | #FFB8FF |
| Text / "READY!" | #FFFFFF / #FFFF00 |

## 18. Implementation Phases

Each phase produces something testable:

1. **Window + Maze** — Render the static maze grid. Verify the 28x36 layout.
2. **Pac-Man Movement** — Keyboard + gamepad input, tile-based movement, direction queuing. No ghosts, no dots.
3. **Dots & Scoring** — Dot/pellet consumption, score tracking, level completion detection.
4. **Ghost Movement** — Four ghosts with basic scatter-mode movement and intersection pathfinding.
5. **Ghost AI** — Chase-mode targeting for all four ghosts. Scatter/chase timing alternation.
6. **Frightened Mode** — Power pellet triggers frightened. Ghost eating with combo scoring. Eaten state with eyes returning to house.
7. **Ghost House** — Exit logic, dot counters, global timer fallback.
8. **Game States** — Title, ready, death sequence, game over, level transitions.
9. **Fruit & Polish** — Fruit spawning, lives system, extra life, speed adjustments per level.
10. **Sprites, Sound & Polish** — Replace placeholders with proper sprites. Add sound effects. Add visual effects (glow, particles, screen shake).
