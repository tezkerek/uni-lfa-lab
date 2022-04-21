#include <fstream>
#include <iostream>

#include "dfa.hpp"
#include "nfa.hpp"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        return 1;
    }

    std::ifstream ifs(argv[1]);

    NFA nfa;
    ifs >> nfa;

    std::cout << nfa << '\n';

    DFA dfa(nfa.to_dfa());

    std::cout << dfa << '\n';

    return 0;
}
