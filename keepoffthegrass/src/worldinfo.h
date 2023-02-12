#ifndef WORLDINFO_H_
#define WORLDINFO_H_

#include <functional>
#include <vector>

#include "world.h"

constexpr int ME = 1;
constexpr int OPP = 0;
constexpr int NONE = -1;

class WorldInfo {
  public:
    explicit WorldInfo(World& world);

    /**
     * Update the tiles in `m_world` from standard input and
     * adjust the data of this class.
     *
     * This should be called before every turn of the game.
     */
    void update();

    [[nodiscard]] const int my_matter() const { return m_my_matter; }
    [[nodiscard]] const int opp_matter() const { return m_opp_matter; }
    [[nodiscard]] const std::vector<std::reference_wrapper<const Tile>>& my_tiles() const { return m_my_tiles; }
    [[nodiscard]] const std::vector<std::reference_wrapper<const Tile>>& opp_tiles() const { return m_opp_tiles; }
    [[nodiscard]] const std::vector<std::reference_wrapper<const Tile>>& neutral_tiles() const { return m_neutral_tiles; }
    [[nodiscard]] const std::vector<std::reference_wrapper<const Tile>>& my_units() const { return m_my_units; }
    [[nodiscard]] const std::vector<std::reference_wrapper<const Tile>>& opp_units() const { return m_opp_units; }
    [[nodiscard]] const std::vector<std::reference_wrapper<const Tile>>& my_recyclers() const { return m_my_recyclers; }
    [[nodiscard]] const std::vector<std::reference_wrapper<const Tile>>& opp_recyclers() const { return m_opp_recyclers; }

  private:
    World& m_world;
    int m_my_matter{0};
    int m_opp_matter{0};
    std::vector<std::reference_wrapper<const Tile>> m_my_tiles;
    std::vector<std::reference_wrapper<const Tile>> m_opp_tiles;
    std::vector<std::reference_wrapper<const Tile>> m_neutral_tiles;
    std::vector<std::reference_wrapper<const Tile>> m_my_units;
    std::vector<std::reference_wrapper<const Tile>> m_opp_units;
    std::vector<std::reference_wrapper<const Tile>> m_my_recyclers;
    std::vector<std::reference_wrapper<const Tile>> m_opp_recyclers;

    /**
     * Resets each member vector of this class.
     */
    void reset_data();
};

#endif // WORLDINFO_H_
