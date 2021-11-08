#include "types.h"
#include "breakthrough.h"

class Agent {

    struct ExtMove {
        Move move;
        int value;
        explicit ExtMove(Move _move)
            : move { _move }
            , value { -32001 }
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
    Move simple_best_move(bool debug = false);
    Move best_move(int search_width, int search_depth);
    void make_root();

private:
    Game& game;
    std::vector<ExtMove> root_moves;
    std::vector<Move> move_buf;
    StateInfo states[max_depth];
    int n_evals = 0;

    int eval_minimax(int alpha, int beta, int depth, int width, Stack* ss);
    int score_move(const Move&, bool debug = false) const;
};
