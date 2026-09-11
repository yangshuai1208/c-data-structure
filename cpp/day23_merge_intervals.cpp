#include <algorithm>
#include <array>
#include <cassert>
#include <iostream>
#include <vector>

/*
 * interval[0]：起点
 * interval[1]：终点
 */
using Interval = std::array<int, 2>;

std::vector<Interval> merge_intervals(
    std::vector<Interval> intervals
)
{
    if (intervals.empty())
    {
        return {};
    }

    /*
     * TODO 1：
     * 使用std::sort，按区间起点升序排序。
     * 起点相同时，按终点升序排序。
     *
     * 比较参数可以写成：
     * const Interval &a, const Interval &b
     */
    std::sort(intervals.begin(),intervals.end(),[](const Interval &a,const Interval &b)
    {

        return a[0]<b[0]||(a[0]==b[0]&&a[1]<b[1]);
    });
    




    std::vector<Interval> result;
    result.push_back(intervals[0]);

    for (std::size_t i = 1U; i < intervals.size(); ++i)
    {
        /*
         * TODO 2：
         * 当前区间为intervals[i]。
         * 最后一个已合并区间为result.back()。
         *
         * 如果当前起点 > 最后一个区间的终点：
         *     不重叠，将当前区间加入result。
         *
         * 否则：
         *     重叠，将最后一个区间的终点
         *     更新为两个终点的较大值。
         */
        if(intervals[i][0]>result.back()[1])
        {
            result.push_back(intervals[i]);
        }
        else 
        {
            result.back()[1]=std::max(result.back()[1],intervals[i][1]);
        }

    }

    return result;
}

int main()
{
    assert(merge_intervals({}).empty());

    assert((
        merge_intervals({{1, 3}})
        == std::vector<Interval>{{1, 3}}
    ));

    /* 输入无序，存在重叠。 */
    assert((
        merge_intervals({{8, 10}, {1, 3}, {2, 6}, {15, 18}})
        == std::vector<Interval>{{1, 6}, {8, 10}, {15, 18}}
    ));

    /* 端点相同，需要合并。 */
    assert((
        merge_intervals({{1, 2}, {2, 3}})
        == std::vector<Interval>{{1, 3}}
    ));

    /* 包含关系不能让终点缩小。 */
    assert((
        merge_intervals({{1, 10}, {2, 3}, {4, 8}})
        == std::vector<Interval>{{1, 10}}
    ));

    /* 相邻但不重叠。 */
    assert((
        merge_intervals({{3, 4}, {1, 2}})
        == std::vector<Interval>{{1, 2}, {3, 4}}
    ));

    /* 负数、重复区间和单点区间。 */
    assert((
        merge_intervals({{-2, 2}, {-5, -2}, {-5, -2}, {4, 4}})
        == std::vector<Interval>{{-5, 2}, {4, 4}}
    ));

    std::cout << "Day23 interval merge tests passed.\n";
    return 0;
}