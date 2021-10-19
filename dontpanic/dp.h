#ifndef DP_H_
#define DP_H_

#include <iosfwd>
#include <string>

namespace dp {

enum class Cell;
struct State;

class Game {
public:
    Game() = default;
    void init(std::istream&);
    void view() const;
    const State* state() const;
private:
    State* ps;
};

inline const State* Game::state() const { return ps; }

} // namespace dp

extern void extract_online_init(std::ostream&);



#endif // DP_H_
