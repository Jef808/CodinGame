#ifndef AGENT_H_
#define AGENT_H_

namespace dp {
class Game;
enum class Action;

namespace agent {
    /// The main search method
    void search(const dp::Game&);

    /// The best choice to date
    dp::Action best_choice();

} // namespace agent
} // namespace dp

#endif // AGENT_H_
