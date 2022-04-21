#pragma once

#include "automaton.hpp"
#include "dfa.hpp"
#include <istream>

class DFA;

class NFA : public Automaton {
public:
    using SymbolType = char;

private:
    using SymbolMap = std::unordered_map<SymbolType, std::vector<StateType>>;
    using TransitionMap = std::unordered_map<StateType, SymbolMap>;
    TransitionMap transition_map;

public:
    void add_state(StateType state) override;
    virtual void add_transition(StateType src_state, StateType dest_state,
                                SymbolType symbol);
    std::optional<std::vector<StateType>>
    verify_word(const std::string &word) override;

    DFA to_dfa() const;

    friend std::ostream &operator<<(std::ostream &os, const NFA &nfa);
};

std::istream &operator>>(std::istream &is, NFA &nfa);
std::ostream &operator<<(std::ostream &os, const NFA &nfa);
