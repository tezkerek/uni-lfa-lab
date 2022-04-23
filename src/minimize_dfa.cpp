#include <fstream>
#include <iostream>

#include "dfa.hpp"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        return 1;
    }

    std::ifstream ifs(argv[1]);

    DFA dfa;
    ifs >> dfa;

    std::cout << "Initial " << dfa << '\n';

    std::cout << "Minimized "<< dfa.minimize() << '\n';

    return 0;
}
