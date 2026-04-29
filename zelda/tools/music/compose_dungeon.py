"""Dungeon 1 theme — dark, echoing, minor-key exploration loop."""
from midiutil import MIDIFile
import os

TEMPO = 110
midi = MIDIFile(8)
for t in range(8):
    midi.addTempo(t, 0, TEMPO)

MELODY, COUNTER, ARP, BASS_T, PAD, ACCENT, EXTRA, DRUM_T = range(8)
CH = [0, 1, 2, 3, 4, 5, 6, 9]

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

for t, v in [(MELODY,90),(COUNTER,65),(ARP,50),(BASS_T,85),
             (PAD,55),(ACCENT,70),(EXTRA,55),(DRUM_T,72)]:
    vc(t, 0, v)

# 24 bars, 3 sections of 8 bars each
# Key: A minor / D minor — dark, dungeon feel
# Instruments: organ, strings, low flute, timpani

# Section A (bars 1-8): creeping exploration
pc(MELODY, 0, 16)       # organ
pc(COUNTER, 0, 71)      # clarinet
pc(ARP, 0, 10)          # glockenspiel (dripping water)
pc(BASS_T, 0, 33)       # fingered bass
pc(PAD, 0, 48)          # strings
pc(ACCENT, 0, 47)       # timpani
pc(EXTRA, 0, 80)        # square lead

# Section B (bars 9-16): tension builds
pc(MELODY, bar(9), 71)    # clarinet takes lead
pc(COUNTER, bar(9), 16)   # organ counter
pc(ARP, bar(9), 80)       # square arp
pc(PAD, bar(9), 89)       # warm pad
pc(ACCENT, bar(9), 47)    # timpani
vc(PAD, bar(9), 60)

# Section C (bars 17-24): climax, darker
pc(MELODY, bar(17), 80)   # square lead
pc(COUNTER, bar(17), 56)  # trumpet counter
pc(ARP, bar(17), 10)      # glockenspiel
pc(PAD, bar(17), 48)      # strings
pc(ACCENT, bar(17), 47)   # timpani
pc(EXTRA, bar(17), 16)    # organ accent
vc(MELODY, bar(17), 100)
vc(PAD, bar(17), 62)

# Chord data — Am, Dm, Em, F, G, Bdim
CD = {
    'Am': (N('A',2), N('E',3), (N('A',3), N('C',4), N('E',4))),
    'Dm': (N('D',3), N('A',3), (N('D',4), N('F',4), N('A',4))),
    'Em': (N('E',3), N('B',3), (N('E',4), N('G',4), N('B',4))),
    'F':  (N('F',2), N('C',3), (N('F',3), N('A',3), N('C',4))),
    'G':  (N('G',2), N('D',3), (N('G',3), N('B',3), N('D',4))),
    'E':  (N('E',2), N('B',2), (N('E',3), N('G',3), N('B',3))),
}

PROG = [
    'Am','Dm','Am','Em', 'F','Dm','Em','Am',      # A
    'Dm','F','Am','G',   'Dm','Em','F','E',        # B
    'Am','Dm','F','Em',  'Am','Dm','E','Am',       # C
]

# Drums
K, SN, HC, HO, CR, TL, TH = 36, 38, 42, 46, 49, 41, 45

def d_sparse(b):
    add(DRUM_T, HC, b+1, E, 35)
    add(DRUM_T, HC, b+3, E, 35)

def d_creep(b):
    add(DRUM_T, K, b, E, 70)
    add(DRUM_T, HC, b+1, E, 40)
    add(DRUM_T, HC, b+2.5, E, 35)
    add(DRUM_T, HC, b+3.5, E, 35)

def d_tension(b):
    add(DRUM_T, K, b, E, 78)
    add(DRUM_T, HC, b+0.5, E, 38)
    add(DRUM_T, HC, b+1, E, 42)
    add(DRUM_T, SN, b+2, E, 65)
    add(DRUM_T, HC, b+2.5, E, 38)
    add(DRUM_T, K, b+3, E, 68)
    add(DRUM_T, HC, b+3.5, E, 38)

def d_fill(b):
    add(DRUM_T, TL, b, E, 60)
    add(DRUM_T, TL, b+0.5, E, 65)
    add(DRUM_T, TH, b+1, E, 70)
    add(DRUM_T, TH, b+1.5, E, 75)
    add(DRUM_T, CR, b+2, Q, 80)
    add(DRUM_T, K, b+2, E, 85)
    add(DRUM_T, K, b+3, E, 72)

d_seq = ([d_sparse]*3 + [d_creep] + [d_creep]*3 + [d_fill] +
         [d_tension]*3 + [d_fill] + [d_tension]*3 + [d_fill] +
         [d_tension]*3 + [d_fill] + [d_tension]*3 + [d_fill])

