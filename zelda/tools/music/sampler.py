"""Play every instrument in the soundfont — C major arpeggio on each."""
from midiutil import MIDIFile
import os

PRESETS = [
    (0, "PIANO 1"), (1, "PIANO 2"), (2, "PIANO 3"), (3, "HONKY TONK PIANO"),
    (4, "E. PIANO 1"), (5, "E. PIANO 2"), (6, "HARPSICHORD"), (7, "CLAVINET"),
    (8, "VIBES 1"), (9, "VIBES 2"), (10, "VIBES 3"), (11, "VIBES 4"),
    (12, "VIBES 5"), (13, "VIBES 6"), (14, "VIBES 7"), (15, "VIBES 8"),
    (16, "ORGAN 1"), (17, "ORGAN 2"), (18, "ORGAN 3"), (19, "ORGAN 4"),
    (20, "ORGAN 6"), (21, "ORGAN 7"), (22, "ORGAN 8"), (23, "ORGAN 9"),
    (24, "GUITAR 1"), (25, "GUITAR 2"), (26, "GUITAR 3"), (27, "GUITAR 4"),
    (28, "GUITAR 5"), (29, "GUITAR 6"), (30, "GUITAR 7"), (31, "GUITAR 8"),
    (32, "BASS 1"), (33, "BASS 2"), (34, "BASS 3"), (35, "BASS 4"),
    (36, "BASS 5"), (37, "BASS 6"), (38, "BASS 7"), (39, "BASS 8"),
    (40, "STRINGS 1"), (41, "STRINGS 2"), (42, "STRINGS 3"), (43, "STRINGS 4"),
    (44, "STRINGS 5"), (45, "STRINGS 6"), (46, "STRINGS 7"), (47, "TIMPANI"),
    (48, "STRINGS 9"), (49, "STRINGS 10"), (50, "STRINGS 11"), (51, "STRINGS 12"),
    (52, "STRINGS 13"), (53, "STRINGS 14"), (54, "STRINGS 15"), (55, "ORCH. HIT"),
    (56, "TRUMPET 1"), (57, "TRUMPET 2"), (58, "TRUMPET 3"), (59, "TRUMPET 4"),
    (60, "TRUMPET 5"), (61, "TRUMPET 6"), (62, "TRUMPET 7"), (63, "TRUMPET 8"),
    (64, "TRUMPET 9"), (65, "TRUMPET 10"), (66, "TRUMPET 11"), (67, "TRUMPET 12"),
    (68, "CLARINET 1"), (69, "CLARINET 2"), (70, "CLARINET 3"), (71, "CLARINET 4"),
    (72, "FLUTE 1"), (73, "FLUTE 2"), (74, "FLUTE 3"), (75, "FLUTE 4"),
    (76, "FLUTE 5"), (77, "FLUTE 6"), (78, "FLUTE 7"), (79, "FLUTE 8"),
    (80, "SQUARE WAVE"), (81, "SQUARE WAVE 2"), (82, "DUTY CYCLE"),
    (83, "SQUARE WAVE 3"), (84, "SQUARE WAVE 4"), (85, "SQUARE WAVE 5"),
    (86, "SQUARE WAVE 6"), (87, "LEAD 1"),
    (88, "PAD 1"), (89, "PAD 2"), (90, "PAD 3"), (91, "PAD 4"),
    (92, "PAD 5"), (93, "PAD 6"), (94, "PAD 7"), (95, "PAD 8"),
    (96, "PAD 9"), (97, "PAD 10"), (98, "PAD 11"), (99, "PAD 12"),
    (100, "PAD 13"), (101, "PAD 14"), (102, "PAD 15"), (103, "PAD 16"),
    (104, "SITAR 1"), (105, "SITAR 2"), (106, "SITAR 3"), (107, "SITAR 4"),
    (108, "SITAR 5"), (109, "BAGPIPE"), (110, "FIDDLE"), (111, "SHENAI"),
    (112, "TINKLE BELL"), (113, "AGOGO"), (114, "STEEL DRUM"),
]

TEMPO = 120
BEATS_PER_INSTRUMENT = 4  # 1 bar each = 2 seconds at 120 BPM

midi = MIDIFile(1)
midi.addTempo(0, 0, TEMPO)

# C major arpeggio: C4, E4, G4, C5
arp = [60, 64, 67, 72]

for i, (prog, name) in enumerate(PRESETS):
    beat = i * BEATS_PER_INSTRUMENT
    midi.addProgramChange(0, 0, beat, prog)
    for j, pitch in enumerate(arp):
        midi.addNote(0, 0, pitch, beat + j, 0.9, 100)

total_secs = len(PRESETS) * BEATS_PER_INSTRUMENT / (TEMPO / 60)

out = os.path.join(os.path.dirname(__file__), "sampler.mid")
with open(out, "wb") as f:
    midi.writeFile(f)

print(f"Created {out} — {len(PRESETS)} instruments, {total_secs:.0f} seconds")
print()
print("Timestamp guide (each instrument = 2 sec):")
print("-" * 50)
for i, (prog, name) in enumerate(PRESETS):
    secs = i * BEATS_PER_INSTRUMENT / (TEMPO / 60)
    mins = int(secs // 60)
    sec = secs % 60
    print(f"  {mins}:{sec:04.1f}  #{prog:3d}  {name}")
