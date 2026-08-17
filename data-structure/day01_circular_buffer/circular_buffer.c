#include "circular_buffer.h"
void rb_init(RingBuffer *rb)
{
    if(rb==NULL)
    {
        return;
    }
    rb->head=0U;
    rb->tail=0U;
    rb->count=0U;
}
bool rb_is_empty(const RingBuffer *rb)
{
   return rb==NULL||rb->count==0U;
}
bool rb_is_full(const RingBuffer *rb)
{
    return rb!=NULL&&rb->count==RB_CAPACITY;
}
size_t rb_size(const RingBuffer *rb)
{
    return rb==NULL?0U:rb->count;
}
bool rb_push(RingBuffer *rb,uint8_t data)
{
   if(rb==NULL||rb_is_full(rb))
   {
    return false;
   }
   rb->data[rb->tail]=data;
   rb->tail=(rb->tail+1U)%RB_CAPACITY;
   ++rb->count;

   return true;
}
bool rb_peek(const RingBuffer *rb,uint8_t *data)
{
    if(rb==NULL||data==NULL||rb_is_empty(rb))
    return false;

        *data=rb->data[rb->head];
        return true;

}
bool rb_pop(RingBuffer *rb,uint8_t *data)
{
    if(rb==NULL||data==NULL||rb_is_empty(rb))
    return false;

    *data=rb->data[rb->head];
    rb->head=(rb->head+1U)%RB_CAPACITY;
    --rb->count;

    return true;
}
