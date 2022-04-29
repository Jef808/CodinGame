#ifndef AGENT_H_
#define AGENT_H_

#include "utilities.h"
#include "escape_cat.h"

#include <string>
#include <string_view>


namespace escape_cat {



class Agent {
public:
    Agent() = default;

    /**
     * Just aim a bit lower than that danger zone at every turn
     */
    Point_Euc choose_move(const Game& game, std::string& debug);

    /**
     * Submission requires integer coordinates for the point p,
     * and allows a debug message which will be displayed.
     *
     * Note that the part of the debug message is only fed with the first 27
     * characters of @debug_string, per the submission rules of CodinGame.
     */
    void format_choice_for_submission(const Point_Euc& p, std::string_view debug_string, std::string& buffer);

};

} // namespace



#endif // AGENT_H_