# Bass
def b_drone(r,f,b): add(BASS_T, r, b, W, 75)
def b_pulse(r,f,b):
    add(BASS_T, r, b, Q, 82)
    add(BASS_T, r, b+2, Q, 78)
def b_walk(r,f,b):
    add(BASS_T, r, b, Q, 82)
    add(BASS_T, f, b+1, Q, 78)
    add(BASS_T, r+12, b+2, Q, 75)
    add(BASS_T, f, b+3, Q, 78)
def b_eighth(r,f,b):
    for i in range(8):
        p = r if i % 2 == 0 else f
        add(BASS_T, p, b + i*0.5, E, 78 + (i%3)*4)

b_seq = ([b_drone]*4 + [b_pulse]*4 +
         [b_walk]*8 +
         [b_eighth]*4 + [b_walk]*3 + [b_drone])

# Pads
def p_swell(tri,b):
    for p in tri: add(PAD, p, b, H, 38)
    for p in tri: add(PAD, p, b+2, H, 50)
def p_hold(tri,b):
    for p in tri: add(PAD, p, b, W, 48)

p_seq = ([p_swell]*8 + [p_hold]*8 + [p_swell]*4 + [p_hold]*4)

# Arps — sparse, eerie dripping
def a_drip(tri,b):
    r,t,f = tri
    add(ARP, f, b+0.5, E, 42)
    add(ARP, r, b+2.5, E, 38)

def a_echo(tri,b):
    r,t,f = tri
    for i,p in enumerate([r,f,t,r+12]): add(ARP, p, b+i, Q, 40)

def a_trill(tri,b):
    r,t,f = tri
    for i,p in enumerate([r,t,f,t,r,t,f,t]): add(ARP, p, b+i*0.5, E, 45)

a_seq = ([a_drip]*8 + [a_echo]*8 + [a_trill]*8)

for i, ch in enumerate(PROG):
    b = bar(i+1)
    r, f, tri = CD[ch]
    d_seq[i](b); b_seq[i](r,f,b); p_seq[i](tri,b); a_seq[i](tri,b)

# Hand-written melodies

# A section melody — haunting organ, minor intervals
A_MEL = [
    (N('A',4), 0,    Q, 85),  (N('C',5), 1,    Q, 90),
    (N('B',4), 2,    E, 82),  (N('A',4), 2.5,  E, 80),
    (N('G',4), 3,    Q, 75),
    (N('F',4), 4,    H, 82),  (N('E',4), 6,    H, 78),
    (N('A',4), 8,    E, 85),  (N('B',4), 8.5,  E, 88),
    (N('C',5), 9,    Q, 92),  (N('D',5), 10,   E, 88),
    (N('C',5), 10.5, E, 85),  (N('B',4), 11,   Q, 80),
    (N('A',4), 12,   Q, 78),  (N('G',4), 13,   Q, 75),
    (N('A',4), 14,   H, 72),
    (N('E',5), 16,   Q, 92),  (N('D',5), 17,   E, 88),
    (N('C',5), 17.5, E, 85),  (N('B',4), 18,   Q, 82),
    (N('A',4), 19,   Q, 78),
    (N('F',4), 20,   Q, 82),  (N('G',4), 21,   Q, 80),
    (N('A',4), 22,   H, 75),
    (N('C',5), 24,   E, 88),  (N('B',4), 24.5, E, 85),
    (N('A',4), 25,   Q, 82),  (N('G',4), 26,   Q, 78),
    (N('E',4), 27,   Q, 75),
    (N('A',4), 28,   Q, 82),  (N('G',4), 29,   E, 78),
    (N('F',4), 29.5, E, 75),  (N('E',4), 30,   H, 70),
]

A_CTR = [
    (N('E',4), 0,  H, 55),  (N('A',3), 2,  H, 52),
    (N('D',4), 4,  H, 55),  (N('C',4), 6,  H, 50),
    (N('E',4), 8,  H, 55),  (N('A',3), 10, H, 52),
    (N('E',4), 12, Q, 55),  (N('D',4), 13, Q, 52),  (N('C',4), 14, H, 50),
    (N('A',3), 16, H, 52),  (N('E',4), 18, H, 55),
    (N('D',4), 20, H, 52),  (N('C',4), 22, H, 50),
    (N('A',3), 24, H, 55),  (N('E',4), 26, H, 52),
    (N('C',4), 28, H, 50),  (N('A',3), 30, H, 48),
]

