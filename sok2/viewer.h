#ifndef VIEWER_H_
#define VIEWER_H_

#include "referee.h"
#include "types.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <string_view>
#include <string>
#include <vector>

namespace sok2::viewer {


enum class CC { noncandidate, candidate, critical, next_window, last_window, bomb_window, reset };
inline std::string_view make_string(CC cc);


/**
 * Viewer for the game and the algorithm
 */
class Viewer {
public:
    Viewer(const Referee& referee)
        : m_referee{ referee }
        , m_last{ current() }
        , m_colors( width() * height(), CC::candidate )
    {
        m_noncandidates.reserve(width() * height());
    }

    void view(std::ostream& os) {
        update_candidates();
        color_special_squares();

        m_last = current();

        for (int y=0; y<height(); ++y) {
            for (int x=0; x<width(); ++x) {
                os << make_string(m_colors[x + width() * y]) << ' ';
            }
            os << '\n';
        }
        os << std::endl;
    }

    void color_special_squares() {
        /* If @c is a special square, we color it accordingly, without further processing. */
        enum class SSquare { none, last, next, bomb };

        const int last_ndx = m_last.x + width() * m_last.y;
        const int current_ndx = current().x + width() * current().y;

        for (int n=0; n<width() * height(); ++n) {
            SSquare ss = n == bomb_ndx ? SSquare::bomb
                : n == current_ndx ? SSquare::next
                : n == last_ndx ? SSquare::last
                : SSquare::none;

            switch (ss) {
                case SSquare::none:                                break;
                case SSquare::bomb: m_colors[n] = CC::bomb_window; break;
                case SSquare::next: m_colors[n] = CC::next_window; break;
                case SSquare::last: m_colors[n] = CC::last_window; break;
                default:
                    assert(false && "Invalid special square enum value in Viewer::move()");
            }
        }
    }

    void update_candidates() {
        /* Return early if there is nothing to update */
        if (m_last == current()) {
            return;
        }

        Heat move_heat = m_referee.get_heat(m_last, current());

        const Window& hot_window  = move_heat != Heat::cold ? current() : m_last;
        const Window& cold_window = move_heat != Heat::cold ? m_last    : current();

        auto it = m_noncandidates.begin();

        for (int c=0; c<width() * height(); ++c) {
            /*  If c is already a noncandidate, do nothing. */
            if (it != m_noncandidates.end() && c == *it) {
                ++it;
                m_colors[c] = CC::noncandidate;
                continue;
            }

            /*
             * Otherwise, we throw away the squares which are closer to the cold window
             * than the hot window. In case of move_heat being neutral, we throw away
             * all squares which are not equidistant to both windows.
             */
            Heat candidate_heat = compare_distances({c % width(), c / width()}, hot_window, cold_window);

            switch (candidate_heat) {
                case Heat::neutral:
                    m_colors[c] = CC::critical;
                    break;
                case Heat::cold:
                    m_noncandidates.push_back(c);
                    m_colors[c] = CC::noncandidate;
                    break;
                case Heat::warm:
                    if (move_heat == Heat::neutral) {
                        m_noncandidates.push_back(c);
                        m_colors[c] = CC::noncandidate;
                        break;
                    }
                    break;
                default:
                    assert(false && "Invalid candidate_heat in Viewer::move()");

            }
        }

        std::sort(m_noncandidates.begin(), m_noncandidates.end());
    }

    void view_legend(std::ostream& os) const {
        os << "\nNon-candidates: " << make_string(CC::noncandidate)  << '\n'
           << "Candidates: "       << make_string(CC::candidate)     << '\n'
           << "Delimiting line: "  << make_string(CC::critical)      << '\n'
           << "Last window: "      << make_string(CC::last_window)
             << " @ (" << m_last.x << ", " << m_last.y << ")\n"
           << "Current window: "      << make_string(CC::next_window)
             << " @ (" << current().x << ", " << current().y << ")\n"
           << "Bomb window: "      << make_string(CC::bomb_window)
             << " @ (" << bomb().x << ", " << bomb().y               << ")\n"
           << std::endl;
    }

private:
    const Referee& m_referee;
    const int bomb_ndx{ bomb().x + bomb().y * width() };
    Window m_last;
    std::vector<int> m_noncandidates;
    std::vector<int> m_candidates;
    std::vector<CC> m_colors;

    int width()             const { return m_referee.game().building.width; }
    int height()            const { return m_referee.game().building.height; }
    const Window& bomb()    const { return m_referee.bomb(); }
    const Window& current() const { return m_referee.game().current_pos; }
};


const std::string square_s      = "\u25A0";                               // unicode filled square
const std::string reset_s       = "\033[0m";                              // default

const std::string noncandidate_s= "\033[30;1;4m" + square_s + reset_s;    // black
const std::string candidate_s   = "\033[34;1;4m" + square_s + reset_s;    // blue
const std::string critical_s    = "\033[32;1;4m" + square_s + reset_s;    // green
const std::string next_window_s = "\033[35;1;4m" + square_s + reset_s;    // magenta
const std::string last_window_s = "\033[33;1;4m" + square_s + reset_s;    // yellow
const std::string bomb_window_s = "\033[31;1;4m" + square_s + reset_s;    // red


inline std::string_view make_string(CC cc) {
    switch (cc) {
        case CC::noncandidate:   return noncandidate_s;
        case CC::candidate:      return candidate_s;
        case CC::critical:       return critical_s;
        case CC::next_window:    return next_window_s;
        case CC::last_window:    return last_window_s;
        case CC::bomb_window:    return bomb_window_s;
        case CC::reset:          return reset_s;
        default:
            assert(false && "Invalid CC input at local_viewer.h:make_string");
    }
}

} // namespace sok2::viewer

#endif // VIEWER_H_
