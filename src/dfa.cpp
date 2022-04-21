#include <cstddef>
#include <fstream>
#include <istream>
#include <ostream>
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
