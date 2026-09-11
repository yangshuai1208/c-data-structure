#include "stats.h"

#include <cassert>
#include <iostream>

int main()
{
    const int values[] = {10, 20, 30};
    const std::size_t count = sizeof(values) / sizeof(values[0]);
    const int result = calculate_sum(values, count);

    std::cout << "sum=" << result << '\n';
    assert(result == 60);
    std::cout << "Day23 sum check passed.\n";
    return 0;
}
