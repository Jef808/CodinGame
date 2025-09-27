def parse_cell(cell: str) -> int:
    """Parse a cell from the input.

    Values are either '.' (no light), '1'-'9' (radius 1-9) or 'A'-'Z' (radius 10-35).
    """
    if cell == '.':
        return 0
    if '1' <= cell <= '9':
        return int(cell)
    return ord(cell) - ord('A') + 10


def fmt_cell(value: int) -> str:
    """Format a cell for output.

    Values are either . (no light), 1-9 (brightness 1-9) or 10-35 (brightness A-Z).
    If a value is > 35, it is capped at Z.
    """
    if 0 <= value <= 9:
        return str(value)
    if 10 <= value <= 35:
        return chr(ord('A') + value - 10)
    return 'Z'


def get_brightness(source: tuple[int, int, int], dest: tuple[int, int, int], radius: int) -> int:
    """Calculate the brightness of a light source at a given distance.

    The formula is brightness = radius - distance, rounded to the nearest integer."""
    a, b, c = source
    x, y, z = dest
    distance: float = ((a - x) ** 2 + (b - y) ** 2 + (c - z) ** 2) ** 0.5
    return max(0, round(radius - distance))


l = int(input())
w = int(input())
d = int(input())
n = int(input())

cells: list[list[list[int]]] = []

assert n == w * d + (d - 1), "Number of cells does not match dimensions"

for z_id, z in enumerate(range(d)):
    layer: list[list[int]] = []
    for y in range(w):
        row = list(map(parse_cell, input()))
        assert len(row) == l, "Row length does not match"
        layer.append(row)
    if z_id < d - 1:
        _ = input()  # Read the blank line
    cells.append(layer)



brightness: list[list[list[int]]] = [[[0 for _ in range(l)] for _ in range(w)] for _ in range(d)]

for z in range(d):
    for y in range(w):
        for x in range(l):
            radius = cells[z][y][x]
            if radius > 0:
                for dz in range(-radius, radius + 1):
                    for dy in range(-radius, radius + 1):
                        for dx in range(-radius, radius + 1):
                            nz, ny, nx = z + dz, y + dy, x + dx
                            if 0 <= nz < d and 0 <= ny < w and 0 <= nx < l:
                                b = get_brightness((x, y, z), (nx, ny, nz), radius)
                                brightness[nz][ny][nx] += b


for z in range(d):
    for y in range(w):
        print(''.join(map(fmt_cell, brightness[z][y])))
    if z < d - 1:
        print()
