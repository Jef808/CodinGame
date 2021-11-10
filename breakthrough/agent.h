#include "types.h"
#include "breakthrough.h"

class Agent {

    struct ExtMove {
        Move move;
        int value;
        ExtMove()
            : move{ Move_None }
            , value{ -32001 }
        {
        }
        explicit ExtMove(Move _move)
            : move{ _move }
            , value{ -32001 }
        {
        }
        ExtMove(Move m, int v)
            : move{ m }
            , value{ v }
        {
        }
        operator Move() const {
            return this->move;
        }
        bool operator<(const ExtMove& move) const {
            return this->value > move.value;
        }
        bool operator==(const Move& move) const
        {
            return move == this->move;
        }
        bool operator!=(const ExtMove& emove) const
        {
            return !(this->move == emove.move);
        }
    };

    struct Stack {
        int depth;
        int move_count;
    };

public:
    Agent(Game&);
    /** Pick the the move maximizing the result of the score_move() method */
    Move simple_best_move(bool debug_eval = false);

    /** Same as simple_best_move but run minimax at depth `search_depth' to pick the best move */
    Move best_move(int search_depth, int search_width = max_n_moves);

    /** Generate root moves and order them wrt to the score_move() method */
    void make_root();

    /** Test method for checking for passed pawns */
    void debug();

private:
    Game& game;
    std::vector<ExtMove> root_moves;
    std::vector<Move> move_buf;
    StateInfo states[max_depth];
    int n_evals = 0;

    /** Witout alpha beta pruning */
    int simple_eval_minimax(int depth, Stack* ss);

    /** With alpha beta pruning */
    int eval_minimax(int alpha, int beta, int depth, int width, Stack* ss);
};
