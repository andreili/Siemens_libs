#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <inttypes.h>

#define RING_BUFFER_SIZE 4096

class RingBuffer
{
public:
    RingBuffer();

    int buf_size();
    void wait_while_revieve_data(int wait_size);
    uint8_t get();
    void put(uint8_t data);
private:
    int     m_buf_start;
    int     m_buf_end;
    uint8_t m_buf[RING_BUFFER_SIZE];
};

#endif // RINGBUFFER_H
