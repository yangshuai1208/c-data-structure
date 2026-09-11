#include "stats.h"

int calculate_sum(const int *values, std::size_t count)
{
    int sum = 0;

    for (std::size_t i = 0; i <count; ++i)
    {
        sum += values[i];
    }

    return sum;
}
