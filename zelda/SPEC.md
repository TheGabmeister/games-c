# SPEC.md

# Modern 2D recreation of The Legend of Zelda

## Intent

Build a modernized top-down 2D action-adventure inspired by the 1986 classic
The Legend of Zelda.

This project should preserve the feel of the original gameplay loop while using
original art direction, original layouts, and original audio. The goal is not a pixel-perfect remake, it recreates the gameplay mechanics while modernizing the graphics.

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

Modern quality-of-life targets:

- Buffered sword/item input for a few frames.
- Brief invulnerability after damage.
- Clear hit flashes, knockback, shield deflect effects, and low-health feedback.
- Optional restart/continue from dungeon entrance after defeat.
- Save and continue support for player progress.

## Movement and Collision Model

Screen and tile geometry:

- Logical resolution: 256x240 pixels, matching the NES aspect ratio. The
  renderer scales this uniformly to fill the window, letterboxing if needed.
- Tile size: 16x16 logical pixels.
- HUD: top strip, 256x56 pixels (16 tiles wide, 3.5 tiles tall). Displays
  hearts, rupees, bombs, keys, equipped item, and dungeon map indicator.
- Play area: 256x176 pixels (16 tiles wide, 11 tiles tall). This is one
  screen of the overworld or one room of a dungeon.
- Player sprite: 16x16 pixels (one tile). The player's hitbox matches the
  sprite.
- Overworld: 16x8 grid of screens. Each screen is 16x11 playable tiles.
  Total overworld tile area: 256x88 tiles.
- Dungeon rooms: same 16x11 tile dimensions as overworld screens. Dungeon
  layouts are grids of rooms (up to 8x8 rooms per dungeon).

Movement:

- The world is built on a tile grid. The player always occupies exactly one
  tile and cannot stop between tiles.
- Pressing a direction begins a smooth slide to the adjacent tile center.
  The slide is purely visual — the player's logical tile changes at the start
  of the move, and the sprite catches up over a short duration.
- Movement speed is measured in tiles per second. Base speed should feel brisk
  but readable — roughly four to five tiles per second.
- Only one direction at a time. Pressing a new direction while sliding queues
  it and executes when the current slide finishes.
- Facing updates immediately on input, even if movement is blocked by a wall
  or obstacle.

Collision:

- Tile collision (walls, water, pits) uses the logical tile grid. A tile is
  passable or impassable. The player cannot enter impassable tiles. Logical
  tile position is only used for terrain checks.
- Combat collision (sword, projectiles, enemy contact) uses hitbox overlap on
  actual sprite positions, not tile occupancy. This applies to all damage
  sources including contact damage — an enemy and player moving toward each
  other take contact damage when their sprites overlap, not when their
  logical tiles match.
- The sword hitbox is a rectangle extending from the player's sprite in the
  facing direction. It is active for a fixed number of frames per swing.
- Enemy hitboxes are rectangles matching their sprite bounds. Contact damage
  occurs when the player's hitbox overlaps an enemy's hitbox.
- Projectile hitboxes are small rectangles that travel in a straight line and
  check overlap each frame.
- Knockback from damage pushes the player or enemy a fixed distance in the
  hit direction, ignoring the tile grid for the slide but snapping back to
  the nearest valid tile center when the knockback ends.

## Health, Damage, and Recovery

- Player health is measured in heart containers. Each heart has two halves,
  so damage and recovery can be half-heart granular.
- The player starts with three hearts.
- Heart containers increase maximum health by one heart.
- Small hearts restore partial health.
- Fairy encounters restore a large amount or full health.
- Potions restore all health and can have one-use or two-use variants.
- Defensive rings/tunics reduce incoming damage.
- At full health, sword attacks may fire a ranged beam.

Death/defeat behavior:

- Overworld defeat returns the player to the starting area or last safe continue
  point with retained permanent progress.
- Dungeon defeat returns the player to that dungeon entrance with retained
  permanent progress.
- The player restarts with three hearts after defeat. Consumable quantities
  (bombs, arrows) are retained. Keys collected and doors opened persist.
- Consumable quantities and current health after continue should follow the
  classic feel: forgiving enough to keep exploring, not so generous that danger
  vanishes.

## Inventory and Items

Items should be divided into permanent equipment, active items, dungeon items,
consumables, and quest relics.

### Permanent Equipment

