#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#define QUEUE_CAPACITY 5U

typedef struct
{
    int data[QUEUE_CAPACITY];
    size_t head;
    size_t tail;
    size_t count;
} RingQueue;

static void ring_queue_init(RingQueue *queue)
{
    if (queue == NULL)
    {
        return;
    }

    queue->head = 0U;
    queue->tail = 0U;
    queue->count = 0U;
}

static bool ring_queue_is_empty(const RingQueue *queue)
{
    if (queue == NULL)
    {
        return false;
    }

    return queue->count == 0U;
}

static bool ring_queue_is_full(const RingQueue *queue)
{
    if (queue == NULL)
    {
        return false;
    }

    return queue->count == QUEUE_CAPACITY;
}

static bool ring_queue_push(RingQueue *queue, int value)
{
    /*
     * ||具有短路特性：
     * queue为NULL时不会继续调用ring_queue_is_full()。
     */
    if (queue == NULL || ring_queue_is_full(queue))
    {
        return false;
    }

    queue->data[queue->tail] = value;

    queue->tail =
        (queue->tail + 1U) % QUEUE_CAPACITY;

    ++queue->count;

    return true;
}

static bool ring_queue_pop(RingQueue *queue, int *value)
{
    if (queue == NULL ||
        value == NULL ||
        ring_queue_is_empty(queue))
    {
        return false;
    }

    *value = queue->data[queue->head];

    queue->head =
        (queue->head + 1U) % QUEUE_CAPACITY;

    --queue->count;

    return true;
}

static bool ring_queue_peek(const RingQueue *queue, int *value)
{
    if (queue == NULL ||
        value == NULL ||
        ring_queue_is_empty(queue))
    {
        return false;
    }

    *value = queue->data[queue->head];

    return true;
}

static void test_null_and_empty(void)
{
    RingQueue queue;
    int value = -1;

    ring_queue_init(NULL);

    assert(!ring_queue_is_empty(NULL));
    assert(!ring_queue_is_full(NULL));
    assert(!ring_queue_push(NULL, 10));
    assert(!ring_queue_pop(NULL, &value));
    assert(!ring_queue_peek(NULL, &value));

    ring_queue_init(&queue);

    assert(ring_queue_is_empty(&queue));
    assert(!ring_queue_is_full(&queue));
    assert(queue.head == 0U);
    assert(queue.tail == 0U);
    assert(queue.count == 0U);

    assert(!ring_queue_pop(&queue, &value));
    assert(!ring_queue_peek(&queue, &value));
    assert(!ring_queue_pop(&queue, NULL));
    assert(!ring_queue_peek(&queue, NULL));

    assert(value == -1);
}

static void test_full_and_wraparound(void)
{
    RingQueue queue;
    int value = 0;

    const int initial_values[QUEUE_CAPACITY] =
    {
        10, 20, 30, 40, 50
    };

    ring_queue_init(&queue);

    for (size_t i = 0U; i < QUEUE_CAPACITY; ++i)
    {
        assert(ring_queue_push(&queue, initial_values[i]));
    }

    assert(ring_queue_is_full(&queue));
    assert(!ring_queue_is_empty(&queue));
    assert(queue.count == QUEUE_CAPACITY);

    /*
     * tail已经从数组末尾回绕到0。
     */
    assert(queue.head == 0U);
    assert(queue.tail == 0U);

    /*
     * 满队列入队必须失败，状态不能改变。
     */
    assert(!ring_queue_push(&queue, 60));
    assert(queue.head == 0U);
    assert(queue.tail == 0U);
    assert(queue.count == QUEUE_CAPACITY);

    /*
     * peek读取10，但不删除。
     */
    assert(ring_queue_peek(&queue, &value));
    assert(value == 10);
    assert(queue.count == QUEUE_CAPACITY);

    assert(ring_queue_pop(&queue, &value));
    assert(value == 10);

    assert(ring_queue_pop(&queue, &value));
    assert(value == 20);

    /*
     * 此时空出了下标0和1，继续入队会发生回绕写入。
     */
    assert(ring_queue_push(&queue, 60));
    assert(ring_queue_push(&queue, 70));

    /*
     * 队列再次装满：
     * head和tail都为2，但count为5。
     */
    assert(queue.head == queue.tail);
    assert(ring_queue_is_full(&queue));

    const int expected[QUEUE_CAPACITY] =
    {
        30, 40, 50, 60, 70
    };

    for (size_t i = 0U; i < QUEUE_CAPACITY; ++i)
    {
        assert(ring_queue_pop(&queue, &value));
        assert(value == expected[i]);
    }

    /*
     * 队列变空后head和tail仍相等，
     * 但此时count为0。
     */
    assert(queue.head == queue.tail);
    assert(queue.count == 0U);
    assert(ring_queue_is_empty(&queue));
    assert(!ring_queue_is_full(&queue));
}

int main(void)
{
    test_null_and_empty();
    test_full_and_wraparound();

    puts("Day20 ring queue: all tests passed.");

    return 0;
}