#include "protocol.h"

char *cfg_server_host = "{\"cfg_server_host\":\"www.xiaotongren.top/design/update/m1/pid/88888/uid/abcdef\"}";

// #ifndef __PROTOCOL_ringbuf_H__
// #define __PROTOCOL_ringbuf_H__

// typedef struct
// {
//     unsigned char *p_o;          /**< Original pointer */
//     unsigned char *volatile p_r; /**< Read pointer */
//     unsigned char *volatile p_w; /**< Write pointer */
//     volatile int fill_cnt;       /**< Number of filled slots */
//     int size;                    /**< Buffer size */
// } RINGBUF;

// int RINGBUF_Init(RINGBUF *r, unsigned char *buf, int size);
// int RINGBUF_Put(RINGBUF *r, unsigned char c);
// int RINGBUF_Get(RINGBUF *r, unsigned char *c);

// #endif

/**
* \brief init a RINGBUF object
* \param r pointer to a RINGBUF object
* \param buf pointer to a byte array
* \param size size of buf
* \return 0 if successfull, otherwise failed
*/
int RINGBUF_Init(RINGBUF *r, unsigned char *buf, int size)
{
    if (r == 0 || buf == 0 || size < 2)
        return -1;

    r->p_o = r->p_r = r->p_w = buf;
    r->fill_cnt = 0;
    r->size = size;

    return 0;
}
/**
* \brief put a character into ring buffer
* \param r pointer to a ringbuf object
* \param c character to be put
* \return 0 if successfull, otherwise failed
*/
int RINGBUF_Put(RINGBUF *r, unsigned char c)
{
    if (r->fill_cnt >= r->size)
        return -1; // ring buffer is full, this should be atomic operation

    r->fill_cnt++; // increase filled slots count, this should be atomic operation

    *r->p_w++ = c; // put character into buffer

    if (r->p_w >= r->p_o + r->size) // rollback if write pointer go pass
        r->p_w = r->p_o;            // the physical boundary

    return 0;
}
/**
* \brief get a character from ring buffer
* \param r pointer to a ringbuf object
* \param c read character
* \return 0 if successfull, otherwise failed
*/
int RINGBUF_Get(RINGBUF *r, unsigned char *c)
{
    if (r->fill_cnt <= 0)
        return -1; // ring buffer is empty, this should be atomic operation

    r->fill_cnt--; // decrease filled slots count

    *c = *r->p_r++; // get the character out

    if (r->p_r >= r->p_o + r->size) // rollback if write pointer go pass
        r->p_r = r->p_o;            // the physical boundary

    return 0;
}


void protocol_pipe_init(struct protocol_pipe *pipeline, unsigned char *buff, unsigned int max_len)
{
    unsigned int offset = max_len / 3;
    RINGBUF_Init(&(pipeline->ringbuf), buff + offset, max_len - offset);

    pipeline->frame_buff = buff;
    pipeline->frame_buff_max_len = offset;
    pipeline->frame_idx = 0;
    pipeline->frame_is_get_head = 0;


}


void protocol_pipe_parseFrame(struct protocol_pipe *pipeline)
{

    char *ptr = pipeline->frame_buff;

    char *id = pipeline->frame_buff + 1;

    char *value;

    for (; ptr < (char *)(pipeline->frame_buff + pipeline->frame_idx); ptr++)
    {
        if (*ptr == ',' || *ptr == '}')
        {
            value = id;
            while (value++ < ptr)
            {
                if (*value == ':')
                {
                    if(*(value+1) == '\"')
                    {
                        pipeline->callback(id, value+2);
                    }
                    else
                    {
                        pipeline->callback(id, value+1);
                    }
                    
                    break;
                }
            }
            id = ptr + 1;
        }
    }
}



void protocol_pipe_get(struct protocol_pipe *pipeline , void (*callback)(const char *id, const char *value) )
{
    unsigned char recv_byte;

    pipeline->callback = callback;

    while (RINGBUF_Get(&(pipeline->ringbuf), &recv_byte) == 0)
    {

        if (recv_byte == '{')
        {
            pipeline->frame_is_get_head = 1;
            pipeline->frame_idx = 0;
            pipeline->frame_buff[pipeline->frame_idx++] = recv_byte;

            continue;
        }
        else if (recv_byte == '}')
        {
            if (pipeline->frame_is_get_head)
            {
                pipeline->frame_buff[pipeline->frame_idx++] = recv_byte;

                //callback : parse success and get one frame

                protocol_pipe_parseFrame(pipeline);

                pipeline->frame_idx = 0;
                pipeline->frame_is_get_head = 0;
            }
            else
            {
                pipeline->frame_idx = 0;
            }
        }
        else
        {
            if (pipeline->frame_is_get_head && pipeline->frame_idx < pipeline->frame_buff_max_len - 2)
            {
                pipeline->frame_buff[pipeline->frame_idx++] = recv_byte;
            }
        }
    }
}








void protocol_pipe_put(struct protocol_pipe *pipeline,unsigned char * buff ,unsigned int len)
{
    unsigned int i ;
    for(i=0;i<len;i++)
    {
        RINGBUF_Put(& pipeline->ringbuf , buff[i]);
    }
}


unsigned char protocol_pipe_compare(const char * src , const char * aa ,int len)
{
    const char * start = src  ;
    int i = 0;

    if(*src == '\"') start = src+1;

    for(i=0 ; i<len;i++)
    {
        if(start[i] != aa[i])  return 0;
    }

    return 1;
    
}
















