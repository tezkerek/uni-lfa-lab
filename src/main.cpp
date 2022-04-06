#include "lnfa.hpp"
#include <fstream>

int main() {
    LNFA lnfa;

    std::ifstream ifs("lnfa_input.txt");
    ifs >> lnfa;

    // Start verifying words
    std::ofstream ofs("output.txt");
    std::size_t word_count;
    ifs >> word_count;

    for (std::size_t i = 0; i < word_count; i++) {
        std::string word;
        ifs >> word;

        auto result = lnfa.verify_word(word);
        ofs << word << ' ';
        if (result.has_value()) {
            auto &chain = result.value();
            ofs << "DA:";
            for (auto it = chain.rbegin(); it != chain.rend(); it++) {
                ofs << " -> " << *it;
            }
        } else {
            ofs << "NU";
        }
        ofs << '\n';
    }

    ifs.close();
    ofs.close();

    return 0;
}
