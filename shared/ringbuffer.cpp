#include "ringbuffer.h"
#include <QThread>
#include <algorithm>
#include <cstring>

RingBuffer::RingBuffer()
    : m_buf_start (0)
    , m_buf_end (0)
{

}

int RingBuffer::buf_size()
{
    if (m_buf_end >= m_buf_start)
        return m_buf_end - m_buf_start;
    else
        return ((RING_BUFFER_SIZE - m_buf_start) + m_buf_end);
}

void RingBuffer::wait_while_revieve_data(int wait_size)
{
    while (buf_size() < wait_size)
        QThread::msleep(1);
    printf("Buf size: %i bytes\n", buf_size());
}

uint8_t RingBuffer::get()
{
    if (buf_size() == 0)
        return 0;

    int pos = m_buf_start;
    if (++m_buf_start >= RING_BUFFER_SIZE)
        m_buf_start = 0;
    return m_buf[pos];
}

void RingBuffer::put(uint8_t data)
{
    if ((m_buf_end+1) == m_buf_start)
        return;

    m_buf[m_buf_end] = data;

    if (++m_buf_end >= RING_BUFFER_SIZE)
        m_buf_end = 0;
}
