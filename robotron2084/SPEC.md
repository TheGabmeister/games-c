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

This is an original recreation inspired by the 1982 arcade design. Do not use
the original ROM, ripped arcade sprites, ripped cabinet art, or sampled arcade
audio. Generate new sprites and sounds locally.

## Research Summary

Robotron: 2084 is a 1982 Williams/Vid Kidz arcade twin-stick shooter. The
original uses two 8-way joysticks: one for movement and one for firing. Each
wave is a single screen containing the player, robot enemies, humans, and
electrodes. The player advances by destroying all vulnerable robots; Hulks are
indestructible and do not need to be killed. Touching enemies, projectiles, or
electrodes costs a life. Humans are rescued by touch and score escalating points
within the current wave; the rescue ladder also resets if the player dies.

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
- Render sprites at `2x` or `3x` with crisp nearest-neighbor filtering.
- Use neon color contrast, additive glows, hit flashes, trails, particles, and
  screen shake to make the game feel more kinetic.
- Keep the playfield clean. The player must always be able to read threats.

Recommended visual language:

- Black or near-black arena floor with a subtle readable grid treatment.
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
    GAME_MODE_PAUSED,
    GAME_MODE_PLAYER_DEAD,
    GAME_MODE_GAME_OVER
} GameMode;
```

State behavior:

- `TITLE`: show title, high score, controls, and "press start".
- `WAVE_INTRO`: freeze gameplay for a short countdown, display wave number, and
  allow the player to assess the spawn layout.
- `PLAYING`: normal update/draw.
- `PAUSED`: stop simulation, dim the arena, show a compact pause label, and let
  `P` resume.
- `PLAYER_DEAD`: short explosion/fade pause, reset the human bonus ladder, remove
  hostile projectiles, respawn player if lives remain.
- `GAME_OVER`: show final score and allow restart.

## Input

Keyboard:

- Move: `WASD`
- Fire: arrow keys, with diagonal firing through combined arrow keys.
- Start/restart: `Enter`
- Pause: `P`
- Quit: window close button or `Escape`

Gamepad:

- Move: left stick or D-pad.
- Fire: right stick.
- Start: start/menu button.

Implementation detail:

- Normalize movement vectors so diagonal movement is not faster.
- Quantize keyboard fire into 8 directions.
- Keep the last non-zero firing direction for a short time only if it improves
  feel; do not auto-fire with no aim input.
- Apply gamepad dead zones around `0.25f`.
- For gamepad right stick, allow analog direction but emit bullets at a fixed
  speed. If strict classic feel is desired, quantize the stick to 8 directions.

## Arena

- Window: `1200x900`.
- Playfield: full window, with a small HUD band drawn over the top edge without
  changing the playable area.
- Gameplay-safe area: keep spawn points at least `96` pixels from the player and
  `32` pixels from screen edges unless the wave intentionally creates a trap.
- Keep all active entities clamped inside the screen.
- Player collision radius: about `12-14` world pixels. Drawn sprite can be larger
  than the collision circle.
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
    float flash_timer;
    int score_value;
} Enemy;
```

Use similar active-slot structs for humans, bullets, projectiles, electrodes,
particles, and floating score text.

Important fields to include in `Game`:

- `GameMode mode`
- `int score`
- `int high_score`
- `int lives`
- `int wave`
- `int next_extra_life_score`
- `int humans_rescued_this_wave`
- `float mode_timer`
- `float player_fire_timer`
- `float screen_shake`
- Fixed arrays for enemies, humans, bullets, projectiles, electrodes, particles,
  and floating score text.

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
- Clamp player position inside the playfield using the collision radius.
- On death, decrement lives once, trigger effects, clear enemy projectiles, and
  delay respawn by about `1.2` seconds.

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

- Maintain `humans_rescued_this_wave`.
- Award `1000`, `2000`, `3000`, `4000`, then `5000`.
- Reset the ladder when a new wave starts and when the player dies.

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
- Spawn no more than the remaining enemy capacity allows; if the enemy array is
  full, skip spawning rather than reallocating.
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
- Spawn no more than the remaining enemy capacity allows.
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
- If `MAX_BULLETS` is full, skip the new shot but keep the fire timer stable.

