# SPEC.md

# Modern 2D recreation of The Legend of Zelda

## Intent

Build a modernized top-down 2D action-adventure inspired by the 1986 classic
The Legend of Zelda.

This project should preserve the feel of the original gameplay loop while using
original art direction, original layouts, and original audio. The goal is not a pixel-perfect remake, it recreates the gameplay mechanics while modernizing the graphics.

## World Structure

The overworld should be a grid of connected screens with hard edges, readable
biomes, and one-screen combat spaces. Scrolling or transitions happen when the
player exits a screen edge.

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

## Health, Damage, and Recovery

- Player health is measured in heart containers.
- The player starts with a small number of hearts.
- Heart containers increase maximum health.
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
- Candle/fire tool: lights dark rooms, burns shrubs, damages enemies.
- Red candle/fire upgrade: reusable fire tool.
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

- Small key: opens one locked dungeon door.
- Magic key: opens locked doors indefinitely after found.
- Dungeon map: reveals room layout for the current dungeon.
- Compass: marks the relic/boss objective in the current dungeon.
- Relic fragment: one of eight pieces needed to unlock the final dungeon.

### Currency and Drops

- Rupees/gems: money for shops, arrows, hints, and services.
- Heart drops: health recovery.
- Fairy drops: rare full or large recovery.
- Clock/time-freeze drop: temporarily freezes enemies on the current screen.
- Bomb drops: replenish bombs.

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

- Currency cap should create spending decisions.
- Arrows may cost currency per shot to preserve the classic economy pressure.
- Bomb capacity upgrades should exist and be optional.
- Prices should make early purchases meaningful without requiring grinding.

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
- Sound-sensitive urchin: must be weakened or split with the recorder/flute.
- Eye crab: armored except when its eye opens; arrows are the intended answer.
- Repeat guardian variant: harder version of an earlier boss with new hazards.
- Late multi-boss gauntlet: combines several familiar bosses before the final
  fragment.
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
- Theme: mossy ruins and patrolling knights.
- Main item: raft.
- New concepts: shielded enemies, more locked-route choices.
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
- Entrance requirement: fire tool.
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

Quality of life:

- Save slots or at least persistent save/continue.
- Inventory screen with clear item selection.
- Optional map viewing for discovered overworld screens.
- Clear distinction between discovered, hinted, and unexplored dungeon areas.
- No long unskippable text.

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
