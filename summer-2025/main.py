import sys
from dataclasses import dataclass, field
from enum import Enum

class Tile(Enum):
    EMPTY = 0
    LOW_COVER = 1
    HIGH_COVER = 2

@dataclass(frozen=True)
class Point:
    x: int = -1
    y: int = -1

@dataclass
class Agent:
    # agent_id: Unique identifier for this agent
    # player: Player id of this agent
    # shoot_cooldown: Number of turns between each of this agent's shots
    # optimal_range: Maximum manhattan distance for greatest damage output
    # soaking_power: Damage output within optimal conditions
    # splash_bombs: Number of splash bombs this can throw this game
    # cooldown: Number of turns before this agent can shoot
    # wetness: Damage (0-100) this agent has taken
    # pos: Position of this agent on the map
    # command: Command to execute this turn
    agent_id: int
    player: int
    shoot_cooldown: int
    optimal_range: int
    soaking_power: int
    splash_bombs: int
    commands: list[str]
    cooldown: int = 0
    wetness: int = 0
    is_dead: bool = False
    pos: Point = field(default_factory=Point)
    messages: list[str] = field(default_factory=list)


agents: dict[int, Agent] = {}
tiles: dict[Point, Tile] = {}

def distance(a: Point, b: Point) -> int:
    """Calculate the Manhattan distance between two points."""
    return abs(a.x - b.x) + abs(a.y - b.y)


def get_cover_positions(defender_pos: Point, attacker_pos: Point) -> list[Point]:
    """Determine the tile positions which could provide cover."""
    cover_positions: list[Point] = []

    # Check the rightwards direction
    if (
            attacker_pos.x == defender_pos.x + 2 and abs(attacker_pos.y - defender_pos.y) > 1
            or attacker_pos.x > defender_pos.x + 2
    ):
        cover_positions.append(Point(defender_pos.x + 1, defender_pos.y))

    # Check the leftwards direction
    if (
            attacker_pos.x == defender_pos.x - 2 and abs(attacker_pos.y - defender_pos.y) > 1
            or attacker_pos.x < defender_pos.x - 2
    ):
        cover_positions.append(Point(defender_pos.x - 1, defender_pos.y))

    # Check the upwards direction
    if (
            attacker_pos.y == defender_pos.y - 2 and abs(attacker_pos.x - defender_pos.x) > 1
            or attacker_pos.y < defender_pos.y - 2
    ):
        cover_positions.append(Point(defender_pos.x, defender_pos.y - 1))

    # Check the downwards direction
    if (
            attacker_pos.y == defender_pos.y + 2 and abs(attacker_pos.x - defender_pos.x) > 1
            or attacker_pos.y > defender_pos.y + 2
    ):
        cover_positions.append(Point(defender_pos.x, defender_pos.y + 1))

    return cover_positions


my_id = int(input())  # Your player id (0 or 1)
agent_data_count = int(input())  # Total number of agents in the game

for i in range(agent_data_count):
    agent_id, player, shoot_cooldown, optimal_range, soaking_power, splash_bombs = [int(j) for j in input().split()]
    agents[agent_id] = Agent(
        agent_id=agent_id,
        player=player,
        shoot_cooldown=shoot_cooldown,
        optimal_range=optimal_range,
        soaking_power=soaking_power,
        splash_bombs=splash_bombs,
        commands=[str(agent_id)]
    )

# width: Width of the game map
# height: Height of the game map
width, height = [int(i) for i in input().split()]
for i in range(height):
    inputs = input().split()
    for j in range(width):
        # x: X coordinate, 0 is left edge
        # y: Y coordinate, 0 is top edge
        # tile_type:
        #   0: Empty tile
        #   1: Low Cover
        #   2: High Cover
        x = int(inputs[3*j])
        y = int(inputs[3*j+1])
        tile_type = int(inputs[3*j+2])

        tiles[Point(x, y)] = Tile(tile_type)

