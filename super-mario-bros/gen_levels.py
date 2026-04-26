#!/usr/bin/env python3
"""Generate World 7 level files with guaranteed equal-width rows."""

import os

BASE = "C:/dev/games-c/super-mario-bros/src/resources/levels"


def pad_rows(rows, width=None):
    """Pad all rows to the same width with dots."""
    if width is None:
        width = max(len(r) for r in rows)
    return [r.ljust(width, '.') for r in rows]


def write_level(filename, header, tile_rows, blocks="", spawns="", warps=""):
    """Write a level file with properly padded tile rows."""
    padded = pad_rows(tile_rows)
    content = header + "\n\ntiles\n"
    content += "\n".join(padded)
    content += "\n"
    if blocks:
        content += "\nblocks\n" + blocks + "\n"
    if spawns:
        content += "\nspawns\n" + spawns + "\n"
    if warps:
        content += "\nwarps\n" + warps + "\n"
    path = os.path.join(BASE, filename)
    with open(path, 'w', newline='\n') as f:
        f.write(content)
    # Verify
    widths = [len(r) for r in padded]
    assert len(set(widths)) == 1, f"{filename}: rows have different widths: {widths}"
    assert len(padded) == 15, f"{filename}: expected 15 rows, got {len(padded)}"
    print(f"{filename}: {len(padded)} rows, width={widths[0]}")


# ============================================================
# 7-1: Overworld with Hammer Bros and Bill Blasters (~180 wide)
# ============================================================
def gen_7_1():
    W = 180
    fp = 168  # flagpole column

    r = [list("." * W) for _ in range(15)]

    # Row 5: blocks
    for i, c in enumerate("BQBQB"):
        r[5][20+i] = c
    r[5][35] = 'Q'
    for i, c in enumerate("BBQBB"):
        r[5][48+i] = c
    r[5][72] = 'Q'
    r[5][80] = 'Q'
    for i, c in enumerate("BQBQB"):
        r[5][96+i] = c
    for i, c in enumerate("BBQBB"):
        r[5][120+i] = c
    r[5][142] = 'Q'
    for i, c in enumerate("BQQB"):
        r[5][152+i] = c

    # Row 7: HHHHHH platforms
    for start in [50, 88, 130]:
        for i in range(6):
            r[7][start+i] = 'H'

    # Pipes at tx=28, 56, 82, 112, 140
    pipes = [28, 56, 82, 112, 140]
    for p in pipes:
        r[8][p] = '['
        r[8][p+1] = ']'
        for row_i in [9, 10, 11]:
            r[row_i][p] = '{'
            r[row_i][p+1] = '}'

    # Bill blasters at tx=38, 64, 92, 120, 150
    blasters = [38, 64, 92, 120, 150]
    for b in blasters:
        for row_i in [8, 9, 10, 11]:
            r[row_i][b] = 'T'

    # Flagpole column (F on rows 3-12, f on row 12 is wrong, f is the base)
    for row_i in range(3, 12):
        r[row_i][fp] = 'F'
    r[12] = list("." * W)  # ensure clean
    # Staircase: rows 4-11, growing left
    for step in range(9):
        row_i = 11 - step  # from row 11 (bottom of staircase) up to row 3
        for col in range(fp - 1 - step, fp):
            if col >= 0:
                r[row_i][col] = 'G'
    # Actually, let me use the pattern from existing levels:
    # Row 3: just F
    # Row 4: GF
    # Row 5: GGF (but row 5 already has blocks, F should be separate)
    # Let me clear and redo flagpole properly
    for row_i in range(15):
        for col in range(fp-9, fp+1):
            if col >= 0 and col < W:
                if r[row_i][col] in ('G', 'F', 'f'):
                    r[row_i][col] = '.'

    # Flagpole F column
    r[3][fp] = 'F'
    r[4][fp] = 'F'
    r[5][fp] = 'F'
    r[6][fp] = 'F'
    r[7][fp] = 'F'
    r[8][fp] = 'F'
    r[9][fp] = 'F'
    r[10][fp] = 'F'
    r[11][fp] = 'f'  # base

    # Staircase of G tiles to the LEFT of the flagpole
    # Row 4: G at fp-1
    r[4][fp-1] = 'G'
    # Row 5: G at fp-1, fp-2
    r[5][fp-1] = 'G'
    r[5][fp-2] = 'G'
    # Row 6: fp-1 to fp-3
    for i in range(1, 4):
        r[6][fp-i] = 'G'
    # Row 7: fp-1 to fp-4
    for i in range(1, 5):
        r[7][fp-i] = 'G'
    # Row 8: fp-1 to fp-5
    for i in range(1, 6):
        r[8][fp-i] = 'G'
    # Row 9: fp-1 to fp-6
    for i in range(1, 7):
        r[9][fp-i] = 'G'
    # Row 10: fp-1 to fp-7
    for i in range(1, 8):
        r[10][fp-i] = 'G'
    # Row 11: fp-1 to fp-8
    for i in range(1, 9):
        r[11][fp-i] = 'G'

    # Ground rows 12-13 with gaps, row 14 solid
    g_with_gaps = list("G" * W)
    gaps = [(38, 40), (72, 74), (110, 112), (148, 150)]
    for s, e in gaps:
        for i in range(s, e):
            g_with_gaps[i] = '.'
    r[12] = list("".join(g_with_gaps))
    r[13] = list("".join(g_with_gaps))
    r[14] = list("G" * W)

    rows = pad_rows(["".join(row) for row in r], W)

    write_level("7-1.txt",
        "type overworld\nbg 92 148 252",
        rows,
        blocks="\n".join([
            "20 5 mushroom",
            "22 5 coin",
            "24 5 coin",
            "35 5 fire_flower",
            "48 5 coin",
            "72 5 coin",
            "80 5 coin",
            "96 5 starman",
            "98 5 coin",
            "100 5 coin",
            "120 5 coin",
            "142 5 fire_flower",
            "152 5 coin",
            "153 5 coin",
        ]),
        spawns="\n".join([
            "15 12 goomba",
            "16 12 goomba",
            "25 12 goomba",
            "33 12 hammer_bro",
            "44 12 goomba",
            "45 12 goomba",
            "52 12 hammer_bro",
            "60 12 goomba",
            "61 12 goomba",
            "70 12 koopa",
            "75 12 hammer_bro",
            "85 12 goomba",
            "86 12 goomba",
            "92 12 koopa red",
            "102 12 goomba",
            "103 12 goomba",
            "108 12 hammer_bro",
            "115 12 goomba",
            "116 12 goomba",
            "125 12 koopa",
            "132 12 goomba",
            "137 12 hammer_bro",
            "144 12 goomba",
            "145 12 goomba",
            "155 12 koopa",
            "160 12 goomba",
            "161 12 goomba",
        ]),
    )


