#include <stddef.h>
#include <stdio.h>
#include <assert.h>

int first_index(const int *arr, int n, int target)
{
    if (arr == NULL || n <= 0)
    {
        return -1;
    }

    int left = 0;
    int right = n - 1;
    int result = -1;

    while (left <= right)
    {
        int middle = left + (right - left) / 2;

        if (arr[middle] < target)
        {
            /* 中间值太小，目标只能在右边 */
            left = middle + 1;
        }
        else if (arr[middle] > target)
        {
            /* 中间值太大，目标只能在左边 */
            right = middle - 1;
        }
        else
        {
            /* 找到了，但左边可能还有相同值 */
            result = middle;
            right = middle - 1;
        }
    }

    return result;
}
int main(void)
{
    int same[] = {2, 2, 2, 2};
    int single[] = {5};
    int arr[] = {1, 2, 2, 2, 4, 7};

    /* 空指针、无效长度 */
    assert(first_index(NULL, 4, 2) == -1);
    assert(first_index(arr, 0, 2) == -1);
    assert(first_index(arr, -1, 2) == -1);

    /* 全部相同 */
    assert(first_index(same, 4, 2) == 0);
    assert(first_index(same, 4, 3) == -1);

    /* 单元素 */
    assert(first_index(single, 1, 5) == 0);
    assert(first_index(single, 1, 2) == -1);

    /* 重复值、首尾、不存在 */
    assert(first_index(arr, 6, 2) == 1);
    assert(first_index(arr, 6, 1) == 0);
    assert(first_index(arr, 6, 7) == 5);
    assert(first_index(arr, 6, 3) == -1);
    assert(first_index(arr, 6, 0) == -1);
    assert(first_index(arr, 6, 8) == -1);

    puts("first_index tests passed");
    return 0;
}