- Basic sword: starting weapon, short melee attack.
- Strong sword: mid-game upgrade gated by maximum health.
- Master sword equivalent: late-game upgrade gated by higher maximum health.
- Small shield: blocks simple frontal projectiles.
- Large shield: blocks stronger projectiles and magic attacks.
- Blue armor/ring: halves incoming damage.
- Red armor/ring: reduces incoming damage further.
- Strength bracelet: allows pushing or moving heavy overworld objects.

### Active Items

- Boomerang: stuns many enemies, retrieves drops, damages weak foes.
- Long boomerang: faster or farther boomerang upgrade.
- Bombs: damage enemies and reveal cracked or suspicious walls.
- Bow: fires arrows when arrows or currency are available.
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

### Traversal Items

- Raft: launches from docks to cross water routes.
- Ladder: crosses one-tile gaps, rivers, or dungeon pits.
- Strength bracelet: doubles as a traversal item for heavy rocks.
- Recorder/flute: reveals certain entrances and enables fast travel.

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

Door and room persistence:

- Locked doors stay open permanently once unlocked (persists through death
  and save/load).
- Shutter doors (lock until all enemies are defeated) reset on room re-entry
  and enemies respawn.
- Bombable walls stay revealed permanently once bombed.
- Pushed blocks reset on room re-entry, but any stairs they revealed remain
  accessible.

### Currency and Drops

- Rupees/gems: money for shops, arrows, hints, and services.
- Heart drops: health recovery.
- Fairy drops: rare full or large recovery.
- Clock/time-freeze drop: temporarily freezes enemies on the current screen.
- Bomb drops: replenish bombs.

### Item Acquisition Summary

Dungeon rewards are listed in each dungeon's progression entry above. Items
not found in dungeons are acquired as follows:

- Basic sword: found in the starting cave before any dungeon.
- Strong sword: upgrade cave, requires five or more heart containers.
- Master sword equivalent: upgrade cave, requires ten or more heart
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
- Bombs: available from the start (starting supply of eight). Additional
  bombs from shops, drops, and caves.
- Bow: found in Dungeon 1. Requires rupees to fire until an optional quiver
  upgrade is found.

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
  The bow is useless without currency or arrow drops.
- The player starts with eight bombs. Bomb capacity upgrades (up to sixteen)
  should exist and be optional, found in caves or purchased.
- Arrow quiver starts unlimited (uses rupees). An optional quiver upgrade
  allows carrying arrows as a separate resource.
- Prices should make early purchases meaningful without requiring grinding.

Drop system:

- Defeated enemies have a chance to drop one item: rupees (common), hearts
  (uncommon), bombs (rare), fairy (very rare), or clock/time-freeze (very
  rare).
- Drop rates should use a fixed table based on enemy type and a kill counter,
  not pure randomness. This prevents streaks of no drops and guarantees
  resource flow during combat-heavy rooms.
- Destructible environment objects (shrubs, pots) always drop a fixed reward
  or nothing — no random table.
- Bosses always drop a heart container. Mini-bosses always drop a useful
  reward (key, bombs, or large rupee).

## Combat Model

Combat should be deterministic and pattern-based.

Player combat:

- Sword has a short active arc or thrust in the facing direction.
- Sword beam fires only at full health.
- Shield blocks from the facing direction while idle.
- Items create alternative strategies but do not replace sword fundamentals.
- Damage knockback should be short and predictable.

Enemy combat:

- Enemies move in readable patterns.
- Tough enemies have directional weaknesses or timing windows.
- Projectiles should be clear, bright, and slower than unfair reaction speed.
- Rooms can lock until all enemies are defeated.
- Some enemies should split, multiply, teleport, hide, or steal equipment.

Enemy spawning:

- Overworld enemies respawn every time the player re-enters a screen.
- Dungeon enemies respawn when the player re-enters a room, except for
  mini-bosses and bosses which stay defeated.
- Shutter rooms (doors lock until all enemies are dead) re-lock and respawn
  enemies on re-entry. Rooms cleared by pushing a block to reveal stairs do
  not respawn those enemies.
- Boss rooms remain empty after the boss is defeated.

## Tuning Defaults

All values are first-pass defaults at 60 FPS. Tune from playtesting.

Timing (in frames at 60 FPS):

- Input buffer window: 6 frames (100ms). A sword or item press within this
  window before the previous action ends queues the next action.
- Sword swing active frames: 8 frames (~133ms). The hitbox is live during
  this window.
