#ifndef SERIAL_INTERFACE_H
#define SERIAL_INTERFACE_H

#include <inttypes.h>

class SerialInterface
{
public:
    virtual void init(const char* name) = 0;
    virtual bool open() = 0;
    virtual bool close() = 0;

    virtual void set_timeout(int32_t timeout_ms) = 0;
    virtual void set_baudrate(int32_t baudrate) = 0;

    virtual uint32_t send(const uint8_t* buf, uint32_t size) = 0;
    virtual uint32_t recv(uint8_t* buf, uint32_t size) = 0;
};

#endif // SERIAL_INTERFACE_H
