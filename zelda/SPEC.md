# SPEC.md

# Modern 2D recreation of The Legend of Zelda

## Intent

Build a modernized top-down 2D action-adventure inspired by the 1986 classic
The Legend of Zelda.

This project should preserve the feel of the original gameplay loop while using original art direction, original layouts, and original audio. The goal is not a pixel-perfect remake, it recreates the gameplay mechanics while modernizing the graphics.

## World Structure

The overworld should be a grid of connected screens with hard edges, readable
biomes, and one-screen combat spaces. The grid should be approximately 16
columns by 8 rows (128 screens). Not every cell needs unique content — some can
be filler or blocked — but the playable area should feel large enough to
explore and get lost in.

When the player exits a screen edge, the view scrolls in the direction of
travel to reveal the adjacent screen. The scroll should take roughly half a
second and lock player input until complete. Caves, dungeon entrances, and
stair passages use an instant fade-to-black transition instead.

Required overworld regions:

- Starting field: low danger, first cave, basic enemies, clear routes.
- Forest: projectile enemies, hidden shops, burnable secrets, maze path.
- Lake and river: water hazards, raft docks, ladder crossings, aquatic enemies.
- Mountains: tougher enemies, bombable walls, final dungeon foreshadowing.
- Graveyard: ghosts, pushable graves, high-risk optional rewards.
- Desert or wasteland: fast enemies, rock hazards, hidden cave entrances.
- Coast or southern marsh: traversal-item checks and heart secrets.
- Lost woods / lost hills equivalent: navigation puzzle that loops incorrectly
  unless the player knows the route or has the correct clue.

Required overworld content:

- 9 dungeon entrances: 8 fragment dungeons plus the final dungeon.
- At least 12 caves with NPC hints, gifts, shops, or upgrades.
- At least 5 optional heart containers outside dungeons.
- At least 3 fairy/healing locations.
- Multiple bombable walls, burnable shrubs, pushable stones, and revealed
  staircases.
- A few "pay for information" NPCs with useful but terse hints.
- At least one secret that requires returning with each major traversal item.

## Controls and Player Actions

The player should have four-direction movement and facing.

Core actions:

- Move north, south, east, and west.
- Attack with sword.
- Use equipped item.
- Block frontal projectiles while not attacking if a shield is equipped.
- Pick up drops and shop items by walking into them.
- Push specific blocks, stones, or grave markers.
- Transition between screens, caves, stair passages, and dungeons.
- Pause/open inventory to choose the equipped item.

Control schemes (keyboard and gamepad supported simultaneously):

- Movement: WASD or arrow keys; left stick or d-pad on gamepad.
- Attack (sword): Space or left-click; gamepad face button south (A / Cross).
- Use equipped item: Left Shift or right-click; gamepad face button west
  (X / Square).
- Pause/inventory: Escape or P; gamepad Start.
- Confirm (menus): Enter or Space; gamepad face button south.
- Cancel (menus): Escape; gamepad face button east (B / Circle).
- Analog stick deadzone: 0.5. Input beyond the deadzone is treated as full
  digital direction (no variable speed).

## Movement and Collision Model

Screen and tile geometry:

- Window and logical resolution: 1024x960 pixels, rendered 1:1 with no
  scaling. Sprites are authored at display size.
- Tile size: 64x64 pixels.
- HUD: top strip, 1024x224 pixels (16 tiles wide, 3.5 tiles tall). Displays
  hearts, rupees, bombs, keys, equipped item, and dungeon map indicator.
- Play area: 1024x704 pixels (16 tiles wide, 11 tiles tall). This is one
  screen of the overworld or one room of a dungeon.
- Player sprite: 64x64 pixels (one tile). The player's hitbox matches the
  sprite.
- Dungeon rooms: same 16x11 tile dimensions as overworld screens. Dungeon
  layouts are grids of rooms (up to 8x8 rooms per dungeon).

Movement:

- The world is built on a tile grid, but player movement is continuous in
  pixel space rather than tile-stepped. The player can stop between tile
  centers.
- Movement speed is measured in pixels per second. See Tuning Defaults for
  concrete values.
- Movement is four-directional only. Diagonal input resolves to one direction
  using the most recently pressed direction.
- Facing updates immediately on directional input, even if movement is blocked
  by a wall or obstacle.
- The player should feel grid-aware when navigating narrow passages, but the
  movement model should preserve the loose pixel-position feel of the original
  game.

Collision:

- Tile collision (walls, water, pits) uses the tile grid, but checks the
  player's pixel hitbox against impassable tile rectangles. The player cannot
  overlap impassable tiles.
- Combat collision (sword, projectiles, enemy contact) uses hitbox overlap on
  actual sprite positions, not tile occupancy. This applies to all damage
  sources including contact damage — an enemy and player moving toward each
  other take contact damage when their sprites overlap, not when they occupy
  the same terrain tile.
- The sword hitbox is a rectangle extending from the player's sprite in the
  facing direction. It is active for a fixed number of frames per swing.
- Enemy hitboxes are rectangles matching their sprite bounds. Contact damage
  occurs when the player's hitbox overlaps an enemy's hitbox.
- Projectile hitboxes are small rectangles that travel in a straight line and
  check overlap each frame.

Collision layers:

- Player ↔ Terrain: blocked by impassable tiles (tile-grid check).
- Player ↔ Enemy body: contact damage to player.
- Player ↔ Enemy projectile: damage to player, unless shield faces the
  projectile.