- Invulnerability after damage: 60 frames (1 second). Player flashes and
  cannot take further damage.
- Knockback duration: 8 frames. Player or enemy slides during this time.
- Knockback distance: half a tile (8 logical pixels).
- Boss attack tell: minimum 20 frames (~333ms) of visible windup before any
  major attack.

Damage (in half-hearts):

- Basic sword: 1. Strong sword: 2. Master sword: 4. Sword beam: 1.
- Bombs: 4.
- Arrows: 2. Silver arrow: instant kill on final boss.
- Boomerang: 0 (stun only). Long boomerang: 0 (longer stun).
- Magic rod: 2. Magic rod with spell book: 2 + fire damage over time.

Enemy damage tiers (contact / projectile):

- Tier 1 (slime, bat, hopper): 1 / 1.
- Tier 2 (skeleton, snake, spear thrower): 2 / 2.
- Tier 3 (knight, mage, mountain guard): 4 / 4.
- Bosses: 2-4 depending on the attack.
- Blue armor halves all incoming damage. Red armor quarters it.

Enemy health (in basic sword hits to kill):

- Tier 1: 1 hit. Tier 2: 2 hits. Tier 3: 4 hits.
- Mini-bosses: 8 hits. Bosses: 12-16 hits.

Speeds (in tiles per second):

- Player movement: 4.
- Slow enemy (slime, mummy): 1-2.
- Normal enemy (skeleton, snake): 2-3.
- Fast enemy (charging snake, centipede): 5-6.
- Player projectiles (arrow, sword beam, magic rod): 8.
- Enemy projectiles (rocks, spears, magic): 3-4.
- Boomerang: 6 outbound, 6 return.

Drop table (kill-counter cycle, repeats every 10 kills):

- Kill 1: rupee (1). Kill 2: nothing. Kill 3: rupee (1). Kill 4: heart.
  Kill 5: rupee (1). Kill 6: nothing. Kill 7: bomb. Kill 8: heart.
  Kill 9: rupee (5). Kill 10: nothing.
- Every 40th kill: fairy instead of the normal drop.
- Clock/time-freeze: replaces the rupee(5) drop once per 100 kills.

## Enemy Roster

Use original-inspired roles with modernized names and visuals. Names below are
mechanical references, not a requirement for shipped presentation.

### Overworld Enemies

- Rock spitter: stationary or wandering enemy that fires rocks.
- Hopper: jumping enemy with red/blue speed variants.
- Burrower: emerges from ground, chases briefly, then dives.
- Spear thrower: forest enemy that throws linear projectiles.
- Flying flower/seed: invulnerable while flying, vulnerable when landed.
- Living statue: appears inert until touched or approached.
- Ghost: graveyard enemy; touching graves may spawn extras.
- Mountain guard: strong late-game ranged enemy.
- River shooter: emerges from water and fires projectiles.
- Rolling boulder: environmental hazard in mountains.

### Dungeon Enemies

- Slime: splits into smaller slimes when hit.
- Bat: small erratic flyer.
- Charging snake: waits, then rushes in a straight line.
- Skeleton: simple melee enemy, some variants throw projectiles.
- Boomerang imp: throws boomerangs and catches them.
- Teleport mage: appears, fires magic, vanishes.
- Shield knight: blocks frontal sword attacks; vulnerable from side/back.
- Ear ghost: vulnerable to sound or arrows.
- Mummy: slow, high-health pressure enemy.
- Shield eater: grabs the player and can consume the large shield.
- Wall hand: emerges from walls and returns player to dungeon entrance if it
  grabs them.
- Blade trap: darts along straight lines when the player aligns with it.
- Statue turret: fires projectiles from fixed positions.
- Bubble/jinx orb: disables sword use temporarily or until cleansed.
- Segmented worm: body shrinks as segments are destroyed.
- Centipede: fast segmented enemy; head may be armored.

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

- Every boss must have a readable tell before major attacks.
- Every boss room must fit on one screen.
- Boss weaknesses should be discoverable via NPC hints, dungeon item placement,
  or visual language.
- Bosses should become more intense as health drops, but not random.

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
- Light dark rooms with the candle/fire tool. Dark rooms render only a small
  radius around the player until lit. Lighting is permanent for the current
  dungeon visit. Enemies still move and attack in the dark.
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
exited, bombs are available from the start (no bomb-gated dungeon entrance
traps the player), and all boss-required items are either found within the
same dungeon (D5 recorder, D9 silver arrow) or obtainable from shops/earlier
dungeons.

