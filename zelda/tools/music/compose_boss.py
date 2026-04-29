"""Boss 1 theme — intense, urgent, driving battle loop."""
from midiutil import MIDIFile
import os

TEMPO = 165
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

for t, v in [(MELODY,105),(COUNTER,72),(ARP,55),(BASS_T,90),
             (PAD,58),(ACCENT,80),(EXTRA,62),(DRUM_T,85)]:
    vc(t, 0, v)

# 16 bars, 2 sections of 8 bars — tight aggressive loop
# Key: E minor / B minor — menacing

# Section A (bars 1-8): main attack theme
pc(MELODY, 0, 80)       # square lead
pc(COUNTER, 0, 56)      # trumpet
pc(ARP, 0, 81)          # saw wave
pc(BASS_T, 0, 33)       # fingered bass
pc(PAD, 0, 48)          # strings
pc(ACCENT, 0, 47)       # timpani
pc(EXTRA, 0, 55)        # orch hit

# Section B (bars 9-16): intensification
pc(MELODY, bar(9), 56)    # trumpet takes lead
pc(COUNTER, bar(9), 80)   # square counter
pc(ARP, bar(9), 80)       # square arp
pc(PAD, bar(9), 89)       # warm pad
vc(MELODY, bar(9), 110)
vc(PAD, bar(9), 62)

CD = {
    'Em': (N('E',2), N('B',2), (N('E',3), N('G',3), N('B',3))),
    'Am': (N('A',2), N('E',3), (N('A',3), N('C',4), N('E',4))),
    'Bm': (N('B',2), N('F',3), (N('B',3), N('D',4), N('F',4))),
    'C':  (N('C',3), N('G',3), (N('C',4), N('E',4), N('G',4))),
    'D':  (N('D',3), N('A',3), (N('D',4), N('F',4), N('A',4))),
    'F':  (N('F',2), N('C',3), (N('F',3), N('A',3), N('C',4))),
    'G':  (N('G',2), N('D',3), (N('G',3), N('B',3), N('D',4))),
}

PROG = [
    'Em','C','Am','Bm', 'Em','D','C','Bm',     # A
    'Am','F','Em','G',  'Am','Bm','C','Em',     # B
]

# Drums — aggressive driving beat
K, SN, HC, HO, CR, TL, TH = 36, 38, 42, 46, 49, 41, 45

def d_drive(b):
    add(DRUM_T, K,  b,     E, 105)
    add(DRUM_T, HC, b+0.5, E, 55)
    add(DRUM_T, HC, b+1,   E, 60)
    add(DRUM_T, K,  b+1.5, E, 90)
    add(DRUM_T, SN, b+2,   E, 100)
    add(DRUM_T, HC, b+2.5, E, 55)
    add(DRUM_T, K,  b+3,   E, 92)
    add(DRUM_T, HC, b+3.5, E, 55)

def d_double(b):
    add(DRUM_T, K,  b,     E, 108)
    add(DRUM_T, K,  b+0.5, E, 95)
    add(DRUM_T, HC, b+1,   E, 62)
    add(DRUM_T, HC, b+1.5, E, 55)
    add(DRUM_T, SN, b+2,   E, 105)
    add(DRUM_T, HC, b+2.5, E, 58)
    add(DRUM_T, K,  b+3,   E, 95)
    add(DRUM_T, SN, b+3.5, E, 85)

def d_fill(b):
    add(DRUM_T, SN, b,     E, 85)
    add(DRUM_T, SN, b+0.5, E, 90)
    add(DRUM_T, TL, b+1,   E, 88)
    add(DRUM_T, TH, b+1.5, E, 92)
    add(DRUM_T, CR, b+2,   Q, 105)
    add(DRUM_T, K,  b+2,   E, 108)
    add(DRUM_T, K,  b+3,   E, 95)
    add(DRUM_T, SN, b+3.5, E, 88)

d_seq = ([d_drive]*3 + [d_fill] + [d_double]*3 + [d_fill] +
         [d_double]*3 + [d_fill] + [d_double]*3 + [d_fill])