- Player ↔ Pickup: collect on overlap.
- Player weapon ↔ Enemy body: damage to enemy.
- Player weapon ↔ Terrain: stopped (arrows, sword beam, magic rod hit walls).
- Enemy ↔ Terrain: blocked for most enemies. Bats, ghosts, and flying
  enemies ignore walls.
- Enemy ↔ Enemy: no collision. Enemies overlap freely.
- Enemy projectile ↔ Terrain: stopped.
- Player projectile ↔ Enemy projectile: pass through each other.
- Bomb blast ↔ Bombable wall: reveal permanently.

- Knockback from damage pushes the player or enemy a fixed distance in the
  hit direction while respecting impassable terrain. Knockback does not snap
  the player to tile centers.

## Health, Damage, and Recovery

- Player health is measured in heart containers. Each heart has two halves,
  so damage and recovery can be half-heart granular.
- The player starts with three hearts.
- Heart containers increase maximum health by one heart.
- Small hearts restore partial health.
- Fairy encounters restore a large amount or full health.
- Potions restore all health and can have one-use or two-use variants.
- Defensive rings/tunics reduce incoming damage.

Death/defeat behavior:

- Overworld defeat returns the player to the starting screen.
- Dungeon defeat returns the player to that dungeon's entrance room.
- Health resets to three hearts. All permanent progress is retained.
- See Save Data Contract for the full list of what persists.

## Inventory and Items

Items should be divided into permanent equipment, active items, dungeon items,
consumables, and quest relics.

Pause screen:

- One equipped-item slot. The player opens the inventory to swap which active
  item is assigned to the use button.
- Grid of collected items. Items not yet found show as empty slots.
- Current sword, shield, armor, and ring shown in an equipment section.
- Dungeon map view when a map has been collected, with compass marker if the
  compass has been collected.
- Three save slots with file select at the title screen.

### Permanent Equipment

- Basic sword: starting weapon, short melee attack.
- Strong sword: mid-game upgrade gated by maximum health.
- Master sword equivalent: late-game upgrade gated by higher maximum health.
- Small shield: blocks simple frontal projectiles.
- Large shield: blocks stronger projectiles and magic attacks.
- Blue armor/ring: reduces incoming damage.
- Red armor/ring: reduces incoming damage further. See Tuning Defaults for
  multipliers.
- Strength bracelet: allows pushing or moving heavy overworld objects.

### Active Items

- Boomerang: stuns many enemies and damages weak foes.
- Long boomerang: faster or farther boomerang upgrade.
- Bombs: damage enemies and reveal cracked or suspicious walls.
- Bow: fires arrows only after the arrow item has been bought. Each shot costs
  one rupee.
- Silver/light arrow: required to finish the final boss.
- Candle/fire tool: lights dark rooms, burns shrubs, damages enemies. Limited
  to one use per screen; resets on screen transition.
- Red candle/fire upgrade: unlimited uses per screen.
- Recorder/flute: affects sound-sensitive enemies, reveals hidden entrances,
  and may create fast-travel whirlwinds to completed dungeons.
- Food/bait: distracts specific enemies and solves at least one dungeon gate.
- Magic rod: fires magical projectiles.
- Spell book: upgrades the magic rod with fire effects.
- Potion: restores health.
- Letter/prescription: unlocks potion shops or healer services.

### Dungeon Items

- Small key: opens one locked dungeon door. Keys are global — a key found in
  any dungeon can be used in any other dungeon. Keys are consumed on use and
  persist through death. This allows players to stockpile keys from easier
  dungeons.
- Magic key: opens locked doors indefinitely after found. Replaces the need
  for small keys entirely.
- Dungeon map: reveals room layout for the current dungeon. Per-dungeon item;
  each dungeon has its own map to find.
- Compass: marks the relic/boss room in the current dungeon. Per-dungeon
  item; each dungeon has its own compass to find.
- Relic fragment: one of eight pieces needed to unlock the final dungeon.
  Fragments are permanent — never lost.

Persistence:

- Locked doors stay open permanently once unlocked (persists through death
  and save/load).
- Bombable walls stay revealed permanently once bombed.
- See Combat Model for enemy spawning and room reset rules.

### Item Acquisition Summary

Dungeon rewards are listed in each dungeon's progression entry above. Items
not found in dungeons are acquired as follows:

- Basic sword: found in the starting cave before any dungeon.
- Strong sword: upgrade cave, requires five or more heart containers.
- Master sword equivalent: upgrade cave, requires twelve or more heart
  containers.
- Small shield: purchased from item shops.
- Large shield: purchased from item shops at higher price, available after
  mid-game.
- Blue armor/ring: found in a hidden overworld cave.
- Red armor/ring: found in Dungeon 9 (Death Mountain).
- Strength bracelet: found in an overworld cave, gated behind bombs.
- Candle/fire tool: purchased from item shops.
- Food/bait: purchased from item shops.
- Letter/prescription: found in an overworld cave or gifted by an NPC. Once
  shown to the healer, it unlocks potion shops permanently and is consumed.
- Bombs: acquired from shops, drops, or gifts. The player starts with zero
  bombs.
- Bow: found in Dungeon 1.
- Arrow: purchased from item shops. Required to fire the bow; each shot costs
  one rupee.

## Economy and Shops

The overworld should contain merchants and cave NPCs.

Shop/service types:

- Item shop: shield, arrows, candle, food, keys.
- Potion shop: locked until the player shows the correct letter.
- Hint cave: charges currency for cryptic but useful information.
- Gift cave: grants currency, health, or an item.
- Penalty cave: charges money or health for trespassing, used sparingly.
- Upgrade cave: grants sword, armor, or capacity upgrade if requirements are met.

Economy expectations:

- Currency cap of 255 rupees. Creates spending decisions without punishing
  exploration.
- Arrows cost one rupee per shot, preserving the classic economy pressure.
  The bow is useless without both the arrow item and currency.
- Bomb capacity upgrades (up to sixteen) should exist and be optional, found
  in caves or purchased.
- Arrows use rupees directly rather than a separate arrow inventory.
- Prices should make early purchases meaningful without requiring grinding.

Drop system:

- Defeated enemies have a chance to drop one item: rupees (common), hearts
  (uncommon), bombs (rare), fairy (very rare), or clock/time-freeze (very
  rare).
- Drops should use hidden drop groups and counters inspired by the original
  game rather than a fully predictable public cycle. Different enemy groups
  can favor different resources, but outcomes should still feel uncertain.
- Destructible environment objects (shrubs, pots) use authored fixed rewards
  or original-style drop chances, depending on the object type and screen.
- Bosses always drop a heart container. Mini-bosses may guard a key, item,
  room unlock, or useful drop depending on the room.

## Combat Model

Combat should be deterministic and pattern-based.

Player combat:

- Sword has a short active arc or thrust in the facing direction.
- Sword beam fires only at full health.
- Items create alternative strategies but do not replace sword fundamentals.

Enemy combat:

- Enemies move in readable patterns.
- Tough enemies have directional weaknesses or timing windows.
- Projectiles should be clear, bright, and slower than unfair reaction speed.
- Rooms can lock until all enemies are defeated.
- Some enemies should split, multiply, teleport, hide, or steal equipment.

Enemy spawning:

- Overworld enemies respawn every time the player re-enters a screen.
- Dungeon rooms remember cleared enemies during the current dungeon visit.
  Cleared normal rooms generally remain empty until the player exits the
  dungeon or reloads a save, then reset. Rooms that are designed as repeat
  hazards may respawn explicitly.
- Shutter rooms unlock when cleared and remain open for the current dungeon
  visit. They reset after leaving the dungeon or loading a save unless the
  room contains a permanent progression event.
- Mini-bosses and bosses stay defeated permanently.
- Rooms cleared by pushing a block to reveal stairs do not respawn those
  enemies during the current dungeon visit.
- Boss rooms remain empty after the boss is defeated.

## Tuning Defaults

All values are first-pass defaults at 60 FPS. Tune from playtesting.

Timing (in frames at 60 FPS):

- Input buffer window: none for sword and item actions. Inputs are accepted
  when the player is able to act, preserving the stricter original feel.
- Sword swing active frames: 8 frames (~133ms). The hitbox is live during
  this window.
- Invulnerability after damage: 60 frames (1 second). Player flashes and
  cannot take further damage.
- Knockback duration: 8 frames. Player or enemy slides during this time.
- Knockback distance: half a tile (32 pixels).
- Boss attack tells should be readable through simple sprite state, movement,
  or positioning, but do not need modern explicit windup phases.

Damage (in half-hearts):

- Basic sword: 1. Strong sword: 2. Master sword: 4. Sword beam: 1.
- Bombs: 4.
- Arrows: 2. Silver arrow: instant kill on final boss.
- Boomerang: 0 (stun only). Long boomerang: 0 (longer stun).
- Magic rod: 2. Magic rod with spell book: 2 + fire damage over time.

Enemy damage and health (per-enemy values in Enemy Roster are authoritative):

- Tier 1 (slime, bat, hopper, snake): 1 half-heart contact. 1 hit HP.
- Tier 2 (skeleton, rock spitter, burrower, spear thrower, boomerang imp):
  1–2 half-hearts contact/projectile. 2–3 hits HP.
- Tier 3 (knight, mage, mountain guard, ghost, mummy, shield eater, ear
  ghost): 2–4 half-hearts contact. 4–10 hits HP.
- Blue/upgraded variants have higher HP and damage within their tier range.
- Mini-bosses: 8 hits. Bosses: 12-16 hits.
- Bosses deal 2-4 half-hearts depending on the attack.
- Blue armor halves all incoming damage. Red armor quarters it.

Speeds (in pixels per second):

- Player movement: 256.
- Slow enemy (slime, mummy): 64-128.
- Normal enemy (skeleton, snake): 128-192.
- Fast enemy (charging snake, centipede): 320-384.
- Player projectiles (arrow, sword beam, magic rod): 512.
- Enemy projectiles (rocks, spears, magic): 192-256.
- Boomerang: 384 outbound, 384 return.

Drop behavior:

- Use several hidden enemy drop groups, each with different odds for rupees,
  hearts, bombs, fairies, clocks, or no drop.
- Use an internal kill/drop counter to shape resource flow in the spirit of
  the original game, but do not expose a simple guaranteed cycle to players.
- Clock/time-freeze drops should remain rare and screen-local.

## Enemy Roster

Use original-inspired roles with modernized names and visuals. Names below are
mechanical references, not a requirement for shipped presentation.

Each entry specifies authoritative HP (basic sword hits) and damage
(half-hearts, contact / projectile). Speed categories (slow, normal, fast)
refer to the ranges in Tuning Defaults.

