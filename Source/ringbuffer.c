#include "ringbuffer.h"


#define min(a, b) (a)<(b)?(a):(b)


static void * mymemcpy(void*destaddr, void const*srcaddr, int len)
{
    char* dest = destaddr;
    char const* src = srcaddr;
    while (len-- > 0)
    {
        *dest++ = *src++;
    }
    return destaddr;
}


void rb_new(RingBuffer* rb , unsigned char * global_buff , unsigned int len)
{
    rb->rb_buff = (char *) global_buff ;
    rb->rb_capacity = len;
    rb->rb_head     = rb->rb_buff;
    rb->rb_tail     = rb->rb_buff;
};

void  rb_free(RingBuffer *rb)
{
    //free((char*)rb);
}

int rb_capacity(RingBuffer *rb)
{
    return rb->rb_capacity;
}

int rb_can_read(RingBuffer *rb)
{

    if (rb->rb_head == rb->rb_tail) return 0;
    if (rb->rb_head < rb->rb_tail) return rb->rb_tail - rb->rb_head;
    return rb_capacity(rb) - (rb->rb_head - rb->rb_tail);
}


int rb_can_write(RingBuffer *rb)
{
    //assert(rb != NULL);
    return rb_capacity(rb) - rb_can_read(rb);
}

int rb_read(RingBuffer *rb, void *data, int count)
{
    //assert(rb != NULL);
    //assert(data != NULL);
    if (rb->rb_head < rb->rb_tail)
    {
        int copy_sz = min(count, rb_can_read(rb));
        mymemcpy(data, rb->rb_head, copy_sz);
        rb->rb_head += copy_sz;
        return copy_sz;
    }
    else
    {
        if (count < rb_capacity(rb) - (rb->rb_head - rb->rb_buff))
        {
            int copy_sz = count;
            mymemcpy(data, rb->rb_head, copy_sz);
            rb->rb_head += copy_sz;
            return copy_sz;
        }
        else
        {
            int copy_sz = rb_capacity(rb) - (rb->rb_head - rb->rb_buff);
            mymemcpy(data, rb->rb_head, copy_sz);
            rb->rb_head = rb->rb_buff;
            copy_sz += rb_read(rb, (char*)data + copy_sz, count - copy_sz);
            return copy_sz;
        }
    }
}

int rb_write(RingBuffer *rb, const void *data, int count)
{
    if (count >= rb_can_write(rb))
        return -1;

    if (rb->rb_head <= rb->rb_tail)
    {
        int tail_avail_sz = rb_capacity(rb) - (rb->rb_tail - rb->rb_buff);
        if (count <= tail_avail_sz)
        {
            mymemcpy(rb->rb_tail, data, count);
            rb->rb_tail += count;
            if (rb->rb_tail == rb->rb_buff + rb_capacity(rb))
                rb->rb_tail = rb->rb_buff;
            return count;
        }
        else
        {
            mymemcpy(rb->rb_tail, data, tail_avail_sz);
            rb->rb_tail = rb->rb_buff;

            return tail_avail_sz + rb_write(rb, (char*)data + tail_avail_sz, count - tail_avail_sz);
        }
    }
    else
    {
        mymemcpy(rb->rb_tail, data, count);
        rb->rb_tail += count;
        return count;
    }
}

