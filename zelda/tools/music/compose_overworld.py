"""Epic overworld theme — maximum 8-bit instrument variety across 5 sections."""
from midiutil import MIDIFile
import os

TEMPO = 140
midi = MIDIFile(8)
for t in range(8):
    midi.addTempo(t, 0, TEMPO)

# ── Track / channel assignments ──
MELODY, COUNTER, ARP, BASS_T, PAD, ACCENT, EXTRA, DRUM_T = range(8)
CH = [0, 1, 2, 3, 4, 5, 6, 9]

# ── Helpers ──
NT = {'C':0,'D':2,'E':4,'F':5,'G':7,'A':9,'B':11}
def N(n, o=4): return 12*(o+1) + NT[n]
W, H, DQ, Q, E = 4.0, 2.0, 1.5, 1.0, 0.5
def bar(n): return (n-1)*4

notes = []
def add(trk, pitch, beat, dur, vel=90):
    notes.append((trk, CH[trk], pitch, beat, dur, min(vel, 127)))

def mel(p, b, d, v=100): add(MELODY, p, b, d, v)
def ctr(p, b, d, v=72):  add(COUNTER, p, b, d, v)
def acc(p, b, d, v=78):  add(ACCENT, p, b, d, v)
def ext(p, b, d, v=65):  add(EXTRA, p, b, d, v)

def pc(trk, time, prog): midi.addProgramChange(trk, CH[trk], time, prog)
def vc(trk, time, v):    midi.addControllerEvent(trk, CH[trk], time, 7, v)

# ── Initial volumes ──
for t, v in [(MELODY,100),(COUNTER,72),(ARP,58),(BASS_T,85),
             (PAD,52),(ACCENT,75),(EXTRA,62),(DRUM_T,78)]:
    vc(t, 0, v)

# ══════════════════════════════════════════════════════════════
# INSTRUMENT PLAN — 20 unique timbres across 5 sections
#
#  Intro (1-4):   Vibes 1, Pad 1, Bass 1, Vibes 3 (glock)
#  A (5-12):      Square Wave, Flute 1, Guitar 1, Strings 9
#  B (13-20):     Trumpet 1, Strings 1, Harpsichord, Pad 3, Clarinet 1
#  C (21-28):     Square Wave, Trumpet 2, Organ 1, Strings 9, Orch Hit, Lead 1
#  Outro (29-32): Piano 1, Vibes 2, Pad 1, Timpani
# ══════════════════════════════════════════════════════════════

# Intro
pc(MELODY, 0, 8);      pc(PAD, 0, 88);       pc(BASS_T, 0, 32)
pc(EXTRA, 0, 10);      pc(ACCENT, 0, 47)

# A section
pc(MELODY, bar(5), 80);  pc(COUNTER, bar(5), 72);  pc(ARP, bar(5), 24)
pc(PAD, bar(5), 48);     vc(PAD, bar(5), 55)

# B section
pc(MELODY, bar(13), 56);  pc(COUNTER, bar(13), 40);  pc(ARP, bar(13), 6)
pc(PAD, bar(13), 90);     pc(EXTRA, bar(13), 68);    vc(EXTRA, bar(13), 58)

# C section
pc(MELODY, bar(21), 80);  pc(COUNTER, bar(21), 57);  pc(ARP, bar(21), 16)
pc(PAD, bar(21), 48);     pc(ACCENT, bar(21), 55);   pc(EXTRA, bar(21), 87)
vc(PAD, bar(21), 62);     vc(ARP, bar(21), 55);      vc(EXTRA, bar(21), 60)

# Outro
pc(MELODY, bar(29), 0);   pc(ARP, bar(29), 9);       pc(PAD, bar(29), 88)
pc(ACCENT, bar(29), 47)
vc(PAD, bar(29), 48);     vc(ARP, bar(29), 48);      vc(MELODY, bar(29), 85)

# ── Chord data ──
CD = {
    'C':  (N('C',3), N('G',3), (N('C',4), N('E',4), N('G',4))),
    'Am': (N('A',2), N('E',3), (N('A',3), N('C',4), N('E',4))),
    'F':  (N('F',2), N('C',3), (N('F',3), N('A',3), N('C',4))),
    'G':  (N('G',2), N('D',3), (N('G',3), N('B',3), N('D',4))),
    'Em': (N('E',3), N('B',3), (N('E',4), N('G',4), N('B',4))),
    'Dm': (N('D',3), N('A',3), (N('D',4), N('F',4), N('A',4))),
}

