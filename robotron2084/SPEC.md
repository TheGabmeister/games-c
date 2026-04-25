# Robotron 2084 Recreation Spec

## Goal

Build a 2D modernized Robotron: 2084-style twin-stick arena shooter in this
raylib C template. The game should preserve the original's essential pressure:
the player starts near the center of a single-screen arena, fights waves of
robots from all sides, rescues wandering humans for escalating bonuses, and
survives by moving and firing independently.

Target resolution: `1200x900` at `60 FPS`.

This spec is scoped to this repository only. Do not use other projects in the
parent workspace as reference.

## Research Summary

Robotron: 2084 is a 1982 Williams/Vid Kidz arcade twin-stick shooter. The
original uses two 8-way joysticks: one for movement and one for firing. Each
wave is a single screen containing the player, robot enemies, humans, and
electrodes. The player advances by destroying all vulnerable robots; Hulks are
indestructible and do not need to be killed. Touching enemies, projectiles, or
electrodes costs a life. Humans are rescued by touch and score escalating points
within the current wave or life.

Primary researched mechanics to preserve:

- Independent movement and firing, including diagonals.
- Single-screen wave structure with no scrolling.
- Player begins near the center under immediate pressure.
- Destroy all non-Hulk robots to clear the wave.
- Rescue humans for escalating bonuses: `1000`, `2000`, `3000`, `4000`, then
  `5000` each.
- Start with `3` lives and grant an extra life every `25000` points.
- Grunts chase the player in large packs.
- Hulks are invulnerable, kill humans, and are slowed or pushed by shots.
- Spheroids spawn Enforcers.
- Enforcers fire Sparks toward/near the player.
- Quarks spawn Tanks.
- Tanks fire bouncing Shells.
- Brains appear on every fifth wave, chase humans, turn captured humans into
  Progs, and fire homing Cruise Missiles.
- Electrodes are stationary hazards that kill the player, can kill Grunts, and
  can be destroyed by shots for no points.

Research sources:

- StrategyWiki gameplay notes:
  https://strategywiki.org/wiki/Robotron%3A_2084/Gameplay
- Arcade History entry and scoring summary:
  https://www.arcade-history.com/?id=2243&n=robotron-2084&page=detail
- GameFAQs wave pattern FAQ:
  https://gamefaqs.gamespot.com/arcade/584169-robotron-2084/faqs/24698
- Wikipedia development/enemy behavior overview:
  https://en.wikipedia.org/wiki/Robotron%3A_2084

## Design Direction

The game should be recognizably Robotron-inspired but visually modern:

- Sprites are generated as small readable SVG/PNG assets in `src/assets/`.
- Each sprite uses a 20px source cell, matching the repo's asset instructions.
- Render sprites scaled up with crisp nearest-neighbor filtering.
- Use neon color contrast, additive glows, hit flashes, trails, particles, and
  screen shake to make the game feel more kinetic.
- Keep the playfield clean. The player must always be able to read threats.

Recommended visual language:

- Black or near-black arena floor with subtle grid/CRT scanline treatment.
- Bright cyan/white player.
- Red/pink Grunts.
- Green Hulks.
- Orange/yellow humans.
- Blue Enforcers and Sparks.
- Purple Brains and Cruise Missiles.
- White Quarks, red Tanks, orange Shells.
- Electrodes use pulsing hazard colors and rotating silhouettes.

## Game States

Use a simple enum in `Game`:

```c
typedef enum GameMode {
    GAME_MODE_TITLE,
    GAME_MODE_PLAYING,
    GAME_MODE_WAVE_INTRO,
    GAME_MODE_PLAYER_DEAD,
    GAME_MODE_GAME_OVER
} GameMode;
```

State behavior:

- `TITLE`: show title, high score, controls, and "press start".
- `WAVE_INTRO`: freeze gameplay for a short countdown, display wave number, and
  allow the player to assess the spawn layout.
- `PLAYING`: normal update/draw.
- `PLAYER_DEAD`: short explosion/fade pause, reset the human bonus ladder, remove
  hostile projectiles, respawn player if lives remain.
- `GAME_OVER`: show final score and allow restart.

## Input

Keyboard:

- Move: `WASD`
- Alternate move: arrow keys if not used for firing, but default should reserve
  arrows for firing.
- Fire: arrow keys, with diagonal firing through combined arrow keys.
- Start/restart: `Enter`
- Pause: `P`

Gamepad:

- Move: left stick or D-pad.
- Fire: right stick.
- Start: start/menu button.

Mouse fallback:

- Optional modern mode: `WASD` movement and mouse aim/fire while left mouse is
  down. Keep this optional so keyboard twin-stick remains the default.

