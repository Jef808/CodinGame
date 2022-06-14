#ifndef UTILS_H_
#define UTILS_H_

#include "types.h"

#include <cmath>
#include <type_traits>

namespace sok2 {

/**
 * Compute the euclidean distance between two windows.
 */
inline double distance(const Window& a, const Window& b) {
    return std::sqrt((a.x - b.x) * (a.x - b.x)
                     + (a.y - b.y) * (a.y - b.y));
}

enum class Storage { RowMajor, ColMajor };

inline bool operator==(const Window& a, const Window& b) {
    return a.x == b.x && a.y == b.y;
}

// Sort to fit a row-major storage
template<Storage s>
struct Cmp;

template<>
struct Cmp<Storage::RowMajor> {
    bool operator()(const Window& a, const Window& b) {
        return a.y < b.y || (a.y == b.y && a.x < b.x);
    }
};

// Sort to fit a row-major storage
template<>
struct Cmp<Storage::ColMajor> {
    bool operator()(const Window& a, const Window& b) {
        return a.x < b.x || (a.x == b.x && a.y < b.y);
    }
};

template<Storage s>
struct NextWindow {
    const int width;
    const int height;
    Window current_window{ 0, 0 };

    NextWindow(int w, int h)
        : width{ w }, height { h }
    {}

    void set_current_window(const Window& w) {
        current_window = w;
    }

    std::enable_if_t<s == Storage::RowMajor, Window>
    operator()() {
        int x = (current_window.x + 1) % width;
        int y = (current_window.y + (x == 0));
        return { x, y };
    }
};

inline int n_windows(const Game& game) {
    return game.building.height * game.building.width;
}

} // namespace sok2

#endif // UTILS_H_