PROG = ['C','Am','F','G',
        'C','Am','F','G','C','Em','F','G',
        'Am','F','Dm','G','Em','Am','F','G',
        'C','Am','F','G','Am','F','G','C',
        'F','Am','G','C']

# ══════════════════════════════════════════════════════════════
# AUTO-GENERATED BACKING (bass, pads, arps, drums)
# ══════════════════════════════════════════════════════════════

# ── Drums ──
K, SN, HC, HO, CR = 36, 38, 42, 46, 49

def d_none(b): pass
def d_light(b):
    for i in range(4): add(DRUM_T, HC, b+i, E, 50)
def d_basic(b):
    add(DRUM_T, K, b, E, 92);    add(DRUM_T, HC, b+1, E, 55)
    add(DRUM_T, SN, b+2, E, 82); add(DRUM_T, HC, b+3, E, 55)
def d_drive(b):
    add(DRUM_T, K,  b,     E, 95); add(DRUM_T, HC, b+0.5, E, 48)
    add(DRUM_T, HC, b+1,   E, 58); add(DRUM_T, HC, b+1.5, E, 48)
    add(DRUM_T, SN, b+2,   E, 88); add(DRUM_T, HC, b+2.5, E, 48)
    add(DRUM_T, K,  b+3,   E, 78); add(DRUM_T, HC, b+3.5, E, 52)
def d_power(b):
    add(DRUM_T, K,  b,     E, 100); add(DRUM_T, HO, b,     E, 60)
    add(DRUM_T, HC, b+0.5, E, 52);  add(DRUM_T, K,  b+1,   E, 75)
    add(DRUM_T, HC, b+1.5, E, 55);  add(DRUM_T, SN, b+2,   E, 92)
    add(DRUM_T, HC, b+2.5, E, 52);  add(DRUM_T, K,  b+3,   E, 85)
    add(DRUM_T, HC, b+3.5, E, 55)
def d_fill(b):
    add(DRUM_T, SN, b,     E, 72);  add(DRUM_T, SN, b+0.5, E, 78)
    add(DRUM_T, SN, b+1,   E, 84);  add(DRUM_T, SN, b+1.5, E, 90)
    add(DRUM_T, CR, b+2,   Q, 95);  add(DRUM_T, K,  b+2,   E, 100)
    add(DRUM_T, K,  b+3,   E, 82)

d_seq = ([d_none]*4 +
         [d_basic,d_basic,d_drive,d_drive,d_basic,d_basic,d_drive,d_fill] +
         [d_drive]*3+[d_fill]+[d_drive]*3+[d_fill] +
         [d_power]*3+[d_fill]+[d_power]*3+[d_fill] +
         [d_light,d_light,d_light,d_none])

# Crash cymbals on section entrances
add(DRUM_T, CR, bar(5), Q, 90)
add(DRUM_T, CR, bar(13), Q, 88)
add(DRUM_T, CR, bar(21), Q, 95)

# ── Bass ──
def b_whole(r,f,b): add(BASS_T, r, b, W, 80)
def b_basic(r,f,b):
    add(BASS_T,r,b,Q,88);   add(BASS_T,r,b+1,Q,83)
    add(BASS_T,f,b+2,Q,85); add(BASS_T,r,b+3,Q,88)
def b_walk(r,f,b):
    add(BASS_T,r,b,Q,88);     add(BASS_T,f,b+1,Q,85)
    add(BASS_T,r+12,b+2,Q,83); add(BASS_T,f,b+3,Q,85)
def b_drive(r,f,b):
    for i,(p,v) in enumerate([(r,92),(r,82),(f,87),(f,82),
                               (r,92),(r,82),(f,87),(r,84)]):
        add(BASS_T, p, b+i*0.5, E, v)
def b_half(r,f,b): add(BASS_T,r,b,H,78); add(BASS_T,f,b+2,H,75)

b_seq = ([b_whole]*4 + [b_basic]*8 + [b_walk]*8 +
         [b_drive]*8 + [b_half]*4)

# ── Pads ──
def p_swell(tri,b):
    for p in tri: add(PAD, p, b, H, 42); add(PAD, p, b+2, H, 55)
