#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define RB_CAPACITY 8U

typedef struct
{
    uint8_t data[RB_CAPACITY];
    size_t head;
    size_t tail;
    size_t count;
} RingBuffer;

void rb_init(RingBuffer *rb);
bool rb_push(RingBuffer *rb, uint8_t data);
bool rb_pop(RingBuffer *rb, uint8_t *data);
bool rb_peek(const RingBuffer *rb, uint8_t *data);
bool rb_is_empty(const RingBuffer *rb);
bool rb_is_full(const RingBuffer *rb);
size_t rb_size(const RingBuffer *rb);

#endif