Implementation detail:

- Normalize movement vectors so diagonal movement is not faster.
- Quantize keyboard fire into 8 directions.
- For gamepad right stick, allow analog direction but emit bullets at a fixed
  speed. If strict classic feel is desired, quantize the stick to 8 directions.

## Arena

- Window: `1200x900`.
- Playfield: full window, with a small HUD band drawn over the top edge without
  changing the playable area.
- Keep all active entities clamped inside the screen.
- Player radius: about `12-14` world pixels after scaling.
- The arena has no scrolling and no walls besides the screen bounds.

HUD:

- Top-left: score and high score.
- Top-center: current wave.
- Top-right: lives icons.
- Use small text and icon sprites. Avoid large UI that steals play space.

## Core Data Model

Keep the game data contiguous and fixed-size. This game benefits from simple
arrays instead of heap-heavy entity abstractions.

Suggested maximums:

```c
#define MAX_ENEMIES      180
#define MAX_HUMANS        40
#define MAX_BULLETS       96
#define MAX_PROJECTILES  160
#define MAX_ELECTRODES    48
#define MAX_PARTICLES    900
#define MAX_FLOAT_TEXT    64
```

Suggested structs:

```c
typedef enum EnemyType {
    ENEMY_GRUNT,
    ENEMY_HULK,
    ENEMY_SPHEROID,
    ENEMY_ENFORCER,
    ENEMY_QUARK,
    ENEMY_TANK,
    ENEMY_BRAIN,
    ENEMY_PROG
} EnemyType;

typedef struct Enemy {
    bool active;
    EnemyType type;
    Vector2 position;
    Vector2 velocity;
    float radius;
    float speed;
    float hp;
    float cooldown;
    float spawn_timer;
    float stun_timer;
    int score_value;
} Enemy;
```

Use similar active-slot structs for humans, bullets, projectiles, electrodes,
particles, and floating score text.

## Player

Player behavior:

- Starts each wave at screen center.
- Moves freely in 8 directions.
- Shoots continuously while fire input is held.
- Bullet fire rate: start around `0.08` seconds.
- Bullet speed: start around `760` pixels/sec.
- Bullet lifetime: expire on screen edge or after about `0.8` seconds.
- Player dies immediately on contact with enemy, enemy projectile, or electrode.
- Player safely rescues humans by touching them.

Modern polish:

- Add a short invulnerability blink only after respawn, not during a normal wave
  start.
- Draw muzzle flashes, bullet trails, and small recoil particles.
- Add a short screen shake on player death and major explosions.

## Humans

Human types:

- Mommy
- Daddy
- Mikey

Behavior:

- Wander slowly and aimlessly.
- Bounce or turn away from screen edges.
- Can be rescued by player collision.
- Can be killed by Hulks.
- Can be converted into Progs by Brains.

Scoring:

- Maintain `humans_rescued_this_life_or_wave`.
- Award `1000`, `2000`, `3000`, `4000`, then `5000`.
- Reset the ladder on wave clear and player death.

## Enemies

### Grunt

- Basic chaser.
- Moves toward player with slight random wobble.
- Spawn in high numbers.
- Killed by one player bullet or by touching electrodes.
- Worth `100`.

### Hulk

- Invulnerable.
- Seeks humans first; if no humans remain, drifts toward the player.
- Kills humans on contact.
- Player bullets slow, shove, or briefly stun Hulks.
- Does not count toward wave-clear requirements.
- Worth `0`.

### Spheroid

- Mobile spawner.
- Moves toward screen edges/corners or wanders with a bias toward corners.
- Spawns Enforcers after a timer if not destroyed.
- Worth `1000`.

### Enforcer

- Mobile shooter spawned by Spheroids.
- Floats around the arena, often toward edges/corners.
- Fires Sparks aimed near the player's current position with random offset.
- Worth `150`.

### Quark

- Mobile spawner.
- Appears starting with Tank waves and later mixed waves.
- Spawns Tanks after a timer if not destroyed.
- Worth `1000`.

### Tank

- Mobile shooter spawned by Quarks or placed in Tank waves.
- Fires bouncing Shell projectiles.
- Worth `200`.

### Brain

- Appears on every fifth wave.
- Prioritizes humans, converting them into Progs on contact.
- Fires Cruise Missiles that home toward the player with a zigzag/wobble.
- Kills the player on contact.
- Worth `500`.

### Prog

- Former human converted by a Brain.
- Fast hostile chaser.
- Worth `100`.

## Projectiles

Player bullets:

- Fired in the current aim direction.
- One hit destroys normal enemies and hostile projectiles.
- Hitting a Hulk applies knockback/slow instead of damage.
- Hitting an electrode destroys it for `0` points.

Enemy projectiles:

- Spark: Enforcer projectile. Moves toward/near the player, optionally curves.
  Worth `25` if shot.
- Shell: Tank projectile. Bounces off screen edges. Worth `50` if shot.
- Cruise Missile: Brain projectile. Homes with a wobbling path. Worth `25` if
  shot.

## Electrodes

Behavior:

- Stationary hazards placed at wave start.
- Kill the player on contact.
- Kill Grunts on contact.
- Can be shot and destroyed.
- No score value.
- Use multiple sprite silhouettes and cycle shape/color per wave.

## Wave Rules

Use deterministic wave templates with light random placement, then scale counts
and speeds as wave number rises.

Classic pattern to preserve:

- Wave 1: Grunts, Hulks, humans, electrodes.
- Wave 2 onward: Spheroids appear.
- Every fifth wave: Brain wave with many humans.
- Wave 7 and every fifth wave after: Tank waves begin.
- Wave 9, 19, 29, etc.: dense Grunt waves.
- Wave 14, 34, 54, etc.: dense Hulk waves.
- Wave 24, 44, 64, etc.: Tank/Enforcer mixed waves.
- Wave 28 onward: Spheroids and Quarks can both appear in normal waves.
- After wave 255, roll back to wave 1 logic while preserving score/lives.

Practical implementation:

- Start with manually tuned templates for waves `1-10`.
- Add a function `build_wave_plan(int wave)` for later waves.
- Use `GetRandomValue()` with spawn padding so entities do not overlap the player
  at wave start.
- Avoid spawning enemies directly on humans.
- Keep a short `WAVE_INTRO` pause before enemies move.

Wave-clear condition:

- Clear when no active enemy exists except Hulks.
- Remaining humans can stay or be cleared during transition.
- Clear hostile projectiles before the next wave.

Difficulty scaling:

- Increase enemy counts gradually.
- Increase Grunt/Prog speed over time.
- Decrease spawner and shooter cooldowns gradually.
- Cap speeds so the game remains readable at 1200x900.

## Scoring and Lives

Scores:

- Grunt: `100`
- Spheroid: `1000`
- Enforcer: `150`
- Quark: `1000`
- Tank: `200`
- Brain: `500`
- Prog: `100`
- Spark: `25`
- Shell: `50`
- Cruise Missile: `25`
- Human rescue ladder: `1000`, `2000`, `3000`, `4000`, `5000+`
- Electrodes and Hulks: `0`

Lives:

- Start at `3`.
- Extra life every `25000` points.
- Track `next_extra_life_score`, initialized to `25000`, then increment by
  `25000` after each award.

High score:

- In-memory high score is enough for the first implementation.
- Persistent high score can be added later as a small text file in the output
  directory.

## Assets

Store generated assets in `src/assets/`.

Sprites:

- `player.svg` / `player.png`
- `grunt.svg` / `grunt.png`
- `hulk.svg` / `hulk.png`
- `spheroid.svg` / `spheroid.png`
- `enforcer.svg` / `enforcer.png`
- `quark.svg` / `quark.png`
- `tank.svg` / `tank.png`
- `brain.svg` / `brain.png`
- `prog.svg` / `prog.png`
- `human_mommy.svg` / `human_mommy.png`
- `human_daddy.svg` / `human_daddy.png`
- `human_mikey.svg` / `human_mikey.png`
- `electrode_*.svg` / `electrode_*.png`

Conversion command pattern:

```bash
inkscape src/assets/player.svg -o src/assets/player.png -w 20 -h 20
```

Texture loading:

- Add texture IDs in `textures.h`.
- Load all sprite PNGs from `assets/...`.
- Skip or fall back gracefully if a texture is missing during development.
- Call `SetTextureFilter(texture, TEXTURE_FILTER_POINT)` for crisp scaling.

## Sound Plan

Use rfxgen to generate short arcade-style WAV files into `src/assets/`.

Suggested sound IDs:

- `SOUND_START`
- `SOUND_PLAYER_SHOOT`
- `SOUND_ENEMY_EXPLODE`
- `SOUND_PLAYER_DIE`
- `SOUND_HUMAN_RESCUE`
- `SOUND_EXTRA_LIFE`
- `SOUND_WAVE_CLEAR`
- `SOUND_BRAIN_MISSILE`
- `SOUND_ENEMY_SHOOT`
- `SOUND_HULK_HIT`

Suggested rfxgen commands:

