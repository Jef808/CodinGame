#ifndef AGENT_H_
#define AGENT_H_

#include "dp.h"

namespace dp {

class Agent {
public:
    static void init(const Game&);
    Agent() = default;
    /// The main search method
    void search();
    /// The best choice to date
    std::string best_choice();
private:
};


} // namespace dp

#endif // AGENT_H_
