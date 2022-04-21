#include <algorithm>
#include <bits/ranges_algo.h>
#include <iostream>
#include <numeric>
#include <queue>
#include <ranges>
#include <unordered_set>

#include "dfa.hpp"
#include "nfa.hpp"
#include "utils.hpp"

template <> struct std::hash<std::vector<int>> {
public:
    std::size_t operator()(const std::vector<int> &vec) const {
        std::size_t seed = vec.size();
        for (auto &i : vec) {
            seed ^= i + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
};

void NFA::add_state(StateType state) { transition_map[state] = SymbolMap(); }

void NFA::add_transition(StateType src_state, StateType dest_state,
                         SymbolType symbol) {
    transition_map[src_state][symbol].push_back(dest_state);
}

std::optional<std::vector<NFA::StateType>>
NFA::verify_word(const std::string &word) {
    std::vector<QueuedState> current_states;
    current_states.push_back({initial_state, {}});
    std::size_t queue_index = 0;

    for (std::string::size_type i = 0; i < word.size(); i++) {
        // Stop processing for the last symbol in word
        bool stop_at_final_state = i == word.size() - 1;
        auto symbol = word.at(i);
        // Advance from each current state using the symbol
        auto current_states_count = current_states.size();
        for (; queue_index < current_states_count; queue_index++) {
            auto queued_state = current_states[queue_index];
            try {
                auto next_states =
                    transition_map.at(queued_state.state).at(symbol);
                for (auto state : next_states) {
                    current_states.push_back({state, queue_index});
                    if (stop_at_final_state && final_states.contains(state)) {
                        return build_state_chain<StateType>(current_states);
                    }
                }
            } catch (std::out_of_range &exception) {
            }
        }
    }

    // Reached states are in current_states[queue_index:]
    // Return true if any of the reached states are final
    for (; queue_index < current_states.size(); queue_index++) {
        auto reached_state = current_states[queue_index];
        if (final_states.contains(reached_state.state)) {
            return build_state_chain<StateType>(current_states);
        }
    }

    return {};
}

DFA NFA::to_dfa() const {
    DFA dfa;

    dfa.initial_state = initial_state;

    StateType available_state_name =
        1 + std::accumulate(transition_map.begin(), transition_map.end(),
                            initial_state, [](StateType max_state, auto state) {
                                return std::max(max_state, state.first);
                            });

    std::unordered_map<StateType, std::vector<StateType>> combination_of;
    std::unordered_map<std::vector<StateType>, StateType>
        composing_states_to_state_map;

    std::queue<StateType> state_queue;
    state_queue.push(initial_state);

    auto queue_new_combined_state =
        [&](const std::vector<StateType> &composing_states) {
            StateType new_state;
            if (composing_states.size() == 1) {
                // Destination state is probably one of the NFA's states.
                // Reuse the name.
                new_state = composing_states.front();
            } else {
                new_state = available_state_name++;

                std::cerr << "New state " << new_state << " = "
                          << composing_states << '\n';
            }

            combination_of[new_state] = composing_states;
            composing_states_to_state_map[composing_states] = new_state;

            dfa.add_state(new_state);

            if (std::ranges::any_of(composing_states, [&](StateType state) {
                    return final_states.contains(state);
                })) {
                // New state contains a final state from the NFA
                dfa.add_final_state(new_state);
            }

            state_queue.push(new_state);

            return new_state;
        };

    auto add_dfa_transition = [&](StateType src_state,
                                  const std::vector<StateType> &dest_states,
                                  SymbolType symbol) {
        // Check if combined state exists
        auto existing_state = composing_states_to_state_map.find(dest_states);
        if (existing_state == composing_states_to_state_map.end()) {
            // New combined state
            auto new_state = queue_new_combined_state(dest_states);

            // Add it to the transition map
            dfa.add_transition(src_state, new_state, symbol);
        } else {
            // Existing combined state
            dfa.add_transition(src_state, existing_state->second, symbol);
        }
    };

    auto compute_symbol_map_of_sets =
        [&](const std::vector<StateType> &composing_states) {
            // Compute union of reached states from every composing state, for
            // every symbol.
            std::unordered_map<SymbolType, std::unordered_set<StateType>>
                symbol_to_states_map;
            for (const auto composing_state : composing_states) {
                for (const auto &[symbol, reached_states] :
                     transition_map.at(composing_state)) {
                    symbol_to_states_map[symbol].insert(reached_states.begin(),
                                                        reached_states.end());
                }
            }

            return symbol_to_states_map;
        };

    while (!state_queue.empty()) {
        auto queued_state = state_queue.front();
        state_queue.pop();

        // Check if queued state is simple or composed of other states
        auto composing_states_pair = combination_of.find(queued_state);
        if (composing_states_pair != combination_of.end()) {
            // Queued state is composed of multiple other states.
            auto symbol_to_states_map =
                compute_symbol_map_of_sets(composing_states_pair->second);

            // Add a transition for every symbol in the map.
            for (const auto &[symbol, reached_states_set] :
                 symbol_to_states_map) {
                std::vector<StateType> reached_states_vec(
                    reached_states_set.begin(), reached_states_set.end());
                std::ranges::sort(reached_states_vec);

                add_dfa_transition(queued_state, reached_states_vec, symbol);
            }
        } else {
            // Single state
            dfa.add_state(queued_state);
            const auto &symbol_map = transition_map.at(queued_state);

            for (auto [symbol, reached_states] : symbol_map) {
                std::ranges::sort(reached_states);

                add_dfa_transition(queued_state, reached_states, symbol);
            }
        }
    }

    return dfa;
}

std::istream &operator>>(std::istream &is, NFA &nfa) {
    std::size_t state_count;
    is >> state_count;
    for (std::size_t i = 0; i < state_count; i++) {
        NFA::StateType state;
        is >> state;
        nfa.add_state(state);
    }

    std::size_t transition_count;
    is >> transition_count;
    for (std::size_t i = 0; i < transition_count; i++) {
        NFA::StateType src_state, dest_state;
        char symbol;
        is >> src_state >> dest_state >> symbol;
        nfa.add_transition(src_state, dest_state, symbol);
    }

    NFA::StateType initial_state;
    is >> initial_state;
    nfa.set_initial_state(initial_state);

    std::size_t final_state_count;
    is >> final_state_count;
    for (std::size_t i = 0; i < final_state_count; i++) {
        NFA::StateType final_state;
        is >> final_state;
        nfa.add_final_state(final_state);
    }

    return is;
}

std::ostream &operator<<(std::ostream &os, const NFA &nfa) {
    os << "NFA: s = " << nfa.initial_state << ", F = " << nfa.final_states
       << '\n';

    for (const auto &[src_state, symbol_map] : nfa.transition_map) {
        for (const auto [symbol, dest_states] : symbol_map) {
            os << src_state << " --" << symbol << "--> " << dest_states << '\n';
        }
    }

    return os;
}
