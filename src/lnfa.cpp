#include "lnfa.hpp"
#include <queue>

void LNFA::add_state(StateType state) {
    transition_map[state] = SymbolMap();
    lambda_closures[state] = {state};
}

void LNFA::add_transition(StateType src_state, StateType dest_state,
                          SymbolType symbol) {
    transition_map[src_state][symbol].push_back(dest_state);
}

void LNFA::build_lambda_closures() {
    // Build lambda closure for every state
    std::queue<StateType> state_queue;
    for (auto src_pair : transition_map) {
        auto src_state = src_pair.first;
        auto &src_closure = lambda_closures[src_state];

        // BFS
        state_queue.push(src_state);
        while (!state_queue.empty()) {
            auto state = state_queue.front();
            state_queue.pop();

            // Insert states that we can reach via lambda
            for (auto reachable_state : transition_map[state][{}]) {
                auto result = src_closure.insert(reachable_state);
                if (result.second) {
                    // reachable_state was new to the closure, so queue it
                    state_queue.push(reachable_state);
                }
            }
        }
    }
}

LNFA::Verifier LNFA::create_verifier() const { return Verifier(*this); }

std::optional<std::vector<LNFA::StateType>>
LNFA::verify_word(const std::string &word) {
    if (!are_lambda_closures_built) {
        build_lambda_closures();
        are_lambda_closures_built = true;
    }

    auto verifier = create_verifier();

    auto symbol_iter = word.begin();
    while (symbol_iter != word.end() - 1) {
        verifier.advance(*symbol_iter);
        symbol_iter++;
    }
    if (symbol_iter != word.end()) {
        // For the last symbol in word, stop processing when as soon as a final
        // state is reached.
        auto is_word_valid = verifier.advance(*symbol_iter, true);

        if (is_word_valid) {
            return verifier.build_chain();
        }
    }

    return {};
}

LNFA::Verifier::Verifier(const LNFA &lnfa) {
    this->lnfa = std::make_shared<LNFA>(lnfa);

    // Start with the initial state's lambda closure
    for (auto state : lnfa.lambda_closures.at(lnfa.initial_state)) {
        decltype(QueuedState::origin) origin = {};
        if (state != lnfa.initial_state) {
            origin = 0;
        }
        state_queue.push_back({state, origin});
    }
    queue_index = 0;
}

bool LNFA::Verifier::advance(SymbolType symbol, bool stop_at_final_state) {
    bool has_reached_final_state = false;

    auto queued_state_count = state_queue.size();
    while (queue_index < queued_state_count) {
        // Process the new states in the queue
        auto current_state = state_queue[queue_index];
        const auto &symbol_map = lnfa->transition_map.at(current_state.state);

        try {
            // For each state reachable from the current state via the symbol...
            const auto &next_states = symbol_map.at(symbol);
            for (auto reachable_state : next_states) {
                const auto &lambda_closure =
                    lnfa->lambda_closures.at(reachable_state);

                // Queue the state's lambda closure (including the state itself)
                auto reachable_state_index = queue_index;
                for (auto lambda_state : lambda_closure) {
                    // The origin is the current state for the reachable state
                    // itself, and the reachable state for the other states in
                    // the closure.
                    auto origin = reachable_state_index;
                    if (lambda_state == reachable_state) {
                        // Store the index of the reachable_state, to use as
                        // origin when pushing lambda_states.
                        reachable_state_index = state_queue.size();
                        origin = queue_index;
                    }
                    state_queue.push_back({lambda_state, origin});

                    // Stop at the first reached final state if flag is set
                    has_reached_final_state |=
                        lnfa->final_states.contains(lambda_state);
                    if (stop_at_final_state && has_reached_final_state) {
                        return true;
                    }
                }
            }
        } catch (std::out_of_range &exception) {
            // Cannot advance from current_state via the symbol
            // Skip current_state
        }
        queue_index++;
    }

    return has_reached_final_state;
}

std::vector<LNFA::StateType> LNFA::Verifier::build_chain() const {
    return build_state_chain<LNFA::StateType>(state_queue);
}

std::istream &operator>>(std::istream &is, LNFA &lnfa) {
    std::size_t state_count;
    is >> state_count;
    for (std::size_t i = 0; i < state_count; i++) {
        int state;
        is >> state;
        lnfa.add_state(state);
    }

    std::size_t transition_count;
    is >> transition_count;
    for (std::size_t i = 0; i < transition_count; i++) {
        int src_state, dest_state;
        char symbol;
        is >> src_state >> dest_state >> symbol;

        if (symbol == '_') {
            // λ-transition
            lnfa.add_transition(src_state, dest_state, {});
        } else {
            lnfa.add_transition(src_state, dest_state, symbol);
        }
    }

    int initial_state;
    is >> initial_state;
    lnfa.set_initial_state(initial_state);

    std::size_t final_state_count;
    is >> final_state_count;
    for (std::size_t i = 0; i < final_state_count; i++) {
        int final_state;
        is >> final_state;
        lnfa.add_final_state(final_state);
    }

    return is;
}