```bash
"D:/rfxgen_v5.0_win_x64/rfxgen.exe" -g blip -o src/assets/start.wav
"D:/rfxgen_v5.0_win_x64/rfxgen.exe" -g laser -o src/assets/player_shoot.wav
"D:/rfxgen_v5.0_win_x64/rfxgen.exe" -g explosion -o src/assets/enemy_explode.wav
"D:/rfxgen_v5.0_win_x64/rfxgen.exe" -g hit -o src/assets/player_die.wav
"D:/rfxgen_v5.0_win_x64/rfxgen.exe" -g powerup -o src/assets/human_rescue.wav
"D:/rfxgen_v5.0_win_x64/rfxgen.exe" -g coin -o src/assets/extra_life.wav
"D:/rfxgen_v5.0_win_x64/rfxgen.exe" -g powerup -o src/assets/wave_clear.wav
"D:/rfxgen_v5.0_win_x64/rfxgen.exe" -g laser -o src/assets/brain_missile.wav
"D:/rfxgen_v5.0_win_x64/rfxgen.exe" -g blip -o src/assets/enemy_shoot.wav
"D:/rfxgen_v5.0_win_x64/rfxgen.exe" -g hit -o src/assets/hulk_hit.wav
```

Implementation detail:

- Expand `SoundID` in `game.h`.
- Map each sound in `sounds.c`.
- Keep the current resilient loading behavior so missing sounds do not crash the
  game.

## Raylib Graphics Features

Use raylib's simple rendering strengths before adding complexity:

- `RenderTexture2D` for world rendering, then draw it to screen for screen shake
  and optional post effects.
- `BeginBlendMode(BLEND_ADDITIVE)` for glows, laser trails, explosions, and
  enemy spawn pulses.
- `DrawCircleGradient`, `DrawLineEx`, and translucent rectangles for bloom-like
  effects without custom shaders.
- Particle systems for explosions, bullet hits, human rescues, and wave clear.
- Floating score text using `DrawText`.
- Per-entity hit flash timers.
- Brief full-screen flash on player death and wave clear.

Optional later polish:

- A very lightweight shader for CRT curvature or chromatic aberration.
- Background grid distortion during wave transitions.

## Collision

Use circle-vs-circle collision for moving entities:

```c
bool circles_overlap(Vector2 a, float ar, Vector2 b, float br);
```

Use simple rectangle bounds for screen edge clamps and projectile bounces.

Collision order:

1. Player bullets vs enemies/projectiles/electrodes.
2. Enemy projectiles vs player.
3. Enemies/electrodes vs player.
4. Player vs humans.
5. Hulks/Brains vs humans.
6. Grunts vs electrodes.

This order lets player shots protect the player during the current frame, which
feels fair in a fast arcade game.

## File-Level Implementation Plan

Existing files:

- `src/main.c`: set `WINDOW_WIDTH` and `WINDOW_HEIGHT` through `game.h`; keep
  initialization loop structure.
- `src/game.h`: constants, enums, structs, and public game functions.
- `src/game.c`: game initialization, input, wave logic, entity updates,
  collisions, and drawing.
- `src/sounds.c` / `src/sounds.h`: expanded sound loading/playback.
- `src/textures.c` / `src/textures.h`: expanded texture loading/unloading.

Suggested new files only if `game.c` becomes hard to navigate:

- `src/waves.c` / `src/waves.h`
- `src/entities.c` / `src/entities.h`
- `src/particles.c` / `src/particles.h`

Prefer keeping the first playable version simple in `game.c` until duplication
or file size becomes a real problem.

## Build and Verification

Build:

```bash
cmake --build build
```

If needed:

```bash
cmake -S . -B build
cmake --build build
```

Manual smoke test:

- Launch executable from `build/template/Debug/`.
- Confirm window is `1200x900`.
- Confirm assets load from `assets/...` in the executable output directory.
- Confirm missing sounds/textures do not crash during development.
- Confirm player movement and firing work independently.
- Confirm wave 1 can be cleared.
- Confirm every fifth wave uses Brain behavior.
- Confirm humans score in the correct ladder.
- Confirm extra life triggers at `25000`.

## First Playable Milestone

The first playable target should include:

- `1200x900` window.
- Title, playing, death, and game-over states.
- Player movement and 8-direction firing.
- Grunts, Hulks, humans, electrodes.
- Bullet collisions, deaths, rescue scoring, wave clear.
- Basic generated sprites and sounds.
- Additive particles and screen shake.

After that, add:

- Spheroids and Enforcers.
- Brain waves with Progs and Cruise Missiles.
- Quarks and Tanks.
- Full wave pattern and scaling.
- Additional visual polish.
