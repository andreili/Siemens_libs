#include "proto_3964r_rk512.h"
#include <string.h>
#include <QThread>
#include <QFile>

Proto_3964R_RK512::Proto_3964R_RK512(SerialInterface *serial)
    : m_serial (serial)
    , m_state (RK512_UNINITIALIZED)
{
    m_serial->set_timeout(300);
}

uint16_t swap_(uint16_t v)
{
  return ((v & 0xFF) << 8) | ((v & 0xFF00) >> 8);
}

bool Proto_3964R_RK512::send_db(uint8_t db_idx, uint8_t db_offset, uint16_t length, uint16_t* data)
{
    /*while (!connect() && (m_state == RK512_RETRY)) {}

    m_state = RK512_SEND_ACTIVE;
    rk512_request_header_t* req_header = (rk512_request_header_t*)&m_send_buf;
    req_header->type = RK512_TYPE_START;
    req_header->direction = RK512_DIR_SEND;
    req_header->data_type = RK512_DATA_TYPE_DB;
    req_header->db_idx = db_idx;
    req_header->db_offset = db_offset;
    req_header->data_length = swap_(length);
    req_header->sync_flags = RK512_SYNC_FLAGS_NO_FLAGS;
    req_header->cpu_addr = RK512_CPU_ADDR_NO_FLAGS_ANY;

    // convert LE to BE
    int l = length * 2;
    for (int i=0 ; i<l ; ++i)
        *((uint16_t*)&m_send_buf[i*2 + sizeof(rk512_request_header_t)]) = swap_(data[i]);

    m_send_buf[sizeof(rk512_request_header_t) + length * 2] = RK512_3964R_DLE;
    m_send_buf[sizeof(rk512_request_header_t) + length * 2 + 1] = RK512_3964R_ETX;
    calc_bcc(m_send_buf, sizeof(rk512_request_header_t) + length * 2 + 2);

    if (!m_serial->send(m_send_buf, sizeof(rk512_request_header_t) + length * 2 + 3))
        return false;
    if (get_dle() != RK512_3964R_DLE)
        return false;

    m_state = RK512_RECV_ACTIVE;

    if (!get_start())
        return false;
    if (!send_dle())
        return false;
    if (!recv_reply(0))
        return false;
    if (!send_dle())
        return false;

    m_state = RK512_CONNECTED;

    return true;
}

bool Proto_3964R_RK512::recv_db(uint8_t db_idx, uint8_t db_offset, uint16_t length, uint16_t* data)
{
    while (!connect() && (m_state == RK512_RETRY)) {}

    m_state = RK512_SEND_ACTIVE;
    rk512_request_header_t* req_header = (rk512_request_header_t*)&m_send_buf;
    req_header->type = RK512_TYPE_START;
    req_header->direction = RK512_DIR_RECV;
    req_header->data_type = RK512_DATA_TYPE_DB;
    req_header->db_idx = db_idx;
    req_header->db_offset = db_offset;
    req_header->data_length = swap_(length);
    req_header->sync_flags = RK512_SYNC_FLAGS_NO_FLAGS;
    req_header->cpu_addr = RK512_CPU_ADDR_NO_FLAGS_ANY;
    m_send_buf[sizeof(rk512_request_header_t)] = RK512_3964R_DLE;
    m_send_buf[sizeof(rk512_request_header_t) + 1] = RK512_3964R_ETX;
    calc_bcc(m_send_buf, sizeof(rk512_request_header_t) + 2);
    if (!m_serial->send(m_send_buf, sizeof(rk512_request_header_t) + 3))
        return false;
    if (get_dle() != RK512_3964R_DLE)
        return false;

    m_state = RK512_RECV_ACTIVE;

    if (!get_start())
        return false;
    if (!send_dle())
        return false;
    bool recv_err = !recv_reply(length * 2);
    if (!send_dle() || recv_err)
        return false;

    // convert BE to LE
    int l = length * 2;
    for (int i=0 ; i<l ; ++i)
        data[i] = swap_(*((uint16_t*)&m_recv_buf[i*2 + sizeof(rk512_reply_header_t)]));

    m_state = RK512_CONNECTED;
    return true;
}

void Proto_3964R_RK512::calc_bcc(uint8_t* buf, uint32_t len)
{
    uint8_t bcc = 0;
    for (uint32_t i=0 ; i<len ; ++i)
        bcc ^= buf[i];
    buf[len] = bcc;
}

bool Proto_3964R_RK512::check_bcc(uint8_t* buf, uint32_t len, uint8_t bcc_check)
{
    uint8_t bcc = 0;
    for (uint32_t i=0 ; i<len ; ++i)
        bcc ^= buf[i];
    printf("BCC: got 0x%X, calc 0x%X\n", bcc_check, bcc);
    return (bcc == bcc_check);
}

bool Proto_3964R_RK512::connect()
{
    m_state = RK512_DISCONNECTED;
    if (!send_start())
        return false;
    switch (get_dle())
    {
    case RK512_3964R_DLE:
        // подключение установлено
        m_state = RK512_CONNECTED;
        return true;
    case RK512_3964R_STX:
        // приемник хочет передать данные - принимаем их и повторяем попытку подключения
        printf("Client has data!\n");
        send_dle();
        recv_reply(1024);
        m_state = RK512_RETRY;
        return false;
    case RK512_3964R_NAK:
        // приемник не готов к общению - требуется повторить попытку позже
        printf("Client dont ready!\n");
        m_state = RK512_RETRY;
        return false;
    default:
        return false;
    }
}

uint8_t Proto_3964R_RK512::get_dle()
{
    if (!m_serial->recv(m_recv_buf, 1))
        return false;
    return m_recv_buf[0];
}

bool Proto_3964R_RK512::send_dle()
{
    m_send_buf[0] = RK512_3964R_DLE;
    if (!m_serial->send(m_send_buf, 1))
        return false;
    return true;
}

bool Proto_3964R_RK512::get_start()
{
    if (!m_serial->recv(m_recv_buf, 1))
        return false;
    if (m_recv_buf[0] != RK512_3964R_STX)
        return false;
    return true;
}

bool Proto_3964R_RK512::send_start()
{
    m_send_buf[0] = RK512_3964R_STX;
    if (!m_serial->send(m_send_buf, 1))
        return false;
    return true;
}

bool Proto_3964R_RK512::recv_reply(uint32_t data_bytes)
{
    int to_recv = sizeof(rk512_reply_header_t) + data_bytes + 3;
    int readed = m_serial->recv(m_recv_buf, to_recv);
    if (readed != to_recv)
        return false;
    while ((m_recv_buf[readed - 3] != 0x10) || (m_recv_buf[readed - 2] != 0x03))
        readed += m_serial->recv(&m_recv_buf[readed], 1);

    rk512_reply_header_t* header = (rk512_reply_header_t*)&m_recv_buf;
    if (header->error != RK512_REPLY_ERROR_NO_ERROR)
        return false;
    if (check_bcc(m_recv_buf, readed - 1, m_recv_buf[readed - 1]))
    {
        return true;
    }
    else
    {
        printf("BCC Error!\n");
        return false;
    }
}
