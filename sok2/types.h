#ifndef TYPES_H_
#define TYPES_H_

namespace sok2 {


enum class Heat { unknown, cold, warm, neutral };

struct Point {
    int x;
    int y;
};

using Window = Point;

struct Building {
    int width;
    int height;
};

struct Game {
    Building building;
    int turns_left;
    Window current_pos;
};


}  // namespace sok2

#endif // TYPES_H_
