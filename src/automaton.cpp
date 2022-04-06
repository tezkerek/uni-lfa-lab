#include "automaton.hpp"
#include <memory>

void Automaton::set_initial_state(StateType state) { initial_state = state; }

void Automaton::add_final_state(StateType state) { final_states.insert(state); }
