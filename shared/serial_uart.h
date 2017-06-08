#ifndef SERIAL_UART_H
#define SERIAL_UART_H

#include "serial_interface.h"

class QSerialPort;
class RingBuffer;
class DataThread;

class Serial_UART : public SerialInterface
{
public:
    Serial_UART();

    void init(const char* name);
    bool open();
    bool close();

    void set_timeout(int32_t timeout_ms);
    void set_baudrate(int32_t baudrate);

    uint32_t send(const uint8_t *buf, uint32_t size);
    uint32_t recv(uint8_t* buf, uint32_t size);
private:
    QSerialPort*    m_serial;
    RingBuffer*     m_buf_recv;
    DataThread*     m_th_recv;
    RingBuffer*     m_buf_send;
    DataThread*     m_th_send;
    int             m_timeout;
};

#endif // SERIAL_UART_H