# ============================================================
# 7-2: Underwater (~150 wide)
# ============================================================
def gen_7_2():
    W = 150

    r = [list("." * W) for _ in range(15)]

    # Row 0: solid H ceiling
    r[0] = list("H" * W)

    # Row 1: ceiling with gap
    r[1] = list("H" * 4 + "." * (W - 4))
    for i in range(72, 76):
        r[1][i] = 'H'

    # Row 2: ceiling blocks
    for i in range(52, 56):
        r[2][i] = 'H'

    # Row 3: ceiling blocks
    for i in range(68, 72):
        r[3][i] = 'H'

    # Row 4: HHH platforms
    for s, e in [(30, 33), (38, 42), (70, 74), (85, 88), (89, 91)]:
        for i in range(s, e):
            r[4][i] = 'H'

    # Row 5: HHH
    for i in range(62, 65):
        r[5][i] = 'H'

    # Row 6: HHH platforms
    for s, e in [(52, 55), (59, 62), (78, 81)]:
        for i in range(s, e):
            r[6][i] = 'H'

    # Row 7: coral (CCCC patterns)
    coral7 = [(14, 18), (32, 36), (68, 72), (80, 84), (92, 96), (104, 108), (116, 120)]
    for s, e in coral7:
        for i in range(s, e):
            r[7][i] = 'C'

    # Row 8: coral
    coral8 = [(7, 11), (20, 24), (28, 32), (40, 44), (52, 56), (64, 68),
              (76, 80), (88, 92), (100, 104), (112, 116), (124, 128)]
    for s, e in coral8:
        for i in range(s, e):
            r[8][i] = 'C'

    # Row 9: coral
    coral9 = [(0, 4), (14, 18), (24, 28), (36, 40), (48, 52), (60, 64),
              (72, 76), (84, 88), (96, 100), (108, 112), (120, 124)]
    for s, e in coral9:
        for i in range(s, e):
            r[9][i] = 'C'

    # Rows 10-12: pipes and flagpole
    pipe_positions = [74, 106, 120]
    for p in pipe_positions:
        r[10][p] = '['
        r[10][p+1] = ']'
        r[11][p] = '{'
        r[11][p+1] = '}'
        r[12][p] = '{'
        r[12][p+1] = '}'

    # Flagpole at tx=140
    for row_i in range(10, 13):
        r[row_i][140] = 'F'

    # Row 13: ground with flagpole base
    r[13] = list("G" * W)
    r[13][140] = 'f'

    # Row 14: solid ground
    r[14] = list("G" * W)

    rows = pad_rows(["".join(row) for row in r], W)

    write_level("7-2.txt",
        "type underwater\nbg 0 0 80",
        rows,
        spawns="\n".join([
            "18 6 blooper",
            "30 4 cheep 0",
            "42 5 cheep 1",
            "55 3 blooper",
            "65 6 cheep 0",
            "75 4 cheep 1",
            "82 7 blooper",
            "90 5 cheep 0",
            "98 3 blooper",
            "105 6 cheep 1",
            "112 4 cheep 0",
            "120 7 blooper",
            "130 5 cheep 1",
        ]),
    )


