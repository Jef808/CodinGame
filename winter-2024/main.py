from enum import Enum
from collections import deque
from typing import NamedTuple
import sys

# GLOBALS
width: int
height: int
width, height = [int(i) for i in input().split()]


def log(*message: str):
    print(*message, file=sys.stderr)


WALL: str = 'WALL'


class ProteinType(Enum):
    A = 'A'
    B = 'B'
    C = 'C'
    D = 'D'


class DirectionType(Enum):
    N = 'N'
    E = 'E'
    S = 'S'
    W = 'W'


class OrganType(Enum):
    ROOT = 'ROOT'
    BASIC = 'BASIC'
    SPORER = 'SPORER'
    HARVERSTER = 'HARVESTER'
    TENTACLE = 'TENTACLE'


class Pos(NamedTuple):
    x: int
    y: int


class Organ(NamedTuple):
    id: int
    owner: int
    parent_id: int
    root_id: int
    pos: Pos
    organ_type: OrganType
    dir: DirectionType


class Cell(NamedTuple):
    pos: Pos
    isWall: bool = False
    protein: ProteinType | None = None
    organ: Organ | None = None


class Grid:
    cells: list[Cell] = []

    def __init__(self) -> None:
        self.reset()

    def reset(self) -> None:
        self.cells = []
        for y in range(height):
            for x in range(width):
                self.cells.append(Cell(Pos(x, y)))

    def get_cell(self, pos: Pos) -> Cell | None:
        if width > pos.x >= 0 and height > pos.y >= 0:
            return self.cells[pos.x + width * pos.y]
        return None

    def set_cell(self, pos: Pos, cell: Cell) -> None:
        self.cells[pos.x + width * pos.y] = cell


class Game:
    grid: Grid
    my_proteins: dict[ProteinType, int]
    opp_proteins: dict[ProteinType, int]
    my_organs: list[Organ]
    opp_organs: list[Organ]
    organ_map: dict[int, Organ]

    def __init__(self) -> None:
        self.grid = Grid()
        self.reset()

    def reset(self) -> None:
        self.my_proteins = {}
        self.opp_proteins = {}
        self.grid.reset()
        self.my_organs = []
        self.opp_organs = []
        self.organ_map = {}


def main():
    game: Game = Game()

    turn = 0

    # game loop
    while True:
        game.reset()

        entity_count: int = int(input())
        for _ in range(entity_count):
            inputs = input().split()
            x = int(inputs[0])
            y = int(inputs[1])
            pos = Pos(x, y)
            _type = inputs[2]
            owner = int(inputs[3])
            organ_id = int(inputs[4])
            organ_dir = inputs[5]
            organ_parent_id = int(inputs[6])
            organ_root_id = int(inputs[7])

            cell: Cell | None = None
            organ: Organ | None = None

            if _type == WALL:
                cell = Cell(pos, True)
            elif _type in [protein.value for protein in ProteinType]:
                cell = Cell(pos, False, ProteinType(_type))
            else:
                organ = Organ(organ_id, owner, organ_parent_id, organ_root_id, pos, OrganType(_type), DirectionType(organ_dir))
                cell = Cell(pos, False, None, organ)
                if owner == 1:
                    game.my_organs.append(organ)
                else:
                    game.opp_organs.append(organ)
                game.organ_map[organ_id] = organ

            if cell != None:
                game.grid.set_cell(pos, cell)

        my_proteins = [int(i) for i in input().split()]
        opp_proteins = [int(i) for i in input().split()]

        game.my_proteins = { ProteinType.A: my_proteins[0], ProteinType.B: my_proteins[1], ProteinType.C: my_proteins[2], ProteinType.D: my_proteins[3] }
        game.opp_proteins = { ProteinType.A: opp_proteins[0], ProteinType.B: opp_proteins[1], ProteinType.C: opp_proteins[2], ProteinType.D: opp_proteins[3] }

        required_actions_count = int(input())

        for _ in range(required_actions_count):
            # Write an action using print
            # To debug: print("Debug messages...", file=sys.stderr, flush=True)
            if turn == 0:
                log("First turn")
                root_id = game.my_organs[0].id
                print(f"GROW {root_id} 2 2 SPORER")
            elif turn == 1:
                log("Second turn")
                sporer_id = game.my_organs[1].id
                print(f"SPORE {sporer_id} 15 2")

            turn += 1


if __name__ == "__main__":
    main()
