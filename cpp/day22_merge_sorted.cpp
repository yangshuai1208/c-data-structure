#include <cassert>
#include <cstddef>
#include <iostream>
#include <vector>

std::vector<int> merge_sorted(
    const std::vector<int> &a,
    const std::vector<int> &b
)
{
    std::vector<int> result;

    std::size_t i = 0U;
    std::size_t j = 0U;

    /*
     * TODO 1：
     * 两组都有剩余元素时，
     * 比较 a[i] 和 b[j]，
     * 将较小者加入 result，
     * 并推进对应下标。
     */
    while(i<a.size()&&j<b.size())
    {
        if(a[i]<b[j])
        {
            result.push_back(a[i]);
            ++i;
        }
        else
        {
            result.push_back(b[j]);
            ++j;
        }
    }


    /*
     * TODO 2：
     * 将 a 中剩余元素加入 result。
     */
    while(i<a.size())
    {
        result.push_back(a[i]);
        ++i;
    }

    /*
     * TODO 3：
     * 将 b 中剩余元素加入 result。
     */
    while(j<b.size())
    {
        result.push_back(b[j]);
        ++j;
    }
    return result;
}

int main()
{
    assert(merge_sorted({}, {}).empty());

    assert((
        merge_sorted({}, {1, 2})
        == std::vector<int>{1, 2}
    ));

    assert((
        merge_sorted({1, 2}, {})
        == std::vector<int>{1, 2}
    ));

    assert((
        merge_sorted({1, 3, 3, 8}, {2, 3, 7})
        == std::vector<int>{1, 2, 3, 3, 3, 7, 8}
    ));

    assert((
        merge_sorted({-5, 0, 4}, {-3, 4, 9})
        == std::vector<int>{-5, -3, 0, 4, 4, 9}
    ));

    assert((
        merge_sorted({5, 6}, {1, 2})
        == std::vector<int>{1, 2, 5, 6}
    ));

    std::cout << "Day22 merge tests passed.\n";
    return 0;
}