# ============================================================
# 7-3: Athletic/bridge (~155 wide)
# ============================================================
def gen_7_3():
    W = 155
    fp = 143  # flagpole column

    r = [list("." * W) for _ in range(15)]

    # Flagpole
    for row_i in range(3, 12):
        r[row_i][fp] = 'F'
    r[12][fp] = 'f'

    # Staircase
    for step in range(9):
        row_i = 12 - step  # row 12 down to row 4
        for col in range(fp - 1 - step, fp):
            if col >= 0:
                r[row_i][col] = 'G'

    # Row 6: Q blocks and HHHH platforms
    r[6][14] = 'Q'
    r[6][28] = 'Q'
    for i in range(34, 38):
        r[6][i] = 'H'
    r[6][44] = 'Q'
    r[6][45] = 'Q'
    for i in range(60, 64):
        r[6][i] = 'H'
    r[6][75] = 'Q'
    r[6][98] = 'Q'
    r[6][118] = 'Q'

    # Row 8: HHHH platforms
    for s, e in [(18, 22), (38, 44), (56, 62), (74, 79)]:
        for i in range(s, e):
            r[8][i] = 'H'

    # Row 10: bridge/platform segments (main jumping platforms)
    platforms = [(10, 17), (22, 29), (34, 41), (48, 55), (62, 68),
                 (74, 81), (88, 96), (102, 110), (116, 123)]
    for s, e in platforms:
        for i in range(s, e):
            r[10][i] = 'H'

    # Ground rows 13-14: ground at start, end, with big gaps
    g = list("." * W)
    for i in range(0, 9):
        g[i] = 'G'
    for s, e in [(35, 43), (57, 65), (79, 87)]:
        for i in range(s, e):
            g[i] = 'G'
    for i in range(100, W):
        g[i] = 'G'
    r[13] = list("".join(g))
    r[14] = list("".join(g))

    rows = pad_rows(["".join(row) for row in r], W)

    write_level("7-3.txt",
        "type athletic\nbg 92 148 252",
        rows,
        blocks="\n".join([
            "14 6 coin",
            "28 6 mushroom",
            "44 6 coin",
            "45 6 starman",
            "75 6 fire_flower",
            "98 6 coin",
            "118 6 coin",
        ]),
        spawns="\n".join([
            "14 9 koopa red",
            "28 9 koopa red",
            "40 9 koopa red",
            "52 7 hammer_bro",
            "70 9 koopa red",
            "85 8 lift 1",
            "92 10 lift 1",
            "105 9 koopa red",
            "120 9 koopa red",
            "15 14 cheep 2",
            "28 14 cheep 2",
            "40 14 cheep 2",
            "52 14 cheep 2",
            "62 14 cheep 2",
            "75 14 cheep 2",
            "88 14 cheep 2",
            "100 14 cheep 2",
            "110 14 cheep 2",
            "125 14 cheep 2",
            "20 5 paratroopa",
            "45 5 paratroopa",
            "65 5 paratroopa",
            "95 5 paratroopa",
            "115 5 paratroopa",
        ]),
    )