# game loop
turn = -1
while True:
    turn += 1

    agent_count = int(input())  # Total number of agents still in the game
    agents_alive: set[int] = set()

    for i in range(agent_count):
        agent_id, x, y, cooldown, splash_bombs, wetness = [int(j) for j in input().split()]
        agents_alive.add(agent_id)
        agents[agent_id].pos = Point(x, y)
        agents[agent_id].cooldown = cooldown
        agents[agent_id].splash_bombs = splash_bombs
        agents[agent_id].wetness = wetness
        agents[agent_id].commands = agents[agent_id].commands[:1]
        agents[agent_id].messages.clear()

    for dead_agent_id in set(agents.keys()) - agents_alive:
        agents[dead_agent_id].is_dead = True

    my_agent_count = int(input())  # Number of alive agents controlled by you

    my_agents = [i for i in agents if agents[i].player == my_id and not agents[i].is_dead]
    opp_agents = [i for i in agents if agents[i].player != my_id and not agents[i].is_dead]

    distance_map = {i: {j: distance(agents[i].pos, agents[j].pos) for j in opp_agents} for i in my_agents}

    for agent_id in my_agents:
        attacker_ids = [j for j in opp_agents if distance_map[agent_id][j] <= agents[j].optimal_range]
        defender_ids = [j for j in opp_agents if distance_map[agent_id][j] <= agents[agent_id].optimal_range]

        current_agent_pos = agents[agent_id].pos

        #######################################################
        # Find the best cover point for each one of my agents #
        #######################################################
        best_cover_point = None
        best_cover_score = 0

        for dx, dy in [(0, -1), (1, 0), (0, 1), (-1, 0)]:
            candidate_pos = Point(current_agent_pos.x + dx, current_agent_pos.y + dy)

            if candidate_pos not in tiles or tiles[candidate_pos] != Tile.EMPTY:
                continue

            # Find the best cover tile around the candidate position
            candidate_cover_score = 0
            for opp_agent_id in attacker_ids:
                best_tile_value = 0
                for cover_point in get_cover_positions(candidate_pos, agents[opp_agent_id].pos):
                    tile_type = tiles.get(cover_point, Tile.EMPTY)

                    # TODO: Weight the cover value based on attacker's optimal range
                    # and attacker's soaking power
                    tile_cover_value = 0 if tile_type == Tile.EMPTY else (50 if tile_type == Tile.LOW_COVER else 75)

                    if tile_cover_value > best_tile_value:
                        best_tile_value = tile_cover_value

                candidate_cover_score += best_tile_value

            if candidate_cover_score > best_cover_score:
                best_cover_score = candidate_cover_score
                best_cover_point = candidate_pos

        if turn == 0 and best_cover_point is not None:
            agents[agent_id].commands.append(f"MOVE {best_cover_point.x} {best_cover_point.y}")
            current_agent_pos = best_cover_point

        #################################################
        # Find the least covered opponents within range #
        #################################################
        least_covered_defender = None
        least_covered_defender_score = 100  # The best cover score possible is 75

        for opp_agent_id in defender_ids:
            best_tile_value = 0
            for cover_point in get_cover_positions(agents[opp_agent_id].pos, current_agent_pos):
                tile_type = tiles.get(cover_point, Tile.EMPTY)
                cover_value = 0 if tile_type == Tile.EMPTY else (50 if tile_type == Tile.LOW_COVER else 75)

                if cover_value > best_tile_value:
                    best_tile_value = cover_value

            if best_tile_value < least_covered_defender_score:
                least_covered_defender_score = best_tile_value
                least_covered_defender = opp_agent_id

        if least_covered_defender is not None:
            agents[agent_id].commands.append(f"SHOOT {least_covered_defender}")

    for agent_id in my_agents:
        command_string = ';'.join(agents[agent_id].commands)

        if agents[agent_id].messages:
            command_string += f";MESSAGE {' -- '.join(agents[agent_id].messages)}"

        print(command_string)
