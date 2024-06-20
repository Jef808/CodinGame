#include "olymbits.h"

#include <array>
#include <algorithm>
#include <iostream>
#include <numeric>
#include <sstream>
#include <stdexcept>

namespace olymbits {

namespace {
    static constexpr auto action_to_char = [](Action action) {
        switch (action) {
            case Action::LEFT:
                return 'L';
            case Action::UP:
                return 'U';
            case Action::RIGHT:
                return 'R';
            case Action::DOWN:
                return 'D';
        }
    };
}  // namespace

void MiniGame::update(std::istream& is) {
    m_state.gpu.clear();
    is >> m_state.gpu >> m_state.regs[0] >> m_state.regs[1]
       >> m_state.regs[2] >> m_state.regs[3] >> m_state.regs[4]
       >> m_state.regs[5] >> m_state.regs[6];
    is.ignore();
}

void HurdleRace::step(const std::array<Action, 3>& actions) {
    static constexpr auto find_next_hurdle = [](const std::string& gpu, int position) {
        auto hurdle = std::find(gpu.begin() + position, gpu.end(), '#');
        return hurdle == gpu.end() ? -1 : std::distance(gpu.begin(), hurdle);
    };

    if (m_state.gpu == "GAME OVER") {
        return;
    }
    for (auto player_id : {0, 1, 2}) {
        const auto action = actions[player_id];
        int& position = m_state.regs[player_id];
        int& stun_timer = m_state.regs[3 + player_id];

        if (stun_timer > 0) {
            --stun_timer;
            continue;
        }

        int move_distance = 0;
        int next_hurdle = find_next_hurdle(m_state.gpu, position);

        switch (action) {
            case Action::UP:
                move_distance = 2;
                break;
            case Action::LEFT:
                move_distance = 1;
                break;
            case Action::DOWN:
                move_distance = 2;
                break;
            case Action::RIGHT:
                move_distance = 3;
                break;
        }

        if (action != Action::UP) {
            position = std::min(position + move_distance, next_hurdle);
        } else {
            position += move_distance;
        }

        if (position == next_hurdle) {
            stun_timer = 3;
        }
    }
    if (std::any_of(m_state.regs.begin(), m_state.regs.begin() + 3, [&](int position) {
        return position >= 30;
    })) {
        m_state.gpu = "GAME OVER";
    }
}

std::array<Medal, 3> HurdleRace::score() const {
    if (m_state.gpu != "GAME OVER") {
        throw std::runtime_error("Game is not over yet");
    }
    std::array<Medal, 3> medals = {Medal::BRONZE, Medal::BRONZE, Medal::BRONZE};

    std::array<int, 3> positions;
    std::transform(m_state.regs.begin(), m_state.regs.begin() + 3, positions.begin(), [](int position) {
        return std::min(position, 30);
    });

    std::array<int, 3> players = {0, 1, 2};
    std::sort(players.begin(), players.end(), [&](int a, int b) {
        return positions[a] > positions[b];
    });

    medals[players[0]] = Medal::GOLD;
    if (positions[players[1]] == positions[players[0]]) {
        medals[players[1]] = Medal::GOLD;
    } else {
        medals[players[1]] = Medal::SILVER;
    }
    if (positions[players[2]] == positions[players[1]]) {
        medals[players[2]] = medals[players[1]];
    }

    return medals;
}

void Archery::step(const std::array<Action, 3>& actions) {
    if (m_state.gpu == "GAME OVER") {
        return;
    }
    for (auto player_id : {0, 1, 2}) {
        const auto action = actions[player_id];
        int& x = m_state.regs[player_id * 2];
        int& y = m_state.regs[player_id * 2 + 1];
        int wind_strength = m_state.gpu[0] - '0';

        switch (action) {
            case Action::UP:
                y += wind_strength;
                break;
            case Action::DOWN:
                y -= wind_strength;
                break;
            case Action::LEFT:
                x -= wind_strength;
                break;
            case Action::RIGHT:
                x += wind_strength;
                break;
        }

        // Ensure coordinates are within bounds [-20, 20]
        x = std::clamp(x, -20, 20);
        y = std::clamp(y, -20, 20);
    }
    m_state.gpu.erase(0, 1);
    if (m_state.gpu.empty()) {
        m_state.gpu = "GAME OVER";
    }
}

std::array<Medal, 3> Archery::score() const {
    if (m_state.gpu != "GAME OVER") {
        throw std::runtime_error("Game is not over yet");
    }
    std::array<Medal, 3> medals = {Medal::BRONZE, Medal::BRONZE, Medal::BRONZE};

    std::array<int, 3> players = {0, 1, 2};
    std::array<float, 3> distances;
    std::transform(players.begin(), players.end(), distances.begin(), [&](int player_id) {
        int x = m_state.regs[player_id * 2];
        int y = m_state.regs[player_id * 2 + 1];
        return x * x + y * y;
    });
    std::sort(players.begin(), players.end(), [&](int a, int b) {
        return distances[a] > distances[b];
    });

    medals[players[0]] = Medal::GOLD;
    if (distances[players[1]] == distances[players[0]]) {
        medals[players[1]] = Medal::GOLD;
    } else {
        medals[players[1]] = Medal::SILVER;
    }
    if (distances[players[2]] == distances[players[1]]) {
        medals[players[2]] = medals[players[1]];
    }

    return medals;
}

void RollerSpeedSkating::step(const std::array<Action, 3>& actions) {
    if (m_state.gpu == "GAME OVER") {
        return;
    }
    for (auto player_id : {0, 1, 2}) {
        const auto action = actions[player_id];
        int& spaces_traveled = m_state.regs[player_id];
        int& risk = m_state.regs[3 + player_id];
        if (risk < 0) {
            ++risk;
            continue;
        }
        auto action_char = action_to_char(action);
        auto risk_index = m_state.gpu.find(action_char);

        auto risk_delta = risk_index - 2;
        auto spaces_delta = risk_delta + 2;

        risk += risk_delta;
        spaces_traveled += spaces_delta;
    }

    // Check for collision with other players
    for (auto player_id : {0, 1, 2}) {
        for (auto other_player_id : {0, 1, 2}) {
            if (other_player_id != player_id && (m_state.regs[player_id] % 10 == m_state.regs[other_player_id] % 10)) {
                int& player_risk = m_state.regs[3 + player_id];
                int& other_player_risk = m_state.regs[3 + other_player_id];
                if (player_risk >= 0) {
                    player_risk += 2;
                }
                if (other_player_risk >= 0) {
                    other_player_risk += 2;
                }
            }
        }
    }

    // Set players to `stunned` if their risk is above 5
    for (auto player_id : {0, 1, 2}) {
        int& risk = m_state.regs[3 + player_id];
        if (risk >= 5) {
            risk = -2;
        }
    }

    --m_state.regs[6];
    if (m_state.regs[6] == 0) {
        m_state.gpu = "GAME OVER";
    }
}

std::array<Medal, 3> RollerSpeedSkating::score() const {
    if (m_state.gpu != "GAME OVER") {
        throw std::runtime_error("Game is not over yet");
    }
    std::array<Medal, 3> medals = {Medal::BRONZE, Medal::BRONZE, Medal::BRONZE};
    std::array<int, 3> players = {0, 1, 2};
    std::sort(players.begin(), players.end(), [&](int a, int b) {
        return m_state.regs[a] > m_state.regs[b];
    });

    medals[players[0]] = Medal::GOLD;
    if (m_state.regs[players[1]] == m_state.regs[players[0]]) {
        medals[players[1]] = Medal::GOLD;
    } else {
        medals[players[1]] = Medal::SILVER;
    }
    if (m_state.regs[players[2]] == m_state.regs[players[1]]) {
        medals[players[2]] = medals[players[1]];
    }

    return medals;
}

void Diving::step(const std::array<Action, 3>& actions) {
    if (m_state.gpu == "GAME OVER") {
        return;
    }
    for (auto player_id : {0, 1, 2}) {
        const auto action = actions[player_id];
        int& points = m_state.regs[player_id];
        int& combo = m_state.regs[3 + player_id];
        char diving_goal = m_state.gpu[0];

        if (diving_goal == action_to_char(action)) {
            ++combo;
            points += combo;
        } else {
            combo = 1;
        }
    }
    m_state.gpu.erase(0, 1);
    if (m_state.gpu.empty()) {
        m_state.gpu = "GAME OVER";
    }
}

std::array<Medal, 3> Diving::score() const {
    if (m_state.gpu != "GAME OVER") {
        throw std::runtime_error("Game is not over yet");
    }
    std::array<Medal, 3> medals = {Medal::BRONZE, Medal::BRONZE, Medal::BRONZE};
    std::array<int, 3> players = {0, 1, 2};
    std::sort(players.begin(), players.end(), [&](int a, int b) {
        return m_state.regs[a] > m_state.regs[b];
    });
    medals[players[0]] = Medal::GOLD;
    if (m_state.regs[players[1]] == m_state.regs[players[0]]) {
        medals[players[1]] = Medal::GOLD;
    } else {
        medals[players[1]] = Medal::SILVER;
    }
    if (m_state.regs[players[2]] == m_state.regs[players[1]]) {
        medals[players[2]] = medals[players[1]];
    }
    return medals;
}

void Olymbits::init(std::istream& is) {
    is >> m_player_id; is.ignore();
    is >> m_ngames; is.ignore();
}

void Olymbits::turn_init(std::istream& is) {
    static std::string buf;
    for (int i = 0; i < 3; ++i) {
        std::getline(is, buf);
        std::istringstream ss{buf};
        int total_score;
        std::array<std::array<int, 3>, 4> medals;
        ss >> total_score
           >> medals[0][0] >> medals[0][1] >> medals[0][2]
           >> medals[1][0] >> medals[1][1] >> medals[1][2]
           >> medals[2][0] >> medals[2][1] >> medals[2][2]
           >> medals[3][0] >> medals[3][1] >> medals[3][2];
        for (int g = 0; g < 4; ++g) {
            m_scores[i][g] = 3 * medals[g][0] + medals[g][1];
        }
    }
    for (int g = 0; g < m_ngames; ++g) {
        m_mini_games[g]->update(is);
    }
}

void Olymbits::step(std::array<Action, 3> action) {
    for (int g = 0; g < m_ngames; ++g) {
        const auto& mini_game = m_mini_games[g];
        mini_game->step(action);
        if (mini_game->is_terminal()) {
            auto score = mini_game->score();
            for (int i = 0; i < 3; ++i) {
                switch (score[i]) {
                    case Medal::GOLD:
                        m_scores[i][g] += 3;
                        break;
                    case Medal::SILVER:
                        m_scores[i][g] += 1;
                        break;
                    case Medal::BRONZE:
                        break;
                }
            }
        }
    }
    ++m_nturns;
}

std::array<State, 4> Olymbits::state() const {
    std::array<State, 4> states;
    std::transform(m_mini_games.begin(), m_mini_games.end(), states.begin(), [](const auto& game) {
        return game->state();
    });
    return states;
}

} // namespace olymbits
