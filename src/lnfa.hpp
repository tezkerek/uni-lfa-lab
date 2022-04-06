#pragma once

#include "automaton.hpp"
#include <istream>

class LNFA : public Automaton {
public:
    using SymbolType = std::optional<char>;

private:
    using SymbolMap = std::unordered_map<SymbolType, std::vector<StateType>>;
    using TransitionMap = std::unordered_map<StateType, SymbolMap>;
    TransitionMap transition_map;

    bool are_lambda_closures_built = false;
    std::unordered_map<StateType, std::unordered_set<StateType>>
        lambda_closures;

    class Verifier {
    private:
        std::shared_ptr<const LNFA> lnfa;
        std::vector<QueuedState> state_queue;
        std::size_t queue_index;

    public:
        Verifier(const LNFA &lnfa);

        // Advance from current states via symbol.
        // Returns true if any of the reached states is final.
        bool advance(SymbolType symbol, bool stop_at_final_state = false);

        std::vector<StateType> build_chain() const;
    };

    Verifier create_verifier() const;

    void build_lambda_closures();

public:
    void add_state(StateType state) override;
    void add_transition(StateType src_state, StateType dest_state,
                        SymbolType symbol);
    std::optional<std::vector<StateType>>
    verify_word(const std::string &word) override;
};

/** Read an NFA from a istream. */
std::istream &operator>>(std::istream &is, LNFA &lnfa);