add(DRUM_T, CR, bar(1), Q, 100)
add(DRUM_T, CR, bar(9), Q, 105)

# Bass — driving eighth notes
def b_eighth(r,f,b):
    for i in range(8):
        p = r if i % 2 == 0 else f
        add(BASS_T, p, b + i*0.5, E, 88 + (i%3)*5)

def b_power(r,f,b):
    add(BASS_T, r, b, E, 95)
    add(BASS_T, r, b+0.5, E, 85)
    add(BASS_T, r+12, b+1, E, 90)
    add(BASS_T, r, b+1.5, E, 85)
    add(BASS_T, f, b+2, E, 92)
    add(BASS_T, f, b+2.5, E, 82)
    add(BASS_T, r, b+3, E, 90)
    add(BASS_T, r+12, b+3.5, E, 88)

b_seq = ([b_eighth]*4 + [b_power]*4 +
         [b_power]*4 + [b_eighth]*4)

# Pads — power chords
def p_stab(tri,b):
    for p in tri: add(PAD, p, b, E, 62)
    for p in tri: add(PAD, p, b+2, E, 58)

def p_sustain(tri,b):
    for p in tri: add(PAD, p, b, W, 55)

p_seq = ([p_stab]*8 + [p_sustain]*4 + [p_stab]*4)

# Arps — aggressive saw patterns
def a_fast(tri,b):
    r,t,f = tri
    for i,p in enumerate([r,t,f,r+12,f,t,r+12,f]):
        add(ARP, p, b+i*0.5, E, 50)

def a_pulse(tri,b):
    r,t,f = tri
    add(ARP, r, b, E, 55)
    add(ARP, f, b+1, E, 50)
    add(ARP, r+12, b+2, E, 55)
    add(ARP, f, b+3, E, 50)

a_seq = ([a_fast]*8 + [a_pulse]*4 + [a_fast]*4)

for i, ch in enumerate(PROG):
    b = bar(i+1)
    r, f, tri = CD[ch]
    d_seq[i](b); b_seq[i](r,f,b); p_seq[i](tri,b); a_seq[i](tri,b)

# A section melody — aggressive, rhythmic
A_MEL = [
    (N('E',5), 0,    E, 108), (N('G',5), 0.5,  E, 105),
    (N('B',5), 1,    Q, 110), (N('A',5), 2,    E, 105),
    (N('G',5), 2.5,  E, 100), (N('E',5), 3,    E, 98),
    (N('F',5), 3.5,  E, 95),
    (N('E',5), 4,    Q, 100), (N('C',5), 5,    E, 95),
    (N('D',5), 5.5,  E, 98),  (N('E',5), 6,    H, 95),
    (N('A',5), 8,    E, 105), (N('G',5), 8.5,  E, 102),
    (N('F',5), 9,    Q, 100), (N('E',5), 10,   E, 98),
    (N('D',5), 10.5, E, 95),  (N('C',5), 11,   Q, 92),
    (N('B',4), 12,   E, 95),  (N('D',5), 12.5, E, 98),
    (N('E',5), 13,   Q, 102), (N('F',5), 14,   E, 98),
    (N('E',5), 14.5, E, 95),  (N('D',5), 15,   Q, 92),
    (N('E',5), 16,   E, 108), (N('G',5), 16.5, E, 110),
    (N('B',5), 17,   E, 112), (N('C',6), 17.5, E, 115),
    (N('B',5), 18,   Q, 110), (N('A',5), 19,   Q, 105),
    (N('G',5), 20,   E, 102), (N('A',5), 20.5, E, 105),
    (N('B',5), 21,   Q, 108), (N('G',5), 22,   E, 100),
    (N('E',5), 22.5, E, 98),  (N('D',5), 23,   Q, 95),
    (N('C',5), 24,   E, 98),  (N('E',5), 24.5, E, 100),
    (N('G',5), 25,   Q, 105), (N('F',5), 26,   E, 100),
    (N('E',5), 26.5, E, 98),  (N('D',5), 27,   Q, 95),
    (N('B',4), 28,   Q, 100), (N('D',5), 29,   E, 95),
    (N('E',5), 29.5, E, 98),  (N('B',4), 30,   H, 92),
]

