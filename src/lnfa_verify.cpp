#include <fstream>
#include <iostream>

#include "lnfa.hpp"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        return 1;
    }

    std::ifstream ifs(argv[1]);

    LNFA lnfa;
    ifs >> lnfa;

    // Start verifying words
    std::size_t word_count;
    ifs >> word_count;

    for (std::size_t i = 0; i < word_count; i++) {
        std::string word;
        ifs >> word;

        auto result = lnfa.verify_word(word);
        std::cout << word << ' ';
        if (result.has_value()) {
            auto &chain = result.value();
            std::cout << "DA:";
            for (auto it = chain.rbegin(); it != chain.rend(); it++) {
                std::cout << " -> " << *it;
            }
        } else {
            std::cout << "NU";
        }
        std::cout << '\n';
    }

    ifs.close();

    return 0;
}