## Difficulty Curve

- Early game: simple movement, low projectile density, short dungeons.
- Mid game: item gates, dark rooms, shielded enemies, stronger projectiles.
- Late game: teleporting enemies, equipment-stealing threats, multi-boss rooms,
  maze navigation, and higher damage.
- Final game: tests full mastery of combat, resource management, secret reading,
  and item usage.

The game should allow brave players to sequence-break some optional upgrades,
but it should not require obscure knowledge to finish.

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

Feedback:

- Secret reveals should have a satisfying visual/audio cue.
- New items should briefly show a name and icon.
- Damage, block, stun, and invulnerability states should be visually clear.
- Boss phase changes should be noticeable.

HUD:

- Top or side bar showing: current hearts, rupee count, bomb count, equipped
  item icon, and current key count (in dungeons).
- The HUD should not overlap the play area. Reserve a strip for it and size
  the play area to fill the remainder.
- Low-health indicator: flashing hearts or audio cue when at one heart or
  below.
- Dungeon map and compass indicator when those items have been collected for
  the current dungeon.

Inventory/pause screen:

- One equipped-item slot. The player opens the inventory to swap which active
  item is assigned to the use button.
- Grid of collected items with clear icons. Items not yet found are blank or
  silhouetted.
- Current sword, shield, armor, and ring shown in an equipment section.
- Dungeon map view when a map has been collected, with compass marker if the
  compass has been collected.
- Three save slots with file select at the title screen.

Quality of life:

- Save and continue support. Auto-save on dungeon entry, screen transition,
  and item pickup.
- Optional map viewing for discovered overworld screens.
- Clear distinction between discovered, hinted, and unexplored dungeon areas.
- No long unskippable text.

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

## Content Scope

Minimum complete game:

- Full overworld with all required regions.
- 8 fragment dungeons.
- Final dungeon.
- Complete item set needed for progression.
- Full enemy roster needed to support dungeon variety.
- All bosses listed above.
- Save/continue.
- Shops, hints, caves, secrets, and heart upgrades.
- Original visual and sound assets.

Possible stretch goals:

- Second Quest mode with remixed dungeon layouts, changed entrances, harder
  enemy placements, and altered item order.
- Optional modern map pins or notes.
- Accessibility toggles for damage, flashing effects, and input buffering.
- Hero mode after completion.
- Time trials or boss rush.

## Out of Scope for the First Full Version

- Side-scrolling areas.
- RPG leveling or experience points.
- Procedural world generation.
- Multiplayer.
- Complex dialogue trees.
- Crafting.
- Physics-heavy puzzles.
- Pixel-perfect recreation of the 1986 map, sprites, music, text, or exact room
  layouts.

## Research Notes

The original reference game uses:

- A main screen for action and a sub screen for treasure selection.
- Sword on the primary action and item use on the secondary action.
- Overworld exploration across forests, lakes, mountains, caves, shops, NPCs,
  and hidden entrances.
- Underworld labyrinths containing more enemies, maps, compasses, keys, locked
  doors, hidden treasures, and relic fragments.
- Items such as boomerangs, bombs, bow/arrows, candles, recorder, bait, potion,
  magic rod, raft, ladder, rings, bracelet, map, compass, keys, and magic key.
- Eight fragment dungeons followed by a ninth final dungeon.
- Dungeon names/themes in the first quest: Eagle, Moon, Manji, Snake, Lizard,
  Dragon, Demon, Lion, and Death Mountain.
- Bosses including Aquamentus, Dodongo, Manhandla, Gleeok, Digdogger, Gohma, and
  Ganon.

Primary research links:

- NES instruction manual transcription:
  https://www.world-of-nintendo.com/manuals/nes/legend_of_zelda.shtml
- Items in The Legend of Zelda:
  https://zeldawiki.wiki/wiki/Items_in_The_Legend_of_Zelda
- Enemies in The Legend of Zelda:
  https://zeldawiki.wiki/wiki/Enemies_in_The_Legend_of_Zelda
- Dungeons in The Legend of Zelda:
  https://www.zeldadungeon.net/wiki/The_Legend_of_Zelda_Dungeons
- Level 9 / final dungeon notes:
  https://zeldawiki.wiki/wiki/Level_9
- Recorder item behavior:
  https://www.zeldadungeon.net/wiki/Recorder