# ============================================================
# 7-4: Castle MAZE with looping pipes (~200 wide)
# ============================================================
def gen_7_4():
    W = 200

    r = [list("." * W) for _ in range(15)]

    # Ceiling rows 0-1: H walls with gaps (creating rooms)
    for i in range(0, 15):
        r[0][i] = 'H'
        r[1][i] = 'H'
    for i in range(28, 55):
        r[0][i] = 'H'
        r[1][i] = 'H'
    for i in range(68, 95):
        r[0][i] = 'H'
        r[1][i] = 'H'
    for i in range(108, 131):
        r[0][i] = 'H'
        r[1][i] = 'H'
    for i in range(145, 161):
        r[0][i] = 'H'
        r[1][i] = 'H'

    # Rows 2-4: vertical H pillars creating maze-like feel
    for row_i in [2, 3, 4]:
        r[row_i][42] = 'H'
        r[row_i][82] = 'H'
        r[row_i][122] = 'H'

    # Row 6: HHH platforms
    for s in [18, 52, 88, 128]:
        for i in range(s, s+3):
            r[6][i] = 'H'

    # Row 7: HHH platforms (offset)
    for s in [10, 36, 72, 108]:
        for i in range(s, s+3):
            r[7][i] = 'H'

    # Pipes for the maze (rows 8-11)
    # 6 pipes: player must choose the right ones
    pipe_positions = [8, 30, 56, 78, 100, 132]
    for p in pipe_positions:
        r[8][p] = '['
        r[8][p+1] = ']'
        for row_i in [9, 10, 11]:
            r[row_i][p] = '{'
            r[row_i][p+1] = '}'

    # Bridge + Axe + Castle door on row 10
    bridge_start = 155
    bridge_end = 175
    for i in range(bridge_start, bridge_end + 1):
        r[10][i] = '='
    r[10][bridge_end + 1] = 'X'  # Axe at tx=176
    # Castle wall and door
    for i in range(177, 181):
        r[10][i] = 'H'
    r[10][181] = 'D'
    for i in range(182, 185):
        r[10][i] = 'H'
    # Walls below bridge area
    for row_i in [11, 12]:
        for i in range(177, 185):
            r[row_i][i] = 'H'

    # Floor rows 13-14: H with lava pits
    for row_i in [13, 14]:
        r[row_i] = list("H" * W)
        # Lava pits
        lava_pits = [(15, 20), (39, 44), (57, 62), (79, 84), (101, 106), (125, 130)]
        for s, e in lava_pits:
            for i in range(s, e):
                r[row_i][i] = 'L'
        # Long lava under bridge
        for i in range(145, 177):
            r[row_i][i] = 'L'

    rows = pad_rows(["".join(row) for row in r], W)

    write_level("7-4.txt",
        "type castle\nbg 0 0 0\nbridge 155 175 10",
        rows,
        spawns="\n".join([
            "12 12 firebar",
            "22 6 firebar cw",
            "36 12 firebar",
            "48 6 firebar cw",
            "64 12 firebar",
            "76 6 firebar cw",
            "90 12 firebar",
            "104 6 firebar",
            "115 12 firebar cw",
            "18 13 podoboo",
            "42 13 podoboo",
            "60 13 podoboo",
            "82 13 podoboo",
            "104 13 podoboo",
            "128 13 podoboo",
            "165 9 bowser",
        ]),
        warps="\n".join([
            "8 8 7 4 5 5",
            "30 8 7 4 5 5",
            "56 8 7 4 60 5",
            "78 8 7 4 5 5",
            "100 8 7 4 5 5",
            "132 8 7 4 140 5",
        ]),
    )


gen_7_1()
gen_7_2()
gen_7_3()
gen_7_4()
print("\nAll World 7 levels generated successfully!")