### Overworld Enemies

#### Rock spitter

- 1 tile. Red and blue variants.
- Movement: wanders in cardinal directions, random direction changes. Stops
  briefly to fire. Respects walls. Red: slow. Blue: normal speed.
- AI states: Wander → Stop (random direction change or fire) → Wander.
  Fires every 1–3 seconds in facing direction.
- Attack: rock projectile, straight line. Blocked by any shield.
- HP: red 1, blue 2.
- Damage: red 1 / 1, blue 1 / 1.

#### Hopper

- 1 tile. Red and blue variants.
- Movement: parabolic jumps to random positions. No walking. Ignores terrain
  during jumps — can land on impassable tiles. Red: short rest between jumps.
  Blue: longer rest between jumps.
- AI states: Rest (stationary) → Jump (arc to random position) → Rest.
  No player targeting.
- Attack: contact only.
- HP: 1 (both variants).
- Damage: 1 / —.
- Weakness: boomerang stuns mid-flight.

#### Burrower

- 1 tile. Red and blue variants.
- Movement: spends most time underground (invisible, invulnerable). Emerges
  near the player, moves briefly on surface for 2–3 seconds, then re-burrows.
  Red: emerges near player, chases in a straight line, slow speed. Blue:
  emerges at random positions, wanders randomly, normal speed.
- AI states: Submerged (invisible, invulnerable) → Emerging (brief animation,
  becoming vulnerable) → Active (moving on surface) → Submerging →
  Submerged. Cycle repeats every 2–4 seconds.
- Attack: contact only.
- HP: red 2, blue 4.
- Damage: red 1 / —, blue 2 / —.
- Immunity: invulnerable while submerged.

#### Spear thrower

- 1 tile. Red and blue variants.
- Movement: wanders cardinal directions. Random direction changes. Respects
  walls. Red: slow. Blue: normal speed.
- AI states: Patrol → Stop (fire spear in facing direction) → Patrol.
  Fires periodically.
- Attack: spear projectile, straight line. Blocked by any shield.
- HP: red 2, blue 3.
- Damage: red 1 / 1, blue 2 / 2.

#### Flying flower/seed

- 1 tile. No variants.
- Movement: alternates between flying (spinning, erratic movement over any
  terrain) and resting (stationary on ground). Flies for several seconds,
  gradually decelerates, rests briefly, then flies again.
- AI states: Rest (stationary, vulnerable) → Spinning Up (accelerating,
  becoming invulnerable) → Flying (moving, invulnerable) → Slowing
  (decelerating) → Rest.
- Attack: contact only while flying.
- HP: 2.
- Damage: 1 / —.
- Weakness: can only be damaged during Rest state.
- Immunity: invulnerable to all attacks while flying. Boomerang has no
  effect.

#### Living statue

- 1 tile. No variants.
- Movement: starts completely stationary, appears as terrain decoration.
  Activates permanently when the player touches or pushes against it. Once
  active, moves erratically in cardinal directions at normal speed. Never
  returns to inert state.
- AI states: Inert (stationary, blocks movement like terrain, immune to all
  damage) → Activated (touch triggers transition) → Active (erratic
  movement, permanently).
- Attack: contact only. Touching from front during activation guarantees
  taking damage.
- HP: 3. Bombs kill in 1 hit.
- Damage: 1 / —.
- Immunity: immune to all damage while inert. Some statues hide items or
  stairs beneath them.

#### Ghost

- 1 tile. No variants.
- Movement: one "main" ghost floats freely in cardinal directions at normal
  speed, drifting semi-randomly. Additional copies spawn when the player
  touches gravestones — copies also float randomly.
- AI states: Wandering (drifting, changing direction periodically). Spawned
  copies use the same behavior.
- Attack: contact only.
- HP: main ghost 9. Spawned copies: invulnerable.
- Damage: 2 / —.
- Weakness: killing the main ghost instantly destroys all spawned copies.
- Immunity: spawned copies cannot be damaged by any means. All ghosts float
  through walls and obstacles.

#### Mountain guard

- 1 tile. Red and blue variants.
- Movement: wanders cardinal directions at normal speed. Random direction
  changes. Respects walls. Found in mountain regions.
- AI states: Wander → Stop (fire sword beam in facing direction) → Wander.
  Fires frequently.
- Attack: sword beam projectile, straight line. Blocked only by large shield
  — small shield cannot block.
- HP: red 4, blue 6.
- Damage: red 2 / 2, blue 4 / 4.
- Strongest standard overworld enemy. Blue variant is the most dangerous
  non-boss enemy in the game.

#### River shooter

- 1 tile. No variants.
- Movement: exists only in water tiles. Surfaces at semi-random water
  position near the player, stays visible briefly, fires, then submerges and
  resurfaces elsewhere. Cannot be lured onto land.
- AI states: Submerged (invisible, invulnerable) → Surfacing (brief
  animation) → Surfaced (visible, fires, vulnerable) → Submerging →
  Submerged. HP resets to full on each submerge.
- Attack: fireball projectile aimed at player's current position. Blocked
  only by large shield.
- HP: 2 (resets on each dive — must kill in one surface window).
- Damage: 1 / 1.
- Weakness: strong sword or better kills in one hit, bypassing the HP-reset
  mechanic.
- Immunity: invulnerable while submerged.

#### Rolling boulder

