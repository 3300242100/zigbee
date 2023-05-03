#ifndef RINGBUFFER_H
#define RINGBUFFER_H

typedef struct {
	int rb_capacity;
	char  *rb_head;
	char  *rb_tail;
	char * rb_buff;
} RingBuffer;


void     rb_new(RingBuffer* rb,unsigned char * global_buff , unsigned int len);
void     rb_free(RingBuffer *rb);

int     rb_capacity(RingBuffer *rb);
int     rb_can_read(RingBuffer *rb);
int     rb_can_write(RingBuffer *rb);

int     rb_read(RingBuffer *rb, void *data, int count);
int     rb_write(RingBuffer *rb, const void *data, int count);

#endif

