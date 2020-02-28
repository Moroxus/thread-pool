#include "ThreadPool.h"
#include <iostream>

int main() {
    std::vector<int> v1{1, 2, 3, 4};
    ThreadPool threadPool;

    for (const auto& item : v1) {
        std::cerr << item << ' ';
    }
    std::cerr << '\n';
    auto f1 = threadPool.push([](int& argument) { ++argument; }, std::ref(v1.front()));
    auto f2 = threadPool.push([](int& argument) { ++argument; }, std::ref(v1.back()));

    f1.get();
    f2.get();
    for (const auto& item : v1) {
        std::cerr << item << ' ';
    }
    std::cerr << '\n';
}
