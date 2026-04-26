import os
base = "C:/dev/games-c/super-mario-bros/src/resources/levels"
for fname in sorted(os.listdir(base)):
    if not fname.endswith('.txt'): continue
    path = os.path.join(base, fname)
    with open(path) as f:
        lines = f.read().splitlines()
    in_tiles = False
    rows = []
    for line in lines:
        if line == 'tiles':
            in_tiles = True
            continue
        if in_tiles:
            if line == '' or line in ('blocks','spawns','warps'):
                break
            rows.append(line)
    widths = [len(r) for r in rows]
    same = "OK" if len(set(widths)) == 1 else "MISMATCH"
    print(f"{fname}: {len(rows)} rows, {same}, widths={widths}")
