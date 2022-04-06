#pragma once

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class Automaton {
public:
    using StateType = int;
    using SymbolType = std::optional<char>;

    struct QueuedState {
        StateType state;
        std::optional<StateType> origin;
    };

protected:
    StateType initial_state;
    std::unordered_set<StateType> final_states;

public:
    void set_initial_state(StateType state);
    virtual void add_state(StateType state) = 0;
    void add_final_state(StateType state);
    virtual std::optional<std::vector<StateType>>
    verify_word(const std::string &word) = 0;
};

template <typename StateT>
std::vector<StateT>
build_state_chain(const std::vector<Automaton::QueuedState> &state_queue) {
    std::vector<StateT> chain;

    // Start from the last queued state
    auto current_state = state_queue.back();
    while (current_state.origin.has_value()) {
        /* printf("%d -> %d\n", current_state.state, current_state.origin); */
        chain.push_back(current_state.state);
        current_state = state_queue[current_state.origin.value()];
    }
    // Also add final state
    chain.push_back(current_state.state);

    return chain;
}