- 1 tile. Environmental hazard, not a true enemy.
- Movement: falls from the top of the screen at random horizontal positions.
  Multiple boulders fall simultaneously. Continuous spawning while the player
  is on the screen.
- AI states: none. Continuous spawn-and-fall.
- Attack: contact only.
- HP: invulnerable. Cannot be destroyed.
- Damage: 1 / —.
- Falls through all terrain. Purely avoidance obstacle.

### Dungeon Enemies

#### Slime

- Large slime: 1 tile. Small slime: half tile (32x32 pixels).
- Movement: both sizes move in cardinal directions in short bursts — slide a
  short distance, pause, slide again. Random direction changes. Slow speed.
  No player targeting. Respects walls.
- AI states: Idle (brief pause) → Slide (short distance, random cardinal
  direction) → Idle.
- Attack: contact only.
- HP: large 1, small 1.
- Damage: large 1 / —, small 1 / —.
- Special: large slime splits into 2 small slimes when hit with basic sword.
  Strong sword and master sword kill outright without splitting. Weak weapons
  multiply enemies.

#### Bat

- 1 tile. Red and blue variants (red is faster).
- Movement: starts stationary when player enters room. Activates and flies in
  erratic, unpredictable patterns with frequent direction changes. Periodically
  slows and rests briefly before flying again. Flies through walls and all
  obstacles.
- AI states: Resting (stationary) → Flying (erratic movement) → Slowing →
  Resting. Activates on room entry.
- Attack: contact only.
- HP: 1 (both variants).
- Damage: 1 / —.
- Weakness: boomerang stuns mid-flight.

#### Charging snake

- 1 tile. No variants (flashing variant in later dungeons: 4 hits).
- Movement: wanders slowly in cardinal directions. When the player aligns on
  the same row or column, charges at fast speed in a straight line toward the
  player. Charge ends on wall contact. Respects walls.
- AI states: Patrol (slow random cardinal movement) → Detect (player enters
  same row or column — instant, no windup) → Charge (high-speed rush) →
  Patrol (after hitting wall or passing player).
- Attack: contact only via charge.
- HP: 1.
- Damage: 1 / —.
- Detection is instant with no telegraph.

#### Skeleton

- 1 tile. No variants.
- Movement: wanders cardinal directions at normal speed. Random direction
  changes. Respects walls.
- AI states: Wander → Direction change → Wander. Ranged variant adds:
  Wander → Stop (fire sword beam in facing direction) → Wander.
- Attack: contact only in standard form. Ranged variant fires sword beam
  projectiles blocked by any shield.
- HP: 2.
- Damage: 1 / 1.

#### Boomerang imp

- 1 tile. Red and blue variants.
- Movement: wanders cardinal directions at normal speed. Random direction
  changes. Respects walls.
- AI states: Wander → Stop (throw boomerang in facing direction) → Wait
  for return (boomerang arcs back) → Wander. Cannot throw again until
  boomerang returns.
- Attack: boomerang projectile. Damages on both outward and return paths.
  Blue variant's boomerang travels farther. Boomerang stopped by walls on
  outbound path.
- HP: red 3, blue 5.
- Damage: red 1 / 1, blue 2 / 2.
- Weakness: stunned by player's boomerang.

#### Teleport mage

- 1 tile. Red and blue variants with fundamentally different behavior.
- Red variant — movement: teleports to random positions. Appears briefly
  (~1–2 seconds), fires if player is in line of sight, then teleports away.
  Invisible and invulnerable between appearances.
- Red AI states: Invisible (invulnerable) → Appearing → Visible (fires if
  player on same row/column) → Disappearing → Invisible.
- Blue variant — movement: drifts continuously in erratic patterns. Passes
  through walls and obstacles. Always visible and active. Does not teleport.
- Blue AI states: Drifting (continuous erratic movement, fires repeatedly
  whenever player crosses line of sight).
- Attack: magic beam projectile, straight line. Blocked only by large shield.
  Red fires once per appearance. Blue fires continuously on line of sight.
- HP: red 4, blue 10.
- Damage: 2 / 2 (both variants).
- Immunity: red is invulnerable while invisible. Blue passes through walls.

#### Shield knight

- 1 tile. Red and blue variants.
- Movement: cardinal directions at normal (red) or fast (blue) speed.
  Frequent random direction changes. Respects walls. Continuous movement
  with no idle state.
- AI states: Patrol (move in cardinal direction) → Direction change → Patrol.
  No ranged attack — purely contact-based.
- Attack: contact only.
- HP: red 4, blue 8.
- Damage: red 2 / —, blue 4 / —.
- Weakness: can only be damaged from the side or rear.
- Immunity: frontal attacks blocked by shield. Immune to arrows, boomerang,
  candle, and magic rod from any direction. Bombs damage from side/rear only.

#### Ear ghost

- 1 tile. No variants.
- Movement: hops and bounces around the room in semi-random patterns, similar
  to hopper. Moderate speed. Respects walls.
- AI states: Idle (brief pause) → Jump (hop to random nearby position) →
  Idle.
- Attack: contact only.
- HP: 10 with basic sword. Arrows kill instantly (1 hit).
- Damage: 2 / —.
- Weakness: arrows are an instant kill. Recorder/flute is also effective.
  Extremely high HP makes sword combat impractical — arrows are the intended
  counter.

#### Mummy

- 1 tile. No variants.
- Movement: wanders cardinal directions at slow speed. Random direction
  changes. Respects walls.
