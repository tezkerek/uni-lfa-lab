#include <algorithm>
#include <cstddef>
#include <fstream>
#include <istream>
#include <iterator>
#include <numeric>
#include <ostream>
#include <queue>
#include <set>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

#include "dfa.hpp"
#include "utils.hpp"

void DFA::add_state(StateType state) { transition_map[state] = SymbolMap(); }

void DFA::add_transition(StateType src_state, StateType dest_state,
                         SymbolType symbol) {
    transition_map[src_state][symbol] = dest_state;
}

std::optional<std::vector<DFA::StateType>>
DFA::verify_word(const std::string &word) {
    std::vector<StateType> chain;

    auto current_state = initial_state;
    chain.push_back(current_state);

    for (auto symbol : word) {
        try {
            current_state = transition_map.at(current_state).at(symbol);
            chain.push_back(current_state);
        } catch (std::out_of_range &exception) {
            return {};
        }
    }

    if (final_states.contains(current_state)) {
        return chain;
    }

    return {};
}

std::unordered_set<DFA::StateType> DFA::get_unreachable_states() const {
    std::unordered_set<StateType> reachable_states{initial_state};

    std::queue<StateType> state_queue;
    state_queue.push(initial_state);

    while (!state_queue.empty()) {
        auto state = state_queue.front();
        state_queue.pop();

        for (auto [symbol, dest_state] : transition_map.at(state)) {
            auto was_inserted = reachable_states.insert(dest_state).second;
            if (was_inserted) {
                state_queue.push(dest_state);
            }
        }
    }

    std::unordered_set<StateType> unreachable_states;
    for (auto state_pair : transition_map) {
        if (!reachable_states.contains(state_pair.first)) {
            unreachable_states.insert(state_pair.first);
        }
    }

    return unreachable_states;
}

std::vector<DFA::SymbolType> DFA::get_alphabet() const {
    std::unordered_set<SymbolType> symbols;

    for (auto src_pair : transition_map) {
        for (auto transition_pair : src_pair.second) {
            symbols.insert(transition_pair.first);
        }
    }

    std::vector<SymbolType> alphabet(symbols.begin(), symbols.end());
    return alphabet;
}

DFA DFA::minimize() const {
    const auto unreachable_states = get_unreachable_states();
    const auto alphabet = get_alphabet();

    using Set = std::set<StateType>;
    using Partitions = std::set<Set>;
    Partitions partitions;
    Partitions w;

    // First, partition into final states and non final states.
    Set final_states_set;
    Set non_final_states;
    for (auto state_pair : transition_map) {
        if (unreachable_states.contains(state_pair.first)) {
            continue;
        }

        if (final_states.contains(state_pair.first)) {
            final_states_set.insert(state_pair.first);
        } else {
            non_final_states.insert(state_pair.first);
        }
    }
    partitions.insert(final_states_set);
    partitions.insert(non_final_states);
    w.insert(final_states_set);
    w.insert(non_final_states);

    // Hopcroft's algorithm
    while (!w.empty()) {
        const Set &a = *w.begin();

        for (auto symbol : alphabet) {
            Set x;
            for (const auto &[src_state, symbol_map] : transition_map) {
                if (unreachable_states.contains(src_state)) {
                    continue;
                }

                auto dest_iter = symbol_map.find(symbol);
                if (dest_iter != symbol_map.end() &&
                    a.contains(dest_iter->second)) {
                    x.insert(src_state);
                }
            }

            for (auto y_iter = partitions.begin();
                 y_iter != partitions.end();) {
                const auto &y = *y_iter;

                Set x_intersect_y;
                std::ranges::set_intersection(
                    x, y, std::inserter(x_intersect_y, x_intersect_y.begin()));

                Set y_minus_x;
                std::ranges::set_difference(
                    y, x, std::inserter(y_minus_x, y_minus_x.begin()));

                if (!x_intersect_y.empty() && !y_minus_x.empty()) {
                    // Replace y with the intersection and the difference
                    partitions.insert(x_intersect_y);
                    partitions.insert(y_minus_x);

                    // If y is in w, replace it there as well
                    // Otherwise insert the smaller set of the two
                    auto y_in_w_iter = w.find(y);
                    if (y_in_w_iter != w.end()) {
                        w.erase(y_in_w_iter);
                        w.insert(x_intersect_y);
                        w.insert(y_minus_x);
                    } else {
                        if (x_intersect_y.size() <= y_minus_x.size()) {
                            w.insert(x_intersect_y);
                        } else {
                            w.insert(y_minus_x);
                        }
                    }

                    y_iter = partitions.erase(y_iter);
                } else {
                    std::advance(y_iter, 1);
                }
            }
        }

        w.erase(w.begin());
    }

    // Hopcroft finished, build minimized DFA from the partitions.
    DFA minimized;

    std::vector<Set> partitions_vec(partitions.begin(), partitions.end());
    std::unordered_map<StateType, StateType> original_state_to_partition;

    // Add new states to the minimized DFA
    for (std::size_t i = 0; i < partitions_vec.size(); i++) {
        const auto &p = partitions_vec[i];

        StateType new_state = i;
        minimized.add_state(new_state);

        for (auto state : p) {
            original_state_to_partition[state] = new_state;
        }

        if (p.contains(initial_state)) {
            minimized.initial_state = new_state;
        }

        if (final_states.contains(*p.begin())) {
            minimized.add_final_state(new_state);
        }
    }

    // Add transitions to the minimized DFA
    for (std::size_t src_partition_name = 0;
         src_partition_name < partitions_vec.size(); src_partition_name++) {
        const auto &p = partitions_vec[src_partition_name];
        for (auto state : p) {
            for (const auto &[symbol, original_dest_state] :
                 transition_map.at(state)) {
                auto dest_partition_name =
                    original_state_to_partition[original_dest_state];
                minimized.add_transition(src_partition_name,
                                         dest_partition_name, symbol);
            }
        }
    }

    return minimized;
}

std::istream &operator>>(std::istream &is, DFA &dfa) {
    std::size_t state_count;
    is >> state_count;
    for (std::size_t i = 0; i < state_count; i++) {
        DFA::StateType state;
        is >> state;
        dfa.add_state(state);
    }

    std::size_t transition_count;
    is >> transition_count;
    for (std::size_t i = 0; i < transition_count; i++) {
        DFA::StateType src_state, dest_state;
        char symbol;
        is >> src_state >> dest_state >> symbol;
        dfa.add_transition(src_state, dest_state, symbol);
    }

    DFA::StateType initial_state;
    is >> initial_state;
    dfa.set_initial_state(initial_state);

    std::size_t final_state_count;
    is >> final_state_count;
    for (std::size_t i = 0; i < final_state_count; i++) {
        DFA::StateType final_state;
        is >> final_state;
        dfa.add_final_state(final_state);
    }

    return is;
}

std::ostream &operator<<(std::ostream &os, const DFA &dfa) {
    os << "DFA: s = " << dfa.initial_state << ", F = " << dfa.final_states
       << '\n';

    for (const auto &[src_state, symbol_map] : dfa.transition_map) {
        for (const auto [symbol, dest_state] : symbol_map) {
            os << src_state << " --" << symbol << "--> " << dest_state << '\n';
        }
    }

    return os;
}
