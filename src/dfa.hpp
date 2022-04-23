#pragma once

#include <istream>
#include <ostream>

#include "automaton.hpp"
#include "nfa.hpp"

class DFA : public Automaton {
public:
    using SymbolType = char;

protected:
    using SymbolMap = std::unordered_map<SymbolType, StateType>;
    using TransitionMap = std::unordered_map<StateType, SymbolMap>;
    TransitionMap transition_map;

    [[nodiscard]] std::unordered_set<StateType> get_unreachable_states() const;

public:
    virtual void add_state(StateType state) override;
    virtual void add_transition(StateType src_state, StateType dest_state,
                                SymbolType symbol);

    std::optional<std::vector<StateType>>
    verify_word(const std::string &word) override;

    [[nodiscard]] std::vector<SymbolType> get_alphabet() const;

    [[nodiscard]] DFA minimize() const;

    friend DFA NFA::to_dfa() const;
    friend std::ostream &operator<<(std::ostream &os, const DFA &dfa);
};

std::istream &operator>>(std::istream &is, DFA &dfa);
std::ostream &operator<<(std::ostream &os, const DFA &dfa);