- AI states: Wander → Direction change → Wander. Simplest AI.
- Attack: contact only.
- HP: 7.
- Damage: 2 / —.
- Pure HP sponge. No special mechanics.

#### Shield eater

- 1 tile. No variants.
- Movement: wanders cardinal directions at slow speed. Random, erratic.
  Respects walls.
- AI states: Wander → Engulf (on player contact, swallows player) →
  Consuming (2–3 second timer — if player does not kill it by attacking
  rapidly during this window, eats large shield) → Release.
- Attack: contact triggers engulf.
- HP: 9.
- Damage: 1 / —. Real threat is shield consumption.
- Special: permanently consumes the player's large shield if not killed
  during the engulf window. Only targets the large shield — small shield
  is unaffected. Player must re-purchase the large shield after loss.
  Often paired with teleport mages in later dungeons.

#### Wall hand

- 1 tile. No variants.
- Movement: emerges from room walls. Stores the player's position at spawn
  time and moves slowly in a straight line toward that stored position. Does
  not continuously track the player. Retreats to wall if it misses.
- AI states: Hidden (inside wall, invulnerable) → Emerging (slides out,
  becomes visible) → Moving (slow approach toward stored position) → Grab
  (if contact, teleports player to dungeon entrance) or Retreat (returns to
  wall on miss).
- Attack: grab teleports player to the first room of the current dungeon.
  Does not deal heart damage.
- HP: 2.
- Damage: grab effect only (no heart damage).
- Weakness: arrows effective at range. Killable before it reaches the player.
- Immunity: invulnerable while inside walls.

#### Blade trap

- 1 tile. Environmental hazard.
- Movement: sits at a fixed position (typically room corners). When the player
  aligns horizontally or vertically, darts at high speed in a straight line
  toward the player's axis. After reaching the far wall, retracts slowly
  (~1/3 dart speed) to its original position.
- AI states: Idle (fixed position) → Triggered (player aligns on same row or
  column) → Dart (high-speed linear rush) → Retract (slow return to origin)
  → Idle.
- Attack: contact during dart or retract phase.
- HP: invulnerable. Cannot be destroyed.
- Damage: 2 / —.
- Multiple traps can trigger simultaneously. Safe to cross during slow
  retract phase.

#### Statue turret

- 1 tile. Environmental hazard.
- Movement: completely stationary. Fixed position, typically flanking doorways
  or along walls. Never moves.
- AI states: Active (fires at regular intervals while room is occupied). No
  idle state.
- Attack: fireball projectile aimed at player or in statue's facing direction.
  Straight line. Blocked by large shield. Fires continuously at fixed
  intervals.
- HP: invulnerable. Cannot be destroyed.
- Damage: — / 1.
- Creates crossfire pressure while player fights other enemies.

#### Bubble/jinx orb

- 1 tile. Standard, red, and blue variants.
- Movement: bounces diagonally around the room, reflecting off walls. Constant
  speed, perpetual motion. Predictable diagonal reflection pattern.
- AI states: Bouncing (diagonal movement, wall reflection). No other states.
- Attack: contact does not deal heart damage. Instead disables the player's
  sword. Standard variant: disabled for ~4 seconds. Red variant: permanently
  disabled until cured by blue bubble contact, fairy, or potion. Blue variant:
  cures red bubble's curse on contact.
- HP: invulnerable. Cannot be destroyed.
- Damage: 0 half-hearts. Sword-disable curse on contact.
- Often placed alongside other enemies to compound difficulty.

#### Segmented worm

- Multi-tile: head + 4 body segments (5 tiles total). No variants.
- Movement: slithers in slow, winding random patterns. Body segments follow
  the head's path (snake-game chain). Respects walls.
- AI states: Slithering (continuous random winding movement). No other states.
- Attack: contact from any segment.
- HP: ~2 per segment (~10 total). Any segment can be hit — destroyed segments
  are removed from the chain.
- Damage: 1 / —.
- Body shortens as segments are destroyed. Segments can be killed in any
  order.

#### Centipede

- Multi-tile: head + 4 body segments (5 tiles total). Red and blue variants.
- Movement: crawls in winding patterns, similar to segmented worm but faster.
  Red: normal speed. Blue: fast speed. Respects walls.
- AI states: Crawling (continuous winding movement). No other states.
- Attack: contact from any segment.
- HP: ~2 per segment. Head is armored — can only be damaged after all body
  segments are destroyed. Must kill tail-to-head.
- Damage: red 1 / —, blue 2 / —.
- Immunity: head invulnerable until all body segments are destroyed.
  Forced tail-to-head destruction order (unlike segmented worm).

## Bosses

Each fragment dungeon should end with a boss that teaches or tests a specific
item, movement skill, or combat pattern. Bosses should drop a heart container
and guard a relic fragment.

Boss roster:

- Dragon guardian: fires beams from a fixed patrol; tests sword, bombs, bow, or
  dodging fundamentals.
- Armored beast: mostly immune to sword; bombs are the primary answer.
- Four-headed plant: loses heads as damaged and moves faster as it weakens.
- Multi-headed dragon: heads detach or continue attacking after being severed.
  Appears in escalating variants: two-headed (D4 boss), three-headed (D6
  mini-boss), and four-headed (D8 boss). More heads means faster attacks and
  more chaotic projectile patterns.
