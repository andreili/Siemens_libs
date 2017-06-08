#include "serial_uart.h"
#include <QtSerialPort/QSerialPort>
#include <ringbuffer.h>
#include <QThread>

class DataThread : public QThread
{
public:
    DataThread(bool is_read, int timeout, QSerialPort* serial, RingBuffer* buf)
        : m_is_read(is_read)
        , m_timeout (timeout)
        , m_serial (serial)
        , m_buf (buf)
    { }

    void run()
    {
        uint8_t buf[2048];
        while (1)
        {
            if (m_is_read)
            {
                if (m_serial->waitForReadyRead(5))
                {
                    int readed = m_serial->read((char*)buf, 2048);
                    for (int i=0 ; i<readed ; ++i)
                        m_buf->put(buf[i]);
                }
            }
            else
            {
                int to_write = m_buf->buf_size();
                if (to_write)
                {
                    for (int i=0 ; i<to_write ; ++i)
                        buf[i] = m_buf->get();
                    m_serial->write((char*)buf, to_write);
                    m_serial->waitForBytesWritten(m_timeout);
                }
            }
        }
    }

private:
    bool            m_is_read;
    int             m_timeout;
    QSerialPort*    m_serial;
    RingBuffer*     m_buf;
};

Serial_UART::Serial_UART()
{
}

void Serial_UART::init(const char* name)
{
    m_serial = new QSerialPort(name);

    m_buf_recv = new RingBuffer();
    m_th_recv = new DataThread(true, m_timeout, m_serial, m_buf_recv);
    m_th_recv->start();

    m_buf_send = new RingBuffer();
    m_th_send = new DataThread(false, m_timeout, m_serial, m_buf_send);
    m_th_send->start();
}

bool Serial_UART::open()
{
    if (!m_serial->open(QIODevice::ReadWrite))
        return false;
    m_serial->setDataBits(QSerialPort::DataBits::Data8);
    m_serial->setParity(QSerialPort::Parity::EvenParity);
    m_serial->setStopBits(QSerialPort::StopBits::OneStop);
    return true;
}

bool Serial_UART::close()
{
    m_serial->close();
    return true;
}

void Serial_UART::set_timeout(int32_t timeout_ms)
{
    m_timeout = timeout_ms;
}

void Serial_UART::set_baudrate(int32_t baudrate)
{
    m_serial->setBaudRate(baudrate);
}

uint32_t Serial_UART::send(const uint8_t* buf, uint32_t size)
{
    for (uint32_t i=0 ; i<size ; ++i)
        m_buf_send->put(buf[i]);
    return size;
    /*if (m_serial->write((const char*)buf, size) != size)
        return false;
    if (!m_serial->waitForBytesWritten(m_timeout))
        return false;
    return true;*/
}

uint32_t Serial_UART::recv(uint8_t *buf, uint32_t size)
{
    m_buf_recv->wait_while_revieve_data(size);
    for (uint32_t i=0 ; i<size ; ++i)
        buf[i] = m_buf_recv->get();
    return size;
    /*uint32_t readed = 0;
    while (readed < size)
    {
        if (!m_serial->waitForReadyRead(m_timeout))
            break;
        readed += m_serial->read((char*)&buf[readed], size - readed);
    }
    if (readed != size)
        return false;
    return true;*/
}
