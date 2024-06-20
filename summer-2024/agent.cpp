#include "agent.h"

#include <algorithm>
#include <cassert>
#include <deque>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <vector>


namespace olymbits {

namespace {

std::unordered_map<char, int> action_index {
  {'L', 0}, {'U', 1}, {'R', 2}, {'D', 3}
};

}  // namespace

void swap_players_default(State& state, int player_id) {
    if (player_id == 0) {
        return;
    }
    int a = player_id;
    int b = 0;
    std::swap(state.regs[a], state.regs[b]);
    std::swap(state.regs[a+3], state.regs[b+3]);
}

void swap_players_archery(State& state, int player_id) {
    if (player_id == 0) {
        return;
    }
    int a = player_id;
    int b = 0;
    std::swap(state.regs[a], state.regs[2*b]);
    std::swap(state.regs[a+1], state.regs[2*b+1]);
}

std::array<int, 4> weight_hurdle_actions(const State& state) {
    std::array<int, 4> weights = {0, 0, 0, 0};
    auto pos = state.regs[0];
    auto stun = state.regs[1];

    if (stun > 0 or state.gpu == "GAME OVER") {
        return weights;
    }

    if (state.gpu[pos+1] == '#') {
        weights[0] = -4;
        weights[1] = 8;
        weights[2] = -4;
        weights[3] = -4;
    } else if (state.gpu[pos+2] == '#') {
        weights[0] = 8;
        weights[1] = -4;
        weights[2] = -4;
        weights[3] = -4;
    } else if (state.gpu[pos+3] == '#') {
        weights[0] = 1;
        weights[1] = 4;
        weights[2] = -3;
        weights[3] = 4;
    } else {
        weights[0] = 1;
        weights[1] = 2;
        weights[2] = 3;
        weights[3] = 2;
    }

    return weights;
}

std::array<int, 4> weight_archery_actions(const State& state) {
    std::array<int, 4> weights = {0, 0, 0, 0};

    if (state.gpu == "GAME OVER") {
        return weights;
    }

    return weights;
}

std::array<int, 4> weight_skating_actions(const State& state) {
    std::array<int, 4> weights = {0, 0, 0, 0};
    std::array<int, 4> order = {'L', 'U', 'R', 'D'};
    auto pos = state.regs[0];
    auto risk = state.regs[3];

    if (risk < 0 or state.gpu == "GAME OVER") {
        return weights;
    }

    std::sort(order.begin(), order.end(), [&gpu=state.gpu](char a, char b) {
        return gpu.find(a) < gpu.find(b);
    });

    if (risk == 4) {
        weights[action_index[order[0]]] = 4;
        weights[action_index[order[1]]] = 2;
        weights[action_index[order[2]]] = -8;
        weights[action_index[order[3]]] = -2;
    } else if (risk == 3) {
        weights[action_index[order[0]]] = 3;
        weights[action_index[order[1]]] = 2;
        weights[action_index[order[2]]] = -4;
        weights[action_index[order[3]]] = -6;
    } else if (risk == 2) {
        weights[action_index[order[0]]] = 1;
        weights[action_index[order[1]]] = 1;
        weights[action_index[order[2]]] = -2;
        weights[action_index[order[3]]] = 2;
    } else if (risk == 1) {
        weights[action_index[order[0]]] = 0;
        weights[action_index[order[1]]] = 2;
        weights[action_index[order[2]]] = 1;
        weights[action_index[order[3]]] = 3;
    } else {
        weights[action_index[order[0]]] = 0;
        weights[action_index[order[1]]] = 2;
        weights[action_index[order[2]]] = 1;
        weights[action_index[order[3]]] = 3;
    }

    return weights;
}

std::array<int, 4> weight_diving_actions(const State& state) {
    std::array<int, 4> weights = {0, 0, 0, 0};

    if (state.gpu == "GAME OVER") {
        return weights;
    }

    weights[action_index[state.gpu[0]]] = 2;

    return weights;
}

std::array<int, 4> get_action_weights(const Olymbits& game) {
    auto player_id = game.player_id();
    auto states = game.state();
    auto& hurdles_state = states[0];
    auto& archery_state = states[1];
    auto& skating_state = states[2];
    auto& diving_state = states[3];

    swap_players_default(hurdles_state, player_id);
    swap_players_archery(archery_state, player_id);
    swap_players_default(skating_state, player_id);
    swap_players_default(diving_state, player_id);

    auto hurdle_weights = weight_hurdle_actions(hurdles_state);
    auto archery_weights = weight_archery_actions(hurdles_state);
    auto skating_weights = weight_skating_actions(skating_state);
    auto diving_weights = weight_diving_actions(diving_state);

    auto weights = std::array<int, 4>{ 0, 0, 0, 0 };
    for (int i = 0; i < 4; ++i) {
        weights[i] = hurdle_weights[i] + archery_weights[i] + skating_weights[i] + diving_weights[i];
    }

    return weights;
}

} // namespace olymbits
