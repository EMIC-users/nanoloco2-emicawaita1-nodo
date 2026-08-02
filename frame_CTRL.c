#include <xc.h>
#include <stdint.h>
#include "inc/emicbus2_core.h"

#define CTRL_GET_STATS 0x03
#define CTRL_CLR_STATS 0x04
#define CTRL_PING      0x05
#define CTRL_RESET     0x06

static void statPack(uint8_t *d, uint8_t idx, uint8_t id, uint16_t val)
{
    uint8_t *p = &d[1 + 5 * idx];
    p[0] = id;
    p[1] = (uint8_t)(val & 0xFF);
    p[2] = (uint8_t)(val >> 8);
    p[3] = 0;
    p[4] = 0;
}

/* RX (trama larga ya validada por el core: layout, CRC y DST logico). */
void emicbus2_rx_CTRL(const uint8_t *b, uint16_t n)
{
    uint16_t plen = (uint16_t)b[5] | ((uint16_t)b[6] << 8);
    uint8_t src = b[3], seq = b[4];
    (void)n;
    if (plen < 1)
        return;

    if (b[2] == 0x00) {                     /* broadcast: solo RESET */
        if (b[7] == CTRL_RESET)
            asm volatile ("reset");         /* -> ventana del bootloader */
        return;
    }

    /* unicast dirigido a mi (el core ya filtro DST == my_id) */
    switch (b[7]) {

    case CTRL_PING: {                       /* eco <=16 B */
        uint8_t elen = (uint8_t)(plen - 1);
        if (elen > 16) elen = 16;
        emicbus2_reply_ack_ext(src, seq, &b[8], elen);
        break;
    }

    case CTRL_GET_STATS: {
        uint8_t d[1 + 5 * 6];
        d[0] = 6;
        statPack(d, 0, 1, emicbus2_tx_ok);
        statPack(d, 1, 2, emicbus2_rx_ok);
        statPack(d, 2, 3, emicbus2_crc_err);
        statPack(d, 3, 4, emicbus2_arb_lost);
        statPack(d, 4, 5, emicbus2_bus_reset);
        statPack(d, 5, 6, emicbus2_drop);
        emicbus2_reply_ack_ext(src, seq, d, sizeof(d));
        break;
    }

    case CTRL_CLR_STATS:
        emicbus2_tx_ok = emicbus2_rx_ok = emicbus2_crc_err = 0;
        emicbus2_arb_lost = emicbus2_bus_reset = emicbus2_drop = 0;
        emicbus2_reply_ack_ext(src, seq, 0, 0);
        break;

    case CTRL_RESET:
        emicbus2_reply_ack_ext(src, seq, 0, 0);
        {
            uint16_t guard = 0;
            while (++guard < 50000) {
                poll_EMICBus2();
                if (emicbus2_tx_idle())
                    break;
            }
        }
        asm volatile ("reset");
        break;

    default:
        break;                              /* UNSUPPORTED: silencio en E3 */
    }
}

