#include <assert.h>
#include <stddef.h>
#include <stdio.h>

static size_t remove_duplicates(
    int *array,
    size_t length)
{
    if(!array||length==0)
    {
        return 0;
    }

    /* TODO：检查空指针和空数组 */
    int slow=0;
    
    /* TODO：定义慢指针 */

    for (size_t fast = 1U; fast < length; ++fast)
    {   
        if(array[slow]!=array[fast])
        {
            slow++;
            array[slow]=array[fast];
        }
     
    }
    /* TODO：使用快指针扫描数组 */
    return slow+1;

    /* TODO：返回新的有效长度 */
}

static void test_normal(void)
{
    int numbers[] = {1, 1, 2, 2, 2, 3, 5, 5};

    size_t new_length = remove_duplicates(
        numbers,
        sizeof(numbers) / sizeof(numbers[0])
    );

    assert(new_length == 4U);
    assert(numbers[0] == 1);
    assert(numbers[1] == 2);
    assert(numbers[2] == 3);
    assert(numbers[3] == 5);
}

static void test_all_same(void)
{
    int numbers[] = {7, 7, 7, 7};

    size_t new_length =
        remove_duplicates(numbers, 4U);

    assert(new_length == 1U);
    assert(numbers[0] == 7);
}

static void test_no_duplicates(void)
{
    int numbers[] = {1, 2, 3, 4};

    size_t new_length =
        remove_duplicates(numbers, 4U);

    assert(new_length == 4U);
    assert(numbers[0] == 1);
    assert(numbers[3] == 4);
}

int main(void)
{
    test_normal();
    test_all_same();
    test_no_duplicates();

    assert(remove_duplicates(NULL, 0U) == 0U);

    printf("remove duplicates tests passed\n");
    return 0;
}