def p_whole(tri,b):
    for p in tri: add(PAD, p, b, W, 55)

p_seq = ([p_swell]*4 + [p_whole]*8 + [p_swell]*8 +
         [p_whole]*8 + [p_swell]*4)

# ── Arpeggios ──
def a_none(tri,b): pass
def a_up(tri,b):
    r,t,f = tri
    for i,p in enumerate([r,t,f,t,r,t,f,t]): add(ARP, p, b+i*0.5, E, 55)
def a_spread(tri,b):
    r,t,f = tri
    for i,p in enumerate([r,f,t,r+12,f,t,r+12,f]): add(ARP, p, b+i*0.5, E, 55)
def a_power(tri,b):
    r,t,f = tri
    add(ARP,r,b,H,58);     add(ARP,f,b,H,53)
    add(ARP,r+12,b+2,H,58); add(ARP,f,b+2,H,53)
def a_gentle(tri,b):
    r,t,f = tri
    for i,p in enumerate([r,t,f,t]): add(ARP, p, b+i, Q, 48)

a_seq = ([a_none]*4 + [a_up]*8 + [a_spread]*8 +
         [a_power]*8 + [a_gentle]*4)

# Generate all auto parts
for i, ch in enumerate(PROG):
    b = bar(i+1)
    r, f, tri = CD[ch]
    d_seq[i](b); b_seq[i](r,f,b); p_seq[i](tri,b); a_seq[i](tri,b)

# ══════════════════════════════════════════════════════════════
# HAND-WRITTEN MELODIES
# ══════════════════════════════════════════════════════════════

# A section melody — the core theme (reused in C section)
# (pitch, beat_offset_from_section_start, duration, velocity)
A_MELODY = [
    # Bar 1 (C): Main motif — heroic ascending arpeggio
    (N('C',5), 0,    E, 100), (N('E',5), 0.5,  E, 100),
    (N('G',5), 1,    Q, 105), (N('C',6), 2,    E, 108),
    (N('B',5), 2.5,  E, 102), (N('A',5), 3,    E, 100),
    (N('G',5), 3.5,  E, 98),
    # Bar 2 (Am): Graceful descent
    (N('A',5), 4,    Q, 100), (N('G',5), 5,    E, 95),
    (N('F',5), 5.5,  E, 92),  (N('E',5), 6,    H, 95),
    # Bar 3 (F): Lyrical second phrase
    (N('F',5), 8,    E, 95),  (N('G',5), 8.5,  E, 98),
    (N('A',5), 9,    Q, 102), (N('G',5), 10,   E, 95),
    (N('F',5), 10.5, E, 92),  (N('E',5), 11,   Q, 90),
    # Bar 4 (G): Half cadence
    (N('D',5), 12,   Q, 92),  (N('E',5), 13,   E, 88),
    (N('D',5), 13.5, E, 85),  (N('D',5), 14,   H, 82),
    # Bar 5 (C): Motif developed higher
    (N('C',5), 16,   E, 100), (N('E',5), 16.5, E, 102),
    (N('G',5), 17,   E, 105), (N('C',6), 17.5, E, 108),
    (N('D',6), 18,   Q, 110), (N('C',6), 19,   Q, 105),
    # Bar 6 (Em): Expressive leap
    (N('B',5), 20,   Q, 102), (N('G',5), 21,   E, 95),
    (N('A',5), 21.5, E, 98),  (N('B',5), 22,   E, 100),
    (N('A',5), 22.5, E, 95),  (N('G',5), 23,   Q, 92),
    # Bar 7 (F): Building intensity
    (N('A',5), 24,   E, 98),  (N('B',5), 24.5, E, 100),
    (N('C',6), 25,   Q, 105), (N('A',5), 26,   E, 95),
    (N('G',5), 26.5, E, 92),  (N('F',5), 27,   Q, 90),
    # Bar 8 (G): Section cadence
    (N('G',5), 28,   Q, 100), (N('F',5), 29,   E, 95),
    (N('E',5), 29.5, E, 92),  (N('D',5), 30,   Q, 88),
    (N('C',5), 31,   Q, 85),
]

