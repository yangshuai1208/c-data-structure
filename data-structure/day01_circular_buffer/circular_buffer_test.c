#include <assert.h>
#include <stdio.h>
#include "circular_buffer.h"

static void test_basic_operations(void)
{
    RingBuffer rb;
    uint8_t value = 0U;

    rb_init(&rb);

    assert(rb_is_empty(&rb));
    assert(rb_size(&rb) == 0U);
    assert(!rb_pop(&rb, &value));

    assert(rb_push(&rb, 10U));
    assert(rb_push(&rb, 20U));
    assert(rb_size(&rb) == 2U);

    assert(rb_peek(&rb, &value));
    assert(value == 10U);

    /* peek不会删除数据 */
    assert(rb_size(&rb) == 2U);

    assert(rb_pop(&rb, &value));
    assert(value == 10U);

    assert(rb_pop(&rb, &value));
    assert(value == 20U);

    assert(rb_is_empty(&rb));
}

static void test_full_and_wraparound(void)
{
    RingBuffer rb;
    uint8_t value = 0U;

    rb_init(&rb);

    /* 压入1～8，填满缓冲区 */
    for (uint8_t input = 1U; input <= 8U; ++input)
    {
        assert(rb_push(&rb, input));
    }

    assert(rb_is_full(&rb));
    assert(rb_size(&rb) == RB_CAPACITY);

    /* 缓冲区已满，第9次写入必须失败 */
    assert(!rb_push(&rb, 9U));

    /* 弹出1～4，腾出4个位置 */
    for (uint8_t expected = 1U; expected <= 4U; ++expected)
    {
        assert(rb_pop(&rb, &value));
        assert(value == expected);
    }

    /* 写入9～12，tail将在数组中回绕 */
    for (uint8_t input = 9U; input <= 12U; ++input)
    {
        assert(rb_push(&rb, input));
    }

    assert(rb_is_full(&rb));

    /* 剩余数据必须按照5～12的顺序弹出 */
    for (uint8_t expected = 5U; expected <= 12U; ++expected)
    {
        assert(rb_pop(&rb, &value));
        assert(value == expected);
    }

    assert(rb_is_empty(&rb));
    assert(rb_size(&rb) == 0U);
    assert(!rb_pop(&rb, &value));
}

int main(void)
{
    test_basic_operations();
    test_full_and_wraparound();

    puts("all circular buffer tests passed");
    return 0;
}