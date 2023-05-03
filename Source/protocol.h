#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

extern char *cfg_server_host;

typedef struct
{
    unsigned char *p_o;          /**< Original pointer */
    unsigned char *volatile p_r; /**< Read pointer */
    unsigned char *volatile p_w; /**< Write pointer */
    volatile int fill_cnt;       /**< Number of filled slots */
    int size;                    /**< Buffer size */
} RINGBUF;

int RINGBUF_Init(RINGBUF *r, unsigned char *buf, int size);
int RINGBUF_Put(RINGBUF *r, unsigned char c);
int RINGBUF_Get(RINGBUF *r, unsigned char *c);

struct protocol_pipe
{
    RINGBUF ringbuf;

    void (*callback)(const char *id, const char *value);

    //get pack
    unsigned char *frame_buff;
    unsigned int frame_buff_max_len;
    int frame_idx;
    unsigned char frame_is_get_head;
};

void protocol_pipe_init(struct protocol_pipe *pipeline, unsigned char *buff, unsigned int max_len);

void protocol_pipe_get(struct protocol_pipe *pipeline , void (*callback)(const char *id, const char *value) );

void protocol_pipe_put(struct protocol_pipe *pipeline,unsigned char * buff ,unsigned int len);

unsigned char protocol_pipe_compare(const char * src , const char * aa ,int len);

#endif
