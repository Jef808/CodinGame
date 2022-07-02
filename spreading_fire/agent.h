/**
 * Implementation of various algorithms.
 */
#ifndef AGENT_H_
#define AGENT_H_

#include "distance.h"
#include "game.h"

/**
 * Agent's constructor initializes tables for distances
 * lookup and shortest paths.
 */
class Agent {
public:
  Agent(Game &game);

  void init();

  /**
   * The main entry point for playing against the online judge.
   */
  Move choose_move();

  Move choose_random_move();

  size_t get_point_towards_houses();

private:
  Game &m_game;
  Point O;
  GridDistance fire_expand;
  GridDistance house_distance;
  std::vector<size_t> m_houses;

  /** Renormalized so that duration_cut == 1 */
  double rel_duration_fire;
};

#endif // AGENT_H_
