#include <iterator>
#include <ostream>
#include <unordered_set>
#include <vector>

template <typename T>
std::ostream &operator<<(std::ostream &os, const std::vector<T> &vec) {
    if (vec.empty()) {
        os << "{}";
        return os;
    }

    os << "{";
    for (std::size_t i = 0; i < vec.size() - 1; i++) {
        os << vec[i] << ", ";
    }
    os << vec.back() << "}";

    return os;
}

template <typename T>
std::ostream &operator<<(std::ostream &os, const std::unordered_set<T> &set) {
    if (set.empty()) {
        os << "{}";
        return os;
    }

    auto iter = set.begin();
    os << '{' << *iter;
    std::advance(iter, 1);
    while (iter != set.end()) {
        os << ", " << *iter;
        std::advance(iter, 1);
    }
    os << '}';

    return os;
}
