#include <xc.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include "inc/emicbus2_core.h"
#include "inc/userFncFile.h"

#define TYPE_MSG   0x01
#define TAG_MAX    16

/* Send(tag, msg): formatea "tag\tmsg" al staging y arma la trama corta. */
void pI2C(char* format_, ...)
{
    va_list arg;
    uint8_t frame[4 + 255];
    uint16_t tl, dlen, len, crc, i;

    va_start(arg, format_);
    emicbus2_stage_reset();
    sendDataToStream(&emicbus2_stage_stream, format_, arg);
    va_end(arg);

    /* separar tag (hasta '\t') y payload; el '\t' no viaja (TAG_LEN nativo) */
    for (tl = 0; tl < emicbus2_stage_len && emicbus2_stage[tl] != '\t'; tl++);
    if (tl > TAG_MAX) { emicbus2_drop++; return; }
    dlen = (tl < emicbus2_stage_len) ? (uint16_t)(emicbus2_stage_len - tl - 1) : 0;
    len = (uint16_t)(1 + tl + dlen);
    if (len == 0 || len > 255) { emicbus2_drop++; return; }

    frame[0] = TYPE_MSG;
    frame[1] = (uint8_t)len;
    frame[2] = (uint8_t)tl;
    for (i = 0; i < tl; i++) frame[3 + i] = emicbus2_stage[i];
    for (i = 0; i < dlen; i++) frame[3 + tl + i] = emicbus2_stage[tl + 1 + i];
    crc = emicbus2_crc16(frame, 2 + len);
    frame[2 + len] = (uint8_t)(crc & 0xFF);
    frame[3 + len] = (uint8_t)(crc >> 8);
    emicbus2_tx(frame, (uint16_t)(4 + len));
}

void emicbus2_rx_MSG(const uint8_t *b, uint16_t n)
{
    char tag[TAG_MAX + 1];
    uint8_t len = b[1], tl = b[2];
    if (tl > TAG_MAX || (uint16_t)tl + 1 > len)
        return;                              /* ya contado por el core */
    memcpy(tag, &b[3], tl);
    tag[tl] = 0;
    emicbus2_in_bind(&b[3 + tl], (uint16_t)(len - 1 - tl));
    eI2C(tag, &emicbus2_in_stream);
}