- Sound-sensitive urchin: must be weakened or split with the recorder/flute.
- Eye crab: armored except when its eye opens; arrows are the intended answer.
- Repeat guardian variant: harder version of an earlier boss with new hazards.
- Final boss: invisible or evasive dark lord; sword reveals/stuns, silver/light
  arrow finishes the fight.

Boss design requirements:

- Boss behavior should be learnable from simple repeated patterns, sprite
  states, movement, and positioning.
- Every boss room must fit on one screen.
- Boss weaknesses should be discoverable via NPC hints, dungeon item placement,
  or visual language.
- Bosses should rely on simple repeatable patterns rather than random phase
  escalation.

## Dungeon Progression

The game should have eight fragment dungeons and one final dungeon. The order
can be mostly open, but item gates should create a soft intended route.

### Dungeon 1: Forest Shrine

- Role: beginner dungeon.
- Theme: roots, stone, shallow water, simple locked doors.
- Main items: boomerang and bow.
- New concepts: map, compass, keys, locked rooms, shutter rooms, boss reward.
- Boss: dragon guardian.
- Reward: fragment 1 and heart container.

### Dungeon 2: Moonlit Den

- Role: early combat dungeon.
- Theme: caves, blue stone, narrow paths.
- Main item: long boomerang.
- New concepts: faster enemies, optional extra keys, bombable walls.
- Boss: armored beast.
- Reward: fragment 2 and heart container.

### Dungeon 3: Green Ruins

- Role: first difficulty bump.
- Theme: mossy ruins, flooded chambers, and patrolling knights.
- Main item: raft.
- New concepts: shielded enemies, water-blocked passages, more locked-route
  choices.
- Boss: four-headed plant.
- Reward: fragment 3 and heart container.

### Dungeon 4: Serpent Keep

- Role: first traversal-heavy dungeon.
- Theme: yellow stone, dark rooms, water gaps.
- Main item: ladder.
- New concepts: dark rooms, mid-dungeon item gate, enemy that eats shields.
- Mini-boss: four-headed plant.
- Boss: two-headed dragon.
- Reward: fragment 4 and heart container.

### Dungeon 5: Lizard Warrens

- Role: hidden entrance and item-weakness dungeon.
- Theme: northern hills, tougher monsters, statue turrets.
- Main item: recorder/flute.
- New concepts: sound puzzle, stronger enemy groups, bomb capacity upgrade.
- Mini-boss: armored beast.
- Boss: sound-sensitive urchin.
- Reward: fragment 5 and heart container.

### Dungeon 6: Dragon Catacombs

- Role: magic and high-threat enemy dungeon.
- Theme: graveyard/mountain catacombs, yellow stone, teleporting mages.
- Main item: magic rod.
- New concepts: magic projectiles, shield limitations, secret passages.
- Mini-boss: three-headed dragon.
- Boss: eye crab.
- Reward: fragment 6 and heart container.

### Dungeon 7: Demon Fountain

- Role: secret-access gauntlet.
- Theme: hidden water shrine, green stone, old traps.
- Main item: red candle/fire upgrade.
- Entrance requirement: recorder/flute.
- New concepts: repeated mini-bosses, bait gate, heavier secret use.
- Mini-bosses: sound-sensitive urchins, armored beasts, eye crab.
- Boss: stronger dragon guardian.
- Reward: fragment 7 and heart container.

### Dungeon 8: Lion Labyrinth

- Role: penultimate mastery dungeon.
- Theme: white stone, many knights, complex room graph.
- Main items: spell book and magic key.
- Entrance requirement: any fire tool (the base candle from shops is
  sufficient; the red candle from D7 is not required).
- New concepts: heavy combat, optional-but-powerful upgrades, many mini-bosses.
- Mini-bosses: four-headed plants and eye crabs.
- Boss: four-headed dragon.
- Reward: fragment 8 and heart container.

### Dungeon 9: Death Mountain

- Role: final dungeon.
- Theme: volcanic mountain fortress, skull/monster-head layout inspiration,
  strongest enemy combinations.
- Entrance requirement: all eight fragments.
- Main items: red armor/ring and silver/light arrow.
- New concepts: maze passages, false routes, highest enemy density, final key
  item hunt.
- Mini-bosses: orbiting eye swarms, repeat bosses, elite enemy rooms.
- Boss: final dark lord.
- Reward: ending rescue scene.

## Puzzle and Secret Vocabulary

The game should teach a small set of verbs and reuse them consistently:

- Burn shrubs or torches.
- Bomb cracked or suspicious walls.
- Push isolated stones, blocks, statues, and grave markers.
- Play recorder/flute at strange ponds, fountains, hills, or boss rooms.
- Launch raft from docks only.
- Cross narrow water/gap tiles with ladder.
- Light dark rooms with the candle/fire tool. Unlit rooms are globally dark,
  obscuring the room until lit; they do not use a moving player light radius.
  Lighting is permanent for the current dungeon visit. Enemies still move and
  attack in the dark.
- Defeat all enemies to open shutters.
- Push a block after clearing a room to reveal stairs.
- Use bait to satisfy or distract hungry gatekeepers.
- Use arrows against eye enemies.
- Use bombs against thick-skinned enemies.
- Use sound against sound-sensitive enemies.

## Progression Gates

Hard gates:

- Raft routes require raft.
- Ladder crossings require ladder.
- Heavy rocks require strength bracelet.
- Certain hidden entrances require recorder/flute.
- Certain bushes require fire.
- Bombable walls require bombs.
- Final dungeon requires all eight fragments.
- Final boss requires silver/light arrow.

