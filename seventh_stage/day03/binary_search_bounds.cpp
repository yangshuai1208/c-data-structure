#include <stdio.h>
#include <assert.h>

int find_first(const int arr[], int size, int target)
{
    int left=0;
    int right=size-1;
    int result=-1;
    while(left<=right)
    {
        int mid=left+(right-left)/2;
        if(arr[mid]==target)
        {
            result=mid;
            right=mid-1;
        }
        else if(arr[mid]<target)
        {
            left=mid+1;
        }
        else
        {
            right=mid-1;
        }
    }
    return result;
}

int find_last(const int arr[], int size, int target)
{
    int left=0;
    int right=size-1;
    int result=-1;
    while(left<=right)
    {
        int mid=left+(right-left)/2;
        if(arr[mid]==target)
        {
            result=mid;
            left=mid+1;
        }
        else if(arr[mid]<target)
        {
            left=mid+1;
        }
        else
        {
            right=mid-1;
        }
    }
    return result;
}
int main(void)
{
    int nums[] = {1, 2, 2, 2, 4, 5, 7};
    int size = sizeof(nums) / sizeof(nums[0]);

    assert(find_first(nums, size, 2) == 1);
    assert(find_last(nums, size, 2) == 3);

    /* 边界 */
    assert(find_first(nums, size, 1) == 0);
    assert(find_last(nums, size, 7) == 6);

    /* 只有一个 */
    assert(find_first(nums, size, 4) == 4);
    assert(find_last(nums, size, 4) == 4);

    /* 不存在 */
    assert(find_first(nums, size, 3) == -1);
    assert(find_last(nums, size, 100) == -1);

    printf("All tests passed!\n");

    return 0;
}