# B section melody — clarinet, more urgent
B_MEL = [
    (N('D',5), 0,    E, 88),  (N('F',5), 0.5,  E, 92),
    (N('A',5), 1,    Q, 95),  (N('G',5), 2,    E, 90),
    (N('F',5), 2.5,  E, 88),  (N('E',5), 3,    Q, 85),
    (N('F',5), 4,    Q, 90),  (N('E',5), 5,    E, 85),
    (N('D',5), 5.5,  E, 82),  (N('C',5), 6,    H, 80),
    (N('A',4), 8,    E, 85),  (N('C',5), 8.5,  E, 88),
    (N('E',5), 9,    Q, 92),  (N('D',5), 10,   Q, 88),
    (N('B',4), 11,   Q, 82),
    (N('C',5), 12,   Q, 88),  (N('D',5), 13,   E, 85),
    (N('E',5), 13.5, E, 88),  (N('D',5), 14,   H, 82),
    (N('D',5), 16,   E, 88),  (N('E',5), 16.5, E, 90),
    (N('F',5), 17,   Q, 95),  (N('E',5), 18,   E, 90),
    (N('D',5), 18.5, E, 85),  (N('C',5), 19,   Q, 82),
    (N('A',4), 20,   Q, 85),  (N('B',4), 21,   E, 80),
    (N('C',5), 21.5, E, 82),  (N('D',5), 22,   H, 78),
    (N('F',5), 24,   Q, 92),  (N('E',5), 25,   E, 88),
    (N('D',5), 25.5, E, 85),  (N('C',5), 26,   Q, 82),
    (N('A',4), 27,   Q, 78),
    (N('B',4), 28,   Q, 85),  (N('A',4), 29,   E, 80),
    (N('G',4), 29.5, E, 75),  (N('A',4), 30,   H, 72),
]

# C section melody — square lead, darker climax
C_MEL = [
    (N('A',5), 0,    E, 100), (N('G',5), 0.5,  E, 98),
    (N('E',5), 1,    Q, 102), (N('C',5), 2,    E, 95),
    (N('D',5), 2.5,  E, 98),  (N('E',5), 3,    Q, 100),
    (N('D',5), 4,    Q, 95),  (N('F',5), 5,    E, 98),
    (N('E',5), 5.5,  E, 95),  (N('D',5), 6,    H, 90),
    (N('F',5), 8,    E, 98),  (N('A',5), 8.5,  E, 102),
    (N('G',5), 9,    Q, 100), (N('F',5), 10,   E, 95),
    (N('E',5), 10.5, E, 92),  (N('D',5), 11,   Q, 88),
    (N('A',5), 12,   Q, 102), (N('G',5), 13,   E, 98),
    (N('F',5), 13.5, E, 95),  (N('E',5), 14,   H, 92),
    (N('C',5), 16,   E, 95),  (N('E',5), 16.5, E, 98),
    (N('A',5), 17,   Q, 105), (N('G',5), 18,   E, 100),
    (N('F',5), 18.5, E, 95),  (N('E',5), 19,   Q, 92),
    (N('D',5), 20,   Q, 90),  (N('C',5), 21,   E, 85),
    (N('D',5), 21.5, E, 88),  (N('E',5), 22,   H, 85),
    (N('A',4), 24,   E, 88),  (N('C',5), 24.5, E, 92),
    (N('E',5), 25,   Q, 95),  (N('D',5), 26,   Q, 90),
    (N('C',5), 27,   Q, 85),
    (N('A',4), 28,   Q, 88),  (N('E',4), 29,   Q, 82),
    (N('A',4), 30,   H, 78),
]

# Write melodies
for p, b, d, v in A_MEL:  mel(p, bar(1)+b, d, v)
for p, b, d, v in A_CTR:  ctr(p, bar(1)+b, d, v)
for p, b, d, v in B_MEL:  mel(p, bar(9)+b, d, v)
for p, b, d, v in C_MEL:  mel(p, bar(17)+b, d, v)

# Trumpet counter in C section — harmony a third below
def third_below(pitch):
    pc_val = pitch % 12
    offsets = {0:3, 2:3, 4:4, 5:3, 7:3, 9:4, 11:4}
    return pitch - offsets.get(pc_val, 3)

for p, b, d, v in C_MEL:
    ctr(third_below(p), bar(17)+b, d, max(v-20, 40))

# Timpani accents
for b_num in [1, 5, 9, 13, 17, 21]:
    r, _, _ = CD[PROG[b_num-1]]
    acc(r, bar(b_num), Q, 65)

acc(N('A',2), bar(24), H, 55)

# Write MIDI
for trk, ch, pitch, beat, dur, vel in notes:
    midi.addNote(trk, ch, pitch, beat, dur, vel)

out = os.path.join(os.path.dirname(__file__), "dungeon1.mid")
with open(out, "wb") as f:
    midi.writeFile(f)
print(f"Created {out}")
