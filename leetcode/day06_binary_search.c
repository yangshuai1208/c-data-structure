#include <assert.h>
#include <stddef.h>
#include <stdio.h>

static int binary_search(
    const int *array,
    size_t length,
    int target)
{
    /* TODO：检查空指针和空数组 */
    if(!array||length==0)
    {
        return -1;
    }
    /* TODO：定义左右边界 */
    size_t left=0;
    size_t right=length;

    /* TODO：循环缩小查找范围 */
    while(left<right)
    {
        size_t mid=left+(right-left)/2;
        if(array[mid]==target)
        {
            return (int)mid;
        }
        else if(array[mid]<target)
        {
            left=mid+1;
        }
        else
        {
            right=mid;
        }
    }
    /* TODO：未找到返回-1 */
    return -1;
}

int main(void)
{
    const int numbers[] = {1, 3, 5, 7, 9, 11};
    const size_t length =
        sizeof(numbers) / sizeof(numbers[0]);

    assert(binary_search(numbers, length, 1) == 0);
    assert(binary_search(numbers, length, 7) == 3);
    assert(binary_search(numbers, length, 11) == 5);

    assert(binary_search(numbers, length, 6) == -1);
    assert(binary_search(numbers, length, -10) == -1);
    assert(binary_search(numbers, length, 20) == -1);

    assert(binary_search(NULL, 0U, 5) == -1);
    assert(binary_search(numbers, 0U, 5) == -1);

    printf("binary search tests passed\n");
    return 0;
}