# Flute counter-melody for A section
A_COUNTER = [
    (N('E',4), 0,  H, 58), (N('C',4), 2,  H, 55),
    (N('C',4), 4,  H, 55), (N('A',3), 6,  H, 52),
    (N('A',3), 8,  H, 55), (N('F',3), 10, H, 52),
    (N('B',3), 12, H, 55), (N('G',3), 14, H, 52),
    (N('E',4), 16, Q, 60), (N('G',4), 17, Q, 58), (N('C',5), 18, H, 60),
    (N('E',4), 20, H, 55), (N('B',3), 22, H, 52),
    (N('F',4), 24, Q, 58), (N('A',4), 25, Q, 55), (N('F',4), 26, H, 52),
    (N('B',3), 28, Q, 55), (N('D',4), 29, Q, 52), (N('G',3), 30, H, 50),
]

# B section melody (trumpet — contrasting, minor feel)
B_MELODY = [
    # Bar 1 (Am): Bold entry
    (N('A',4), 0,    E, 95),  (N('C',5), 0.5,  E, 98),
    (N('E',5), 1,    Q, 102), (N('E',5), 2,    E, 98),
    (N('D',5), 2.5,  E, 95),  (N('C',5), 3,    Q, 92),
    # Bar 2 (F):
    (N('F',5), 4,    Q, 98),  (N('E',5), 5,    E, 92),
    (N('D',5), 5.5,  E, 90),  (N('C',5), 6,    H, 88),
    # Bar 3 (Dm): Tension
    (N('D',5), 8,    E, 95),  (N('E',5), 8.5,  E, 98),
    (N('F',5), 9,    Q, 102), (N('G',5), 10,   E, 100),
    (N('A',5), 10.5, E, 105), (N('G',5), 11,   Q, 100),
    # Bar 4 (G): Release
    (N('F',5), 12,   Q, 98),  (N('E',5), 13,   Q, 92),
    (N('D',5), 14,   H, 88),
    # Bar 5 (Em): Deeper exploration
    (N('E',5), 16,   E, 92),  (N('G',5), 16.5, E, 95),
    (N('B',5), 17,   Q, 100), (N('A',5), 18,   E, 95),
    (N('G',5), 18.5, E, 92),  (N('E',5), 19,   Q, 88),
    # Bar 6 (Am): Response
    (N('A',5), 20,   Q, 98),  (N('G',5), 21,   E, 92),
    (N('F',5), 21.5, E, 90),  (N('E',5), 22,   Q, 88),
    (N('C',5), 23,   Q, 85),
    # Bar 7 (F): Building to climax
    (N('C',5), 24,   E, 90),  (N('D',5), 24.5, E, 92),
    (N('E',5), 25,   E, 95),  (N('F',5), 25.5, E, 98),
    (N('G',5), 26,   Q, 100), (N('A',5), 27,   Q, 102),
    # Bar 8 (G): Transition to C
    (N('B',5), 28,   Q, 105), (N('A',5), 29,   E, 100),
    (N('G',5), 29.5, E, 98),  (N('G',5), 30,   H, 95),
]

# Strings counter-melody for B section
B_COUNTER = [
    (N('E',4), 0,  DQ, 62), (N('A',4), 1.5, DQ, 58), (N('E',4), 3, Q, 55),
    (N('C',4), 4,  H,  58), (N('A',3), 6,   H,  55),
    (N('D',4), 8,  H,  62), (N('F',4), 10,  H,  58),
    (N('B',3), 12, H,  58), (N('G',3), 14,  H,  55),
    (N('E',4), 16, H,  60), (N('G',4), 18,  H,  58),
    (N('C',4), 20, H,  58), (N('A',3), 22,  H,  55),
    (N('F',3), 24, H,  58), (N('A',3), 26,  H,  55),
    (N('B',3), 28, H,  60), (N('D',4), 30,  H,  58),
]

# Clarinet fills for B section
B_EXTRA = [
    (N('A',4), 2,  H, 52), (N('F',4), 6,  H, 50),
    (N('A',4), 10, Q, 55), (N('D',5), 11, Q, 55),
    (N('G',4), 14, H, 50), (N('E',4), 18, H, 52),
    (N('C',4), 22, H, 50), (N('F',4), 26, H, 52),
    (N('G',4), 30, H, 55),
]