Enemy projectiles:

- Spark: Enforcer projectile. Moves toward/near the player, optionally curves.
  Worth `25` if shot.
- Shell: Tank projectile. Bounces off screen edges, has a finite lifetime, and is
  worth `50` if shot.
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
- Default Brain waves should not include Hulks in the early game. Add mixed
  Brain/Hulk pressure only later if the base game becomes too easy.
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
- When the wave clears, remove all remaining Hulks, humans, electrodes, bullets,
  and hostile projectiles during the transition.
- Award no automatic bonus for abandoned humans in the first implementation.

Difficulty scaling:

- Increase enemy counts gradually.
- Increase Grunt/Prog speed over time.
- Decrease spawner and shooter cooldowns gradually.
- Cap speeds so the game remains readable at 1200x900.

Initial wave tuning:

| Wave | Theme | Suggested contents |
| --- | --- | --- |
| 1 | Intro chase | 18 Grunts, 2 Hulks, 5 humans, 10 electrodes |
| 2 | First spawners | 24 Grunts, 2 Hulks, 2 Spheroids, 5 humans, 12 electrodes |
| 3 | More pressure | 32 Grunts, 3 Hulks, 3 Spheroids, 6 humans, 12 electrodes |
| 4 | Mixed normal | 38 Grunts, 3 Hulks, 3 Spheroids, 6 humans, 14 electrodes |
| 5 | Brain wave | 8 Brains, 12 humans, 8 electrodes |
| 6 | Recovery swarm | 42 Grunts, 4 Hulks, 4 Spheroids, 6 humans, 12 electrodes |
| 7 | First tanks | 16 Grunts, 2 Quarks, 4 Tanks, 4 humans, 10 electrodes |
| 8 | Normal mix | 44 Grunts, 4 Hulks, 4 Spheroids, 5 humans, 14 electrodes |
| 9 | Dense Grunts | 70 Grunts, 2 Hulks, 4 humans, 8 electrodes |
| 10 | Brain wave | 10 Brains, 14 humans, 8 electrodes |

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
- If one score event crosses multiple thresholds, award all crossed extra lives
  with a `while (score >= next_extra_life_score)` loop.

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
- If a texture is missing, draw a colored primitive placeholder for that entity
  so gameplay can continue.

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
- Do not play the player shoot sound every bullet if it becomes harsh; throttle
  or vary volume/pitch if needed.

## Raylib Graphics Features

Use raylib's simple rendering strengths before adding complexity:

- `RenderTexture2D` may be used for world rendering and screen shake if needed.
- `BeginBlendMode(BLEND_ADDITIVE)` for glows, laser trails, explosions, and
  enemy spawn pulses.
- `DrawCircleGradient`, `DrawLineEx`, and translucent rectangles for bloom-like
  effects without custom shaders.
- Particle systems for explosions, bullet hits, human rescues, and wave clear.
- Floating score text using `DrawText`.
- Per-entity hit flash timers.
- Brief full-screen flash on player death and wave clear.

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

If player-human rescue and enemy-human contact happen in the same frame, player
rescue wins. That keeps close rescues satisfying instead of feeling stolen by
update order.

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

Implementation sequencing:

1. Set window constants to `1200x900` and build a playable circle/primitive
   version first.
2. Add generated sprites and texture loading after gameplay works.
3. Add rfxgen sounds once the event names are stable.
4. Add visual polish after collision and wave-clear behavior is reliable.

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
- Confirm killing all non-Hulk enemies clears the wave and removes leftover
  Hulks/projectiles before the next wave starts.
- Confirm missing PNG/WAV assets fall back without crashing.

## Milestones

Build the game in small playable slices. Each milestone should compile, run, and
answer one playtest question before moving on.

### Milestone 1: Movement Prototype

Goal: prove the control feel before adding game rules.

- Set the window to `1200x900`.
- Use primitive shapes only; no generated assets yet.
- Add title, playing, pause, and game-over modes only if they stay minimal.
- Implement player movement with `WASD`.
- Implement 8-direction firing with arrow keys.
- Spawn simple bullets with lifetime and screen-edge cleanup.
- Clamp player and bullets to the arena.