Soft gates:

- Strong sword requires enough heart containers.
- Late overworld regions are survivable but dangerous before armor upgrades.
- Some dungeons can be entered early but are hostile without better gear.
- Potion shops require letter/prescription.
- Expensive items require economy engagement.

### Intended Route and Dependency Graph

Intended route: D1 → D2 → D3 → D4 → D5 → D6 → D7 → D8 → D9.

Hard dependencies (entrance or boss requires a specific item):

- D6 boss (eye crab) requires bow → bow is found in D1. D1 must be completed
  before D6 boss can be defeated.
- D7 entrance requires recorder → recorder is found in D5. D5 must be
  completed before D7 can be entered.
- D8 entrance requires any fire tool → the base candle is sold in shops from
  early game. No dungeon dependency.
- D9 entrance requires all eight fragments → all eight dungeons must be
  completed.
- D9 final boss requires silver/light arrow → silver/light arrow is found
  inside D9 before the boss room.

Minimum required ordering: D1 before D6, D5 before D7, all eight before D9.

Allowed sequence breaks:

- D2, D3, D4, D5 can be done in any order without D1, as long as the player
  returns to D1 before attempting D6's boss.
- D8 can be done before D7 (shop candle satisfies entrance).
- D6 can be entered and partially explored without the bow, but the boss
  cannot be defeated. The player can leave and return.
- Any dungeon can be entered, explored partially, and exited. Progress within
  a dungeon (opened doors, collected items) persists.

Softlock analysis: no softlocks are possible because dungeons can always be
exited, no bomb-gated dungeon entrance traps the player, and all boss-required
items are either found within the same dungeon (D5 recorder, D9 silver arrow)
or obtainable from shops/earlier dungeons.

## Modernization Targets

Visuals:

- Hand-drawn or high-resolution sprite style with strong silhouettes.
- Smooth movement and attack animations.
- Animated water, grass, fire, magic, and enemy tells.
- Distinct biome palettes instead of one-note color swaps.
- Clear item icons and pickup effects.
- Screen transitions with subtle polish.

Audio:

- Original music inspired by adventurous fantasy, not copied themes.
- Distinct jingles for secrets, item pickups, dungeon entry, boss defeat, and
  fragment collection.
- Readable combat sounds for sword, shield, damage, enemy defeat, bombs, arrows,
  fire, magic, and low health.

## Save Data Contract

Each save slot persists the following state:

Player state:

- Current and maximum heart count (half-heart precision).
- Rupee count, bomb count, bomb capacity, key count.
- Current equipped item.
- Current sword, shield, and armor tier.

Collected items:

- Bitfield of all permanent equipment and active items obtained.
- Per-dungeon: map collected, compass collected, boss defeated, fragment
  collected.

World state:

- Set of permanently opened doors (locked doors, by dungeon and room ID).
- Set of revealed bombable walls (by screen/room ID).
- Set of collected heart containers (by world location).
- Set of collected overworld items (caves, gifts, upgrades).
- Set of shop purchases that are one-time (upgrades, letter).
- NPCs whose one-time dialogue or gift has been triggered.

Position and context:

- Whether the player is in the overworld or a dungeon (and which one).
- On continue after death: overworld places the player at the starting
  screen; dungeon places the player at that dungeon's entrance room. Health
  resets to three hearts. All other state above is retained.

Not persisted (resets on death or save/load):

- Current screen scroll position.
- Shutter door state (resets per room entry).
- Pushed block positions (reset per room entry).
- Enemy positions and health.
- Active projectiles, drops, and timers.
- Clock/time-freeze effect.

## NPCs and Hints

NPC dialogue should be short, strange, and useful. Hints should point toward
verbs and regions rather than exact coordinates.

Hint categories:

- Item use: "Sound wakes what water hides."
- Dungeon location: "The fifth gate listens from the northern hills."
- Enemy weakness: "The armored eater fears smoke and thunder."
- Secret type: "Lonely stones move for strong hands."
- Economy: "The healer trusts written proof."
- Final dungeon: "Wisdom whole opens the mountain's mouth."

NPC roles:

- Old hermit: cryptic dungeon and item hints.
- Merchant: sells equipment and consumables.
- Healer: sells potions after letter/prescription.
- Fairy: restores health.
- Thief/trickster: takes money or health in dangerous caves.
- Gift-giver: rewards curiosity.

## Collectibles and Completion

Required completion:

- 8 relic fragments.
- Silver/light arrow.
- Final boss defeated.

Optional completion:

- All heart containers.
- All sword upgrades.
- All armor/ring upgrades.
- Magic key.
- Spell book.
- Maximum bomb capacity.
- All overworld secrets found.
- All dungeon maps and compasses collected.

## References

Sources used for enemy behavior, HP/damage values, and AI patterns:

- StrategyWiki — The Legend of Zelda enemies: structured enemy behavior and
  mechanics reference.
- Data Crystal (TCRF) — The Legend of Zelda RAM map: internal timer addresses,
  enemy action cooldowns, and engine-level behavior data.
- Zelda Dungeon Wiki — per-enemy gameplay pages with interaction details
  (shield blocking, vulnerability windows, special mechanics).
- RPG Classics (tartarus.rpgclassics.com) — The Legend of Zelda enemy tables:
  HP and damage values across all sword tiers.