# Intro melody (vibes — sparse, atmospheric)
INTRO = [
    (N('E',5), 0,   Q, 72), (N('G',5), 2,   Q, 68),
    (N('E',5), 4,   Q, 70), (N('C',5), 5,   Q, 65), (N('A',4), 6, H, 62),
    (N('C',5), 8,   E, 68), (N('D',5), 8.5, E, 72),
    (N('E',5), 9,   E, 75), (N('F',5), 9.5, E, 78), (N('G',5), 10, H, 80),
    (N('F',5), 12,  Q, 75), (N('D',5), 13,  Q, 72), (N('D',5), 14, H, 68),
]

# Glockenspiel sparkles in intro
INTRO_SPARKLE = [
    (N('C',6), 1,  E, 48), (N('A',5), 6.5, E, 45),
    (N('F',6), 10, E, 50), (N('B',5), 14,  Q, 48), (N('D',6), 15, E, 45),
]

# Outro melody (piano — gentle restatement)
OUTRO = [
    (N('C',5), 0,   E, 78), (N('E',5), 0.5, E, 78),
    (N('G',5), 1,   Q, 82), (N('F',5), 2,   Q, 78), (N('E',5), 3,   Q, 75),
    (N('C',5), 4,   Q, 75), (N('A',4), 5,   Q, 72), (N('A',4), 6,   H, 68),
    (N('B',4), 8,   E, 72), (N('D',5), 8.5, E, 75),
    (N('G',5), 9,   Q, 78), (N('F',5), 10,  Q, 72), (N('D',5), 11,  Q, 68),
    (N('C',5), 12,  Q, 75), (N('E',5), 13,  E, 70),
    (N('D',5), 13.5,E, 68), (N('C',5), 14,  H, 65),
]

# ── Write all melody sections ──

# Intro
for p, b, d, v in INTRO:
    mel(p, b, d, v)
for p, b, d, v in INTRO_SPARKLE:
    ext(p, b, d, v)

# A section (bars 5-12)
for p, b, d, v in A_MELODY:
    mel(p, bar(5)+b, d, v)
for p, b, d, v in A_COUNTER:
    ctr(p, bar(5)+b, d, v)

# B section (bars 13-20)
for p, b, d, v in B_MELODY:
    mel(p, bar(13)+b, d, v)
for p, b, d, v in B_COUNTER:
    ctr(p, bar(13)+b, d, v)
for p, b, d, v in B_EXTRA:
    ext(p, bar(13)+b, d, v)

# C section (bars 21-28): A melody restated with trumpet harmony + power
def third_below(pitch):
    pc = pitch % 12
    offsets = {0:3, 2:3, 4:4, 5:3, 7:3, 9:4, 11:4}
    return pitch - offsets.get(pc, 3)

for p, b, d, v in A_MELODY:
    mel(p, bar(21)+b, d, min(v+5, 127))
    ctr(third_below(p), bar(21)+b, d, max(v-15, 40))

# Lead 1 shimmer accents in C section
for b_off in [0, 2, 8, 10, 16, 18, 24, 26]:
    ext(N('C',6) + (b_off % 7), bar(21)+b_off, E, 50)

# Outro (bars 29-32)
for p, b, d, v in OUTRO:
    mel(p, bar(29)+b, d, v)

# ── Accent hits ──

# Timpani (bars 1-20, program 47)
for b_num in [5, 8, 9, 12]:
    r, _, _ = CD[PROG[b_num-1]]
    acc(r, bar(b_num), Q, 62)
for b_num in [13, 15, 17, 20]:
    r, _, _ = CD[PROG[b_num-1]]
    acc(r, bar(b_num), Q, 68)

# Orch hits in C section (program switches to 55 at bar 21)
for b_num in [21, 23, 25, 27]:
    r, _, _ = CD[PROG[b_num-1]]
    acc(r+12, bar(b_num), E, 85)

# Timpani in outro (program switches back to 47 at bar 29)
acc(N('C',3), bar(32), H, 60)

# ══════════════════════════════════════════════════════════════
# WRITE MIDI
# ══════════════════════════════════════════════════════════════

for trk, ch, pitch, beat, dur, vel in notes:
    midi.addNote(trk, ch, pitch, beat, dur, vel)

out = os.path.join(os.path.dirname(__file__), "overworld.mid")
with open(out, "wb") as f:
    midi.writeFile(f)
print(f"Created {out}")
