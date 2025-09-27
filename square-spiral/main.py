# https://www.codingame.com/training/easy/square-spiral-for-alien-contact

def parse_instructions():
    instr = input().split(' ')

    size = int(instr[0])
    start = instr[1]  # topLeft, topRight, bottomRight or bottomLeft
    direction = instr[2]  # clockwise or counter-clockwise
    a = instr[3]
    b = instr[4]

    first_letter = a[0]
    first_number = int(a[1:])
    second_letter = b[0]
    second_number = int(b[1:])

    return size, start, direction, first_letter, first_number, second_letter, second_number


def generate_spiral_coordinates(size, start, direction):
    coordinates = []
    left, right = 0, size - 1
    top, bottom = 0, size - 1

    # Generate clockwise spiral coordinates starting from topLeft
    coordinates.extend([(i, 0) for i in range(right + 1)])  # Top row
    while True:
        # Traverse from down to bottom (if possible)
        if top + 2 > bottom:
            break
        coordinates.extend([(right, i) for i in range(top + 1, bottom + 1)])
        top += 2

        # Traverse from right to left (if possible)
        if right - 2 < left:
            break
        coordinates.extend([(i, bottom) for i in range(right - 1, left - 1, -1)])
        right -= 2

        # Traverse from bottom to top (if possible)
        if bottom - 2 < top:
            break
        coordinates.extend([(left, i) for i in range(bottom - 1, top - 1, -1)])
        bottom -= 2

        # Traverse from left to right (if possible)
        if left + 2 > right:
            break
        coordinates.extend([(i, top) for i in range(left + 1, right + 1)])
        left += 2

    # Transpose if direction is counter-clockwise
    if direction == 'counter-clockwise':
        coordinates = [(y, x) for x, y in coordinates]

    # Rotate coordinates based on starting corner
    def rotate(point, angle):
        x, y = point
        if angle == 90:
            return size - 1 - y, x
        elif angle == 180:
            return size - 1 - x, size - 1 - y
        elif angle == 270:
            return y, size - 1 - x
        return x, y

    rotation_angle = {
        'topLeft': 0,
        'topRight': 90,
        'bottomRight': 180,
        'bottomLeft': 270
    }[start]

    return [rotate(point, rotation_angle) for point in coordinates]


def generate_character_sequence(first_letter, first_number, second_letter, second_number):
    sequence = []

    min_letter = ord('A')
    max_letter = ord('Z')

    letter_diff = ord(second_letter) - ord(first_letter)
    number_diff = second_number - first_number

    letter = ord(first_letter)
    quantity = first_number

    while min_letter <= letter <= max_letter and quantity > 0:
        sequence.extend([chr(letter)] * quantity)
        letter += letter_diff
        quantity += number_diff

    return sequence


def main():
    size, start, direction, first_letter, first_number, second_letter, second_number = parse_instructions()

    spiral_coordinates = generate_spiral_coordinates(size, start, direction)
    character_sequence = generate_character_sequence(first_letter, first_number, second_letter, second_number)

    # Create an empty grid
    grid = [[' ' for _ in range(grid_size)] for _ in range(grid_size)]

    # Fill the grid with characters based on spiral coordinates,
    # stopping if we run out of characters
    for (x, y), char in zip(spiral_coordinates, character_sequence):
        grid[y][x] = char

    # Print the grid
    grid_size = min(size, 31)
    grid = [row[:grid_size] for row in grid[:grid_size]]
    for row in grid:
        print(''.join(row))


if __name__ == "__main__":
    main()
