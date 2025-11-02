#!/usr/bin/env python3

import sys

class Room:
    _paths_table = [
        # room_type 0
        {
            "LEFT": None,
            "TOP": None,
            "RIGHT": None,
            "BOTTOM": None,
        },
        # room_type 1
        {
            "LEFT": (0, 1),
            "TOP": (0, 1),
            "RIGHT": (0, 1),
            "BOTTOM": None,
        },
        # room_type 2
        {
            "LEFT": (1, 0),
            "TOP": None,
            "RIGHT": (-1, 0),
            "BOTTOM": None,
        },
        # room_type 3
        {
            "LEFT": None,
            "TOP": (0, 1),
            "RIGHT": None,
            "BOTTOM": None,
        },
        # room_type 4
        {
            "LEFT": None,
            "TOP": (-1, 0),
            "RIGHT": (0, 1),
            "BOTTOM": None,
        },
        # room_type 5
        {
            "LEFT": (0, 1),
            "TOP": (1, 0),
            "RIGHT": None,
            "BOTTOM": None,
        },
        # room_type 6
        {
            "LEFT": (1, 0),
            "TOP": None,
            "RIGHT": (-1, 0),
            "BOTTOM": None,
        },
        # room_type 7
        {
            "LEFT": None,
            "TOP": (0, 1),
            "RIGHT": (0, 1),
            "BOTTOM": None,
        },
        # room_type 8
        {
            "LEFT": (0, 1),
            "TOP": None,
            "RIGHT": (0, 1),
            "BOTTOM": None,
        },
        # room_type 9
        {
            "LEFT": (0, 1),
            "TOP": (0, 1),
            "RIGHT": None,
            "BOTTOM": None,
        },
        # room_type 10
        {
            "LEFT": None,
            "TOP": (-1, 0),
            "RIGHT": None,
            "BOTTOM": None,
        },
        # room_type 11
        {
            "LEFT": None,
            "TOP": (1, 0),
            "RIGHT": None,
            "BOTTOM": None,
        },
        # room_type 12
        {
            "LEFT": (0, 1),
            "TOP": None,
            "RIGHT": None,
            "BOTTOM": None,
        },
        # room_type 13
        {
            "LEFT": (0, 1),
            "TOP": None,
            "RIGHT": None,
            "BOTTOM": None,
        },
    ]

    def __init__(self, x: int, y: int, room_type: int):
        self._type = abs(room_type)
        self._locked = bool(room_type < 0),
        self._x = x
        self._y = y
        self._paths = Room._paths_table[self._type]

    # Param  entry: "LEFT" | "TOP" | "RIGHT"
    # Returns  (exit_x, exit_y) | None
    def get_path(entry) -> tuple[int, int] | None:
        return self._paths[entry]


    # direction: "LEFT" (counterclockwise) | "RIGHT" (clockwise)
    def rotate(direction) -> None:
        if self._locked:
            return

        def rotate(vec):
            if vec is None:
                return None
            rot = [(0, 1), (-1, 0)] if direction == "LEFT" else [(0, -1), (1, 0)]
            return (rot[0][0] * vec[0] + rot[0][1] * vec[1],
                    rot[1][0] * vec[0] + rot[1][1] * vec[1])

        self._paths = {
            "LEFT": rotate(self._paths["TOP"]),
            "TOP": rotate(self._paths["RIGHT"]),
            "RIGHT": rotate(self._paths["BOTTOM"]),
            "BOTTOM": rotate(self._paths["LEFT"]),
        }


def initial_input():
    grid = []
    w, h = [int(x) for x in input().split()]
    for y in range(h):
        _in = input()
        grid.extend([Room(x, y, int(t)) for x, t in enumerate(_in.split())])
    ex = int(input())

    return grid, w, h, ex


def turn_input():
    x, y, pos = input().split()
    x, y = int(x), int(y)

    rocks = []
    r = int(input())
    for _ in range(r):
        xr, yr, posr = input().split()
        xr, yr = int(xr), int(yr)
        rocks.append((xr, yr, posr))

    return x, y, pos, rocks


if __name__ == "__main__":
    grid, w, h, ex = initial_input()
    for i, room in enumerate(grid):
        print(f"{room._type}", end=' ' if (i + 1) % w else '\n', file=sys.stderr)

    while True:
        x, y, pos, rocks = turn_input()

        print("WAIT")
