#include <algorithm>
#include <iostream>
#include <iterator>
#include <set>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

struct Room {
    int id{-1};
    int value{0};
    std::set<int> children;
    std::set<int> parents;

    Room() = default;

    Room(int id, int value)
        : id{id}
        , value{value} { }
};

std::ostream& operator<<(std::ostream& out, const Room& room) {
    out << "{ id: " << room.id << ", value: " << room.value;

    out << ", children: [";
    const size_t n_children = room.children.size();
    if (n_children > 0) {
        size_t count = 0;
        for (const auto child : room.children) {
            out << child << (count < n_children - 1 ? ", " : "");
            ++count;
        }
    }

    out << "], parents: [";
    const size_t n_parents = room.parents.size();
    if (n_parents > 0) {
        size_t count = 0;
        for (const auto parent : room.parents) {
            out << parent << (count < n_parents - 1 ? ", " : "");
            ++count;
        }
    }

    return out << "] }";
}

/**
 * Parse a sequence of space-separated inputs.
 * If the next chunk represents a non-negative integer, return it, otherwise
 * return `-1`.
 */
inline int try_nonnegative_stoi(std::string_view sv) noexcept {
    size_t n = 0;
    try {
        int i = std::stoi(sv.data(), &n);
        return i >= 0 ? i : -1;
    } catch (std::exception& e) {
        return -1;
    }
}

std::vector<Room> input(std::istream& in) {
    std::vector<Room> rooms;

    int nb_rooms = 0;
    std::cin >> nb_rooms;
    std::cin.ignore();

    rooms.resize(nb_rooms);

    std::string buf;
    std::string nbh_buf1;
    std::string nbh_buf2;
    for (int i = 0; i < nb_rooms; ++i) {
        Room& room = rooms[i];

        std::getline(std::cin, buf);
        std::stringstream ss{buf};

        ss >> room.id >> room.value >> nbh_buf1 >> nbh_buf2;

        int nbh1_id = try_nonnegative_stoi(nbh_buf1);
        int nbh2_id = try_nonnegative_stoi(nbh_buf2);

        for (auto nbh_id : {nbh1_id, nbh2_id}) {
            if (nbh_id != -1) {
                room.children.insert(nbh_id);
                rooms[nbh_id].parents.insert(i);
            }
        }
    }

    return rooms;
}

inline bool is_terminal(const Room& room) {
    return room.children.size() == 0;
}

inline bool is_isolated(const Room& room) {
    return room.parents.size() == 0;
}

int max_value(const std::vector<Room>& rooms) {
    int max_result_for_nonterminal_rooms = 0;

    int max_value_for_last_room = 0;
    for (const auto& room : rooms) {
        if (is_isolated(room)) continue;
        if (is_terminal(room)) {
            max_value_for_last_room =
                    std::max(room.value, max_value_for_last_room);
        } else {
            max_result_for_nonterminal_rooms += room.value;
        }
    }

    return max_result_for_nonterminal_rooms + max_value_for_last_room;
}

class BacktrackTree {
  public:
    explicit BacktrackTree(std::vector<Room>&& rooms)
        : rooms{rooms}
        , best_value{std::vector<int>(rooms.size(), 0)} {
        for (auto& room : rooms) {
            if (is_terminal(room) && !is_isolated(room)) {
                leaves.insert(room.id);
                best_value[room.id] = room.value;
            }
        }
    }

    int get_best_value();

  private:
    std::vector<Room> rooms;
    std::vector<int> best_value;
    std::set<int> leaves;

    void backtrack();
};

void BacktrackTree::backtrack() {
    std::set<int> new_leaves;

    for (int leaf_id : leaves) {
        int best_leaf_value = best_value[leaf_id];
        for (int parent_id : rooms[leaf_id].parents) {
            int tentative_parent_best_value =
                    rooms[parent_id].value + best_leaf_value;

            if (tentative_parent_best_value > best_value[parent_id])
                best_value[parent_id] = tentative_parent_best_value;

            new_leaves.insert(parent_id);
        }
    }

    leaves = new_leaves;
}

int BacktrackTree::get_best_value() {
    if (leaves.size() == 0) {
        return rooms[0].value;
    }

    while (leaves.size() > 0) {
        backtrack();
    }

    return best_value[0];
}

int main(int argc, char* argv[]) {

    std::vector<Room> rooms = input(std::cin);

    // std::copy(rooms.begin(), rooms.end(),
    //           std::ostream_iterator<Room>{std::cerr, "\n"});

    BacktrackTree btt{std::move(rooms)};
    std::cout << btt.get_best_value() << std::endl;

    return 0;
}
