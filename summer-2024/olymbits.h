#ifndef __OLYMBITS_H__
#define __OLYMBITS_H__

#include <array>
#include <cmath>
#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

namespace olymbits {

/**
 * @brief Struct reprensenting the State of a mini-game.
 */
struct State {
    std::string gpu;
    std::array<int, 7> regs {-1,-1,-1,-1,-1,-1,-1};
};

/**
 * @brief Enum class for the actions available to perform.
 */
enum class Action {
    LEFT = 0,
    UP = 1,
    RIGHT = 2,
    DOWN = 3,
};

enum class Medal {
    GOLD,
    SILVER,
    BRONZE,
};

/**
 * @brief Base class representing a generic mini-game.
 *
 * This class serves as an interface for the various mini-games in the
 * Olymbits challenge. It provides a common structure for storing game
 * state and defining essential methods.
 */
class MiniGame {
public:
    /**
     * @brief Default constructor for the MiniGame class.
     */
    MiniGame() = default;

    /**
     * @brief Sets the state of the mini-game.
     *
     * This method updates the ASCII representation or other relevant string data
     * for the mini-game and the array of registers used by the mini-game.
     *
     * @param gpu ASCII representation or other relevant string data for the mini-game.
     * @param regs Array of 7 integers representing the registers used by the mini-game.
     */
    virtual void set_state(State state) {
        m_state = state;
    }

    /**
     * @brief Return the current state of the mini-game.
     */
    virtual State state() const {
        return m_state;
    };

    /**
     * @brief Update the current state of the mini-game from the input stream.
     */
    virtual void update(std::istream&);

    /**
     * @brief Advances the game state to the next turn according to the given action.
     *
     * This is a pure virtual method that must be implemented by derived classes to define
     * the logic for advancing the game state.
     */
    virtual void step(const std::array<Action, 3>&) = 0;

    /**
     * @brief Verifies if the mini-game has ended.
     *
     * This simply checks whether the `gpu` string is equal to "GAME OVER".
     */
    virtual bool is_terminal() const {
        return m_state.gpu == "GAME OVER";
    }

    /**
     * @brief Compute the medals earned by each players in the mini-game once it is over.
     *
     * This is a pure virtual method that must be implemented by derived classes to define
     * the logic for computing the score of each player.
     */
    virtual std::array<Medal, 3> score() const = 0;

    /**
     * @brief Compute the score of each players in the mini-game.
     */

    /**
     * @brief Virtual destructor for the MiniGame class.
     */
    virtual ~MiniGame() = default;

protected:
    State m_state;
};


/**
 * @brief Class representing the Hurdle Race mini-game.
 *
 * In this mini-game, players race on a track with hurdles. The goal is to finish the race
 * as quickly as possible while avoiding hurdles that can stun the player.
 */
class HurdleRace : public MiniGame {
public:
    /**
     * @brief Advances the Hurdle Race game state to the next turn.
     *
     * This method implements the logic for progressing the Hurdle Race mini-game by one turn.
     */
    void step(const std::array<Action, 3>&) override;

    /**
     * @brief Compute the medals earned by each players in once the HurdleRace is over.
     */
    std::array<Medal, 3> score() const override;
};

/**
 * @brief Class representing the Archery mini-game.
 *
 * In this mini-game, players control a cursor affected by wind strength. The goal is to
 * move the cursor as close as possible to the target coordinates (0,0) by the end of the game.
 */
class Archery : public MiniGame {
public:
    /**
     * @brief Advances the Archery game state to the next turn.
     *
     * This method implements the logic for progressing the Archery mini-game by one turn.
     */
    void step(const std::array<Action, 3>&) override;

    /**
     * @brief Compute the medals earned by each players in once the Archery game is over.
     */
    std::array<Medal, 3> score() const override;
};

/**
 * @brief Class representing the Roller Speed Skating mini-game.
 *
 * In this mini-game, players race on a cyclical track 10 spaces long. Each player has a risk
 * attribute ranging from 0 to 5. Players move forward based on the action chosen from a risk
 * order provided each turn.
 */
class RollerSpeedSkating : public MiniGame {
public:
    /**
     * @brief Advances the Roller Speed Skating game state to the next turn.
     *
     * This method implements the logic for progressing the Roller Speed Skating mini-game by one turn.
     */
    void step(const std::array<Action, 3>&) override;

    /**
     * @brief Compute the medals earned by each players in once the RollerSpeedSkating game is over.
     */
    std::array<Medal, 3> score() const override;
};

/**
 * @brief Class representing the Diving mini-game.
 *
 * In this mini-game, players must match the sequence of directions given at the start of each run,
 * called the diving goal. Players earn points based on their combo multiplier, which increases
 * with consecutive matches.
 */
class Diving : public MiniGame {
public:
    /**
     * @brief Advances the Diving game state to the next turn.
     *
     * This method implements the logic for progressing the Diving mini-game by one turn.
     */
    void step(const std::array<Action, 3>&) override;

    /**
     * @brief Compute the medals earned by each players in once the Diving game is over.
     */
    std::array<Medal, 3> score() const override;
};

/**
 * @brief Class representing the global Olymbits challenge.
 */
class Olymbits {
public:
    Olymbits() = default;

    /**
     * @brief Initializes the Olymbits challenge by reading the initial state.
     */
    void init(std::istream&);

    /**
     * @brief Initializes one turn of the Olymbits challenge by reading the state of each games.
     */
    void turn_init(std::istream&);

    /**
     * @brief Advances the four mini-games to the next turn according to the given action.
     *
     * This method updates the state of each mini-game by calling their respective `step` methods.
     * It also increments the number of turns and updates the scores of each player when a mini-game ends.
     */
    void step(std::array<Action, 3> action);

    /**
     * @brief Returns the current state of each of the four mini-games.
     */
    std::array<State, 4> state() const;

    /**
     * @brief Returns the current player ID.
     */
    int player_id() const {
        return m_player_id;
    }

    /**
     * @brief Verifies if the Olymbits challenge has ended.
     *
     * This method checks if the number of turns has reached 100.
     */
    bool is_terminal() const {
        return m_nturns == 100;
    }

    /**
     * @brief Returns the score of each player in the Olymbits challenge.
     */
    std::array<std::array<int, 4>, 3> score() const {
        return m_scores;
    };

private:
    int m_nturns{ 0 };
    int m_player_id{-1};
    int m_ngames{ 4 };
    std::array<std::array<int, 4>, 3> m_scores;
    std::array<std::unique_ptr<MiniGame>, 4> m_mini_games {
        std::make_unique<HurdleRace>(),
        std::make_unique<Archery>(),
        std::make_unique<RollerSpeedSkating>(),
        std::make_unique<Diving>()
    };
};

} // namespace olymbits


#endif // __OLYMBITS_H__