Playtest question:

- Does independent movement and firing feel responsive at `60 FPS`?

### Milestone 2: Core Combat Loop

Goal: prove the basic Robotron pressure.

- Add Grunts as simple player-chasing enemies.
- Add bullet-vs-Grunt collision.
- Add player-vs-Grunt death.
- Add score for destroyed Grunts.
- Add lives and game-over.
- Add simple wave clear when all Grunts are destroyed.
- Use primitive explosion particles only if they help debugging feedback.

Playtest question:

- Is it fun to move, shoot, survive, die, and restart with only Grunts?

### Milestone 3: Rescue Loop

Goal: add the conflicting objective that makes Robotron interesting.

- Add wandering humans.
- Add player-vs-human rescue.
- Add rescue score ladder: `1000`, `2000`, `3000`, `4000`, `5000+`.
- Reset the rescue ladder on new wave and player death.
- Add floating score text for rescues.
- Add extra life awards every `25000` points.

Playtest question:

- Do humans create tempting but risky choices without confusing the combat?

### Milestone 4: Arena Hazards

Goal: add path pressure without adding new enemy AI.

- Add Electrodes as stationary hazards.
- Add player-vs-Electrode death.
- Add bullet-vs-Electrode destruction for `0` points.
- Add Grunt-vs-Electrode destruction.
- Add Hulks as invulnerable human hunters.
- Add bullet knockback/slow against Hulks.
- Make wave clear ignore remaining Hulks.

Playtest question:

- Do Hulks and Electrodes add tension without making early waves feel unfair?

### Milestone 5: First Presentation Pass

Goal: make the proven core loop readable and satisfying.

- Generate original SVG/PNG sprites for player, Grunt, Hulk, human variants, and
  Electrodes.
- Load textures through `textures.c` with primitive fallbacks.
- Generate first-pass rfxgen sounds for shooting, rescue, enemy explosion,
  player death, extra life, and wave clear.
- Add restrained screen shake, hit flashes, and additive particles.
- Keep all visual effects subordinate to readability.

Playtest question:

- Is the game clearer and more satisfying than the primitive version?

### Milestone 6: Spawner Enemies

Goal: introduce ranged pressure gradually.

- Add Spheroids.
- Add Enforcers spawned by Spheroids.
- Add Enforcer Sparks.
- Add Quarks.
- Add Tanks spawned by Quarks.
- Add bouncing Tank Shells.
- Add projectile-vs-player death and bullet-vs-projectile scoring.
- Tune projectile counts so the screen remains readable.

Playtest question:

- Does ranged pressure create movement decisions without burying the player in
  noise?

### Milestone 7: Brain Waves

Goal: add the signature rescue panic wave.

- Add Brains every fifth wave.
- Add Brain-vs-human conversion into Progs.
- Add Progs as fast hostile chasers.
- Add Cruise Missiles with homing/wobble behavior.
- Add Brain wave human-heavy layouts.
- Verify early Brain waves do not include Hulks.

Playtest question:

- Do Brain waves feel distinct and urgent because of human conversion?

### Milestone 8: Full Wave Progression

Goal: connect the enemy set into the researched Robotron-like wave pattern.

- Implement tuned wave templates for waves `1-10`.
- Implement rule-based wave planning beyond wave `10`.
- Add dense Grunt waves, Tank waves, Hulk waves, mixed Tank/Enforcer waves, and
  mixed Spheroid/Quark normal waves.
- Add difficulty scaling for enemy counts, speed, spawn timers, and shot timers.
- Roll wave logic after wave `255` while preserving score and lives.

Playtest question:

- Does the game escalate in a readable way while preserving the core loop from
  milestones 2-4?

### Milestone 9: Polish and Persistence

Goal: finish the arcade wrapper after the game is already fun.

- Refine title, pause, death, wave-intro, and game-over screens.
- Add persistent high score if desired.
- Add better HUD icons and final sprite cleanup.
- Balance sounds so repeated shooting is not fatiguing.

Playtest question:

- Does the complete game feel cohesive without losing readability?
