#include <iostream>
#include "tests.hpp"


int main() {
    TestUnqPtr();
    TestShrdPtr();
    TestDynamicArray();

    std::cout << "Все тесты пройдены успешно!\n\n";

    RunBenchmarks();

    return 0;
}