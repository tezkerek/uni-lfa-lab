cmake_minimum_required(VERSION 3.14)

project(Automata)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(lnfa_verify
    src/lnfa_verify.cpp
    src/lnfa.cpp
    src/automaton.cpp
)

add_executable(nfa2dfa
    src/nfa2dfa.cpp
    src/nfa.cpp
    src/dfa.cpp
    src/automaton.cpp
)

add_executable(minimize_dfa
    src/minimize_dfa.cpp
    src/dfa.cpp
    src/automaton.cpp
)
