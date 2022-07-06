/**
 * Implementation of a class to perform the "game-tree search".
 */
#ifndef AGENT_H_
#define AGENT_H_

#include "game.h"

#include <array>
#include <stdexcept>
#include <vector>

enum Direction {
  WEST = 1,     NORTH = 2,
  EAST = -WEST, SOUTH = -NORTH
};

/**
 * Helper structure to store lengths of shortest paths through
 * each parents of a square.
 */
class ParentDistances;

/**
 * Class used to perform computations on a given Game object
 * and generate the moves to play.
 */
class Agent {
public:
  /**
   * Construct an agent.
   *
   * @param game  Reference to the Game object in which the
   * underlying game's state and parameters live.
   */
  Agent(Game &game);

  /**
   * Try to pick a move on the fire's boundary, otherwise
   * pick at random
   */
  Move choose_move();

  /**
   * Choose a random action amonst all the
   * currently legal moves
   */
  Move choose_random_move() const;

  /**
   * Generate a lookup table for the number of turns the fire
   * takes to reach each square of the grid.
   *
   * Perform a BFS traversal of the grid starting at the
   * fire's origin, incrementing the distance according to
   * the squares' type. Also record the shortest distance
   * through each parent.
   */
  void generate_distance_map();


  // For debugging
  const std::vector<int>& view_distance_map() const { return m_distances; }

private:
  /** Local reference to the game's state and parameter. */
  Game &m_game;

  /* Temporary fix for calling choose_random_move by itself */
  bool generated_boundary = false;

  /**
   * Local record of the burning and cutting duration parameters.
   */
  struct {
    int fire_tree;
    int fire_house;
    int cut_tree;
    int cut_house;
  } Duration;

  /**
   * Local record of the Value by cell type.
   */
  struct {
    int tree;
    int house;
  } Value;

  /** The distance table */
  std::vector<int> m_distances;

  /** The distance value at each parent */
  std::vector<ParentDistances> m_parents;

  /**
   * Get the index of interior squares' neighbours.
   *
   * @param s  Index of an interior square.
   * @param d  Direction of the query.
   */
  size_t get_neighbour(size_t s, Direction d) const;
};

/**
 * Store the length of the shortest path going through each
 * parent by direction.
 */
class ParentDistances {
    std::array<int, 4> parents;
  public:
    ParentDistances() = default;

    int& operator[](int i) {
      return parents[index_map(i)];
    }
    int operator[](int i) const {
      return parents[index_map(i)];
    }

  private:
    int index_map(int d) const {
      return d + 3;
    }
  };


#endif // AGENT_H_
