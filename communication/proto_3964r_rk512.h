#ifndef PROTO_3964R_RK512_H
#define PROTO_3964R_RK512_H

/**
 * @brief The proto_3964R_RK512 class, implementation 3964R/RK512 protocol.
 * See more:
 *  https://de.wikipedia.org/wiki/RK512
 *  https://support.industry.siemens.com/cs/document/12932189/differences-between-protocols-3964(r)-and-rk-512?dti=0&lc=en-WW
 */

#include "serial_interface.h"
#include <inttypes.h>

uint16_t swap_(uint16_t v);

enum RK512State
{
    RK512_UNINITIALIZED,
    RK512_INITIALIZED,
    RK512_DISCONNECTED,
    RK512_CONNECTED,
    RK512_SEND_ACTIVE,
    RK512_RECV_ACTIVE,
    RK512_RETRY,
};

#pragma pack(1)
typedef struct
{
    uint16_t    type;
    uint8_t     direction;
    uint8_t     data_type;
    union
    {
        struct
        {
            uint8_t db_idx;
            uint8_t db_offset;
        };
        uint16_t    mem_offset;
    };
    uint16_t    data_length;
    uint8_t     sync_flags;
    uint8_t     cpu_addr;
} rk512_request_header_t;

typedef struct
{
    uint16_t    type;
    uint8_t     reply_code;
    uint8_t     error;
} rk512_reply_header_t;
#pragma pack()

#define RK512_TYPE_START 0x0000
#define RK512_TYPE_START_EXT 0xff00

#define RK512_DIR_SEND 'A'
#define RK512_DIR_SEND_EXT 'O'
#define RK512_DIR_RECV 'E'

#define RK512_DATA_TYPE_DB 'D'
#define RK512_DATA_TYPE_DB_EXT 'X'
#define RK512_DATA_TYPE_INPUT 'E'
#define RK512_DATA_TYPE_OUTPUT 'A'
#define RK512_DATA_TYPE_MEM 'M'
#define RK512_DATA_TYPE_PERIPH 'P'
#define RK512_DATA_TYPE_COUNTER 'Z'
#define RK512_DATA_TYPE_TIMER 'T'
#define RK512_DATA_TYPE_ABS_MEM_ADDR 'S'
#define RK512_DATA_TYPE_SYS_ADDR 'B'

#define RK512_SYNC_FLAGS_NO_FLAGS 0xff

#define RK512_CPU_ADDR_MASK_FLAGS 0x0f
#define RK512_CPU_ADDR_MASK_ADDR  0xf0
#define RK512_CPU_ADDR_ANY 0x00
#define RK512_CPU_ADDR_NO_FLAGS_ANY 0xff

#define RK512_REPLY_CODE_REPLY 0x00

#define RK512_REPLY_ERROR_NO_ERROR 0x00

#define RK512_3964R_STX 0x02
#define RK512_3964R_DLE 0x10
#define RK512_3964R_ETX 0x03
#define RK512_3964R_NAK 0x15

#define RK512_BUF_SIZE 256

class Proto_3964R_RK512
{
public:
    Proto_3964R_RK512(SerialInterface* serial);

    RK512State get_state() { return m_state; }

    bool send_db(uint8_t db_idx, uint8_t db_offset, uint16_t length, uint16_t *data);
    bool recv_db(uint8_t db_idx, uint8_t db_offset, uint16_t length, uint16_t *data);
private:
    SerialInterface*    m_serial;
    RK512State          m_state;
    uint8_t             m_send_buf[RK512_BUF_SIZE];
    uint8_t             m_recv_buf[RK512_BUF_SIZE];

    void calc_bcc(uint8_t* buf, uint32_t len);
    bool check_bcc(uint8_t* buf, uint32_t len, uint8_t bcc_check);
    bool connect();
    uint8_t get_dle();
    bool send_dle();
    bool get_start();
    bool send_start();

    bool recv_reply(uint32_t data_bytes);
};

#endif // PROTO_3964R_RK512_H
