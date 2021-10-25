#ifndef AGENT_H_
#define AGENT_H_

namespace dp {
class Game;
enum class Action;

namespace agent {
    void init(const dp::Game&);

    /// The main search method
    void search();

    /// The best choice to date
    dp::Action best_choice();

} // namespace agent
} // namespace dp

#endif // AGENT_H_
