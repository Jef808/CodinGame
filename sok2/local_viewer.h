#ifndef LOCAL_VIEWER_H_
#define LOCAL_VIEWER_H_

#include "types.h"
#include "utils.h"
#include "fakejudge.h"

#include <algorithm>
#include <string_view>
#include <string>
#include <vector>

namespace sok2::viewer {


const Storage storage = Storage::RowMajor;
enum class CC { colder, warmer, same, current_window, reset };

const std::string colder_cc = "/033e[0m";
const std::string reset_cc = "/033e[0m";
const std::string warmer_cc = "/033e[0;32m";
const std::string same_cc = "/033e[0;33m";
const std::string current_window_cc = "/033e[1;32m";

inline std::string_view to_string(CC cc);

/**
 * Add viewing functionalities to the FakeJudge class
 */
class FakeJudgeWithView : fakejudge::FakeJudge {
public:
    FakeJudgeWithView(Game& game, const Window& bomb)
        : fakejudge::FakeJudge{ game, bomb }
    {
        m_colder.reserve(n_windows(this->game()));
        m_warmer.reserve(n_windows(this->game()));
        m_same.reserve(std::max(width(), height()));
        for (int i=0; i<this->game().building.height; ++i) {
            for (int j=0; j<this->game().building.width; ++j) {
                m_warmer.emplace_back(j, i);
            }
        }
        m_window_colors.reserve(n_windows(this->game()));
    }

    void update_data() {
        const Window& last_window = m_history.back();
        const Window& cur_window  = this->game().current_pos;
        auto prev_last_colder = m_colder.end();
        auto prev_last_same = m_same.end();

        for (const auto& win : m_warmer) {
            const double prev_distance = distance(win, last_window);
            const double new_distance  = distance(win, cur_window);
            if (new_distance < prev_distance) {
                m_colder.push_back(win);
            }
            else if (new_distance == prev_distance) {
                m_same.push_back(win);
            }
        }

        auto should_remove = [&](const Window& win) {
            auto contains = [](auto beg, const auto end, const auto& el) {
               return std::find(beg, end, el) != end;
            };
            return contains(prev_last_colder, m_colder.end(), win)
                || contains(prev_last_same, m_same.end(), win);
        };

        m_warmer.erase(
            std::remove_if( m_warmer.begin(), m_warmer.end(), should_remove));
    }

    void sort_data() {
        const auto cmp = Cmp<storage>{};
        for (auto& windows : {m_colder, m_warmer, m_same}) {
            std::sort(windows.begin(), windows.end(), cmp);
        }
    }

    void sort_window_colors() {
        auto cmp = [cmp_wins=Cmp<storage>{}](const auto& wcc_a, const auto& wcc_b) {
            return cmp_wins(wcc_a.first, wcc_b.first);
        };

        std::sort(m_window_colors.begin(), m_window_colors.end(), cmp);
    }

    void update_window_colors() {
        m_window_colors.clear();

        for (const auto& [i, windows] : {std::make_pair(0, m_colder), std::make_pair(1, m_warmer), std::make_pair(2, m_same)}) {
            const CC cc = i == 0 ? CC::colder : i == 1 ? CC::warmer : CC::same;
            std::transform(windows.begin(), windows.end(), std::back_inserter(m_window_colors), [&cc](const auto& win) {
                    return std::make_pair(win, cc);
            });
        }
    }

    void view(std::ostream& os) {
        sort_data();
        update_window_colors();
        sort_window_colors();

        const char square = (char)(219);
        for (int i=0; i<height(); ++i) {
            for (int j=0; j<width(); ++j) {
                os << to_string(m_window_colors[j + width() * i].second)
                   << square;
            }
            os << '\n';
        }
        os << to_string(CC::reset) << std::endl;
    }

private:
    std::vector<Window> m_history;
    std::vector<Window> m_colder;
    std::vector<Window> m_warmer;
    std::vector<Window> m_same;
    std::vector<std::pair<Window, CC>> m_window_colors;

    int width() const { return this->game().building.width; }
    int height() const { return this->game().building.height; }
};


inline std::string_view to_string(CC cc) {
    switch (cc) {
        case CC::colder:         return colder_cc;
        case CC::warmer:         return warmer_cc;
        case CC::same:           return same_cc;
        case CC::current_window: return current_window_cc;
        case CC::reset:          return reset_cc;
        default: assert(false && "Invalid CC input at local_viewer.h:to_string");
    }
}


} // namespace sok2::viewer

#endif // LOCAL_VIEWER_H_