# B section — trumpet, even more intense
B_MEL = [
    (N('A',5), 0,    E, 110), (N('C',6), 0.5,  E, 112),
    (N('E',5), 1,    Q, 108), (N('A',5), 2,    E, 105),
    (N('G',5), 2.5,  E, 102), (N('F',5), 3,    Q, 100),
    (N('F',5), 4,    E, 102), (N('A',5), 4.5,  E, 105),
    (N('C',5), 5,    Q, 100), (N('F',5), 6,    H, 98),
    (N('E',5), 8,    E, 105), (N('G',5), 8.5,  E, 108),
    (N('B',5), 9,    Q, 112), (N('A',5), 10,   E, 108),
    (N('G',5), 10.5, E, 105), (N('E',5), 11,   Q, 100),
    (N('D',5), 12,   Q, 105), (N('G',5), 13,   E, 102),
    (N('F',5), 13.5, E, 100), (N('D',5), 14,   H, 95),
    (N('A',5), 16,   E, 110), (N('G',5), 16.5, E, 108),
    (N('E',5), 17,   Q, 105), (N('C',5), 18,   E, 100),
    (N('D',5), 18.5, E, 102), (N('E',5), 19,   Q, 105),
    (N('B',4), 20,   Q, 100), (N('D',5), 21,   E, 98),
    (N('F',5), 21.5, E, 102), (N('E',5), 22,   H, 100),
    (N('C',5), 24,   E, 102), (N('E',5), 24.5, E, 105),
    (N('G',5), 25,   Q, 110), (N('A',5), 26,   E, 108),
    (N('G',5), 26.5, E, 105), (N('E',5), 27,   Q, 100),
    (N('E',5), 28,   Q, 108), (N('B',4), 29,   E, 100),
    (N('G',4), 29.5, E, 95),  (N('E',4), 30,   H, 90),
]

# Trumpet counter for A section
A_CTR = [
    (N('B',4), 0,  H, 65), (N('G',4), 2,  H, 62),
    (N('G',4), 4,  H, 62), (N('E',4), 6,  H, 58),
    (N('C',5), 8,  H, 65), (N('A',4), 10, H, 62),
    (N('G',4), 12, H, 62), (N('B',4), 14, H, 60),
    (N('B',4), 16, H, 65), (N('G',4), 18, H, 62),
    (N('D',5), 20, H, 65), (N('B',4), 22, H, 62),
    (N('E',4), 24, H, 62), (N('G',4), 26, H, 60),
    (N('G',4), 28, H, 62), (N('E',4), 30, H, 58),
]

for p, b, d, v in A_MEL:  mel(p, bar(1)+b, d, v)
for p, b, d, v in A_CTR:  ctr(p, bar(1)+b, d, v)
for p, b, d, v in B_MEL:  mel(p, bar(9)+b, d, v)

# Square counter harmony for B
for p, b, d, v in B_MEL:
    pc_val = p % 12
    offsets = {0:3, 2:3, 4:4, 5:3, 7:3, 9:4, 11:4}
    ctr(p - offsets.get(pc_val, 3), bar(9)+b, d, max(v-22, 40))

# Timpani on downbeats
for b_num in range(1, 17):
    r, _, _ = CD[PROG[b_num-1]]
    acc(r, bar(b_num), E, 75)

# Orch hit accents
for b_num in [1, 3, 5, 7, 9, 11, 13, 15]:
    r, _, _ = CD[PROG[b_num-1]]
    ext(r+24, bar(b_num), E, 72)

# Write MIDI
for trk, ch, pitch, beat, dur, vel in notes:
    midi.addNote(trk, ch, pitch, beat, dur, vel)

out = os.path.join(os.path.dirname(__file__), "boss1.mid")
with open(out, "wb") as f:
    midi.writeFile(f)
print(f"Created {out}")
