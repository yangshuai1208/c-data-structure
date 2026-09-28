#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <assert.h>
#include <stdio.h>

#define RB_CAPACITY 8U

typedef struct
{
    uint8_t data[RB_CAPACITY];
    size_t head;
    size_t tail;
    uint32_t overflow_count;
} RingBuffer;

void rb_init(RingBuffer *rb)
{
    if(rb==NULL)
    {
        return;
    }
    rb->head=0;
    rb->tail=0;
    rb->overflow_count=0;
}

bool rb_push(RingBuffer *rb, uint8_t value)
{
    if(rb==NULL)
    {
        return false;
    }
    size_t next_head=(rb->head+1U)%RB_CAPACITY;
    
    if(next_head==rb->tail)
    {
        rb->overflow_count++;
        return false;
    }

    rb->data[rb->head]=value;
    rb->head=next_head;

    return true;
}

bool rb_pop(RingBuffer *rb, uint8_t *out)
{
    if(rb==NULL||out==NULL)
    {
        return false;
    }
    if(rb->head==rb->tail)
    {
        return false;
    }

    *out=rb->data[rb->tail];
    rb->tail=(rb->tail+1U)%RB_CAPACITY;

    return true;
}
int main(void)
{
    RingBuffer rb;
    uint8_t out = 0xEEU;

    rb_init(&rb);

    /* 1. 初始为空，读取失败且不修改out */
    assert(!rb_pop(&rb, &out));
    assert(out == 0xEEU);

    /* 2. 写入7个字节，有效容量为7 */
    for (uint8_t i = 0U; i < 7U; ++i)
    {
        assert(rb_push(&rb, i));
    }

    /* 3. 第8次写入失败，溢出计数加1 */
    assert(!rb_push(&rb, 7U));
    assert(rb.overflow_count == 1U);

    /* 4. 先读取3个字节 */
    for (uint8_t i = 0U; i < 3U; ++i)
    {
        assert(rb_pop(&rb, &out));
        assert(out == i);
    }

    /* 5. 再写入3个字节，验证索引回绕 */
    assert(rb_push(&rb, 7U));
    assert(rb_push(&rb, 8U));
    assert(rb_push(&rb, 9U));

    /* 6. 验证FIFO顺序 */
    for (uint8_t i = 3U; i <= 9U; ++i)
    {
        assert(rb_pop(&rb, &out));
        assert(out == i);
    }

    /* 7. 再次读空，不修改输出 */
    out = 0xEEU;

    assert(!rb_pop(&rb, &out));
    assert(out == 0xEEU);

    /* 8. NULL参数测试 */
    rb_init(NULL);

    assert(!rb_push(NULL, 1U));
    assert(!rb_pop(NULL, &out));
    assert(!rb_pop(&rb, NULL));

    printf("uart_ring_buffer passed\n");

    return 0;
}