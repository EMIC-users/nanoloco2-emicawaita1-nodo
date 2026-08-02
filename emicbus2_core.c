#include <xc.h>
#include <stdint.h>
#include <string.h>
#include "inc/emicbus2_core.h"
#include "inc/pins.h"

/*==================[contadores]============================================*/
uint16_t emicbus2_tx_ok, emicbus2_rx_ok, emicbus2_crc_err,
         emicbus2_arb_lost, emicbus2_bus_reset, emicbus2_drop;

/*==================[identidad: MODULE_ID = direccion I2C (spec 4.0)]=======*/
#define EMICBUS2_COMPILED_ID (26)

static uint8_t my_id = 0xFF;

uint8_t emicbus2_my_id(void) { return my_id; }

#define EMICBUS2_META_PAGE 0x00A400UL

static void flashRead(uint32_t pc, uint16_t *lo, uint8_t *hi)
{
    TBLPAG = (uint16_t)(pc >> 16);
    *lo = __builtin_tblrdl((uint16_t)(pc & 0xFFFF));
    *hi = (uint8_t)__builtin_tblrdh((uint16_t)(pc & 0xFFFF));
}

static uint8_t metaModuleId(void)
{
    uint16_t lo; uint8_t hi;
    flashRead(EMICBUS2_META_PAGE, &lo, &hi);
    if ((uint8_t)lo != 'E' || (uint8_t)(lo >> 8) != 'M' || hi != 'I')
        return 0xFF;
    flashRead(EMICBUS2_META_PAGE + 2, &lo, &hi);
    if ((uint8_t)lo != 'C')
        return 0xFF;
    flashRead(EMICBUS2_META_PAGE + 8, &lo, &hi);
    return (uint8_t)lo;
}

/*==================[CRC16-CCITT-FALSE, motor unico]========================*/
uint16_t emicbus2_crc16(const uint8_t *d, uint16_t n)
{
    uint16_t crc = 0xFFFF;
    uint16_t i;
    uint8_t b;
    for (i = 0; i < n; i++) {
        crc ^= (uint16_t)d[i] << 8;
        for (b = 0; b < 8; b++)
            crc = (crc & 0x8000) ? (crc << 1) ^ 0x1021 : (crc << 1);
    }
    return crc;
}

/*==================[staging de TX (streamOut_t -> buffer lineal)]==========*/
uint8_t  emicbus2_stage[EMICBUS2_STAGE_MAX];
uint16_t emicbus2_stage_len;

static void stagePut(char c)
{
    if (emicbus2_stage_len < EMICBUS2_STAGE_MAX)
        emicbus2_stage[emicbus2_stage_len++] = (uint8_t)c;
}

static uint16_t stageAvailable(void)
{
    return EMICBUS2_STAGE_MAX - emicbus2_stage_len;
}

const streamOut_t emicbus2_stage_stream = { stagePut, stageAvailable };

void emicbus2_stage_reset(void) { emicbus2_stage_len = 0; }

/*==================[streamIn_t del payload recibido]=======================*/
static const uint8_t *in_data;
static uint16_t in_len, in_idx;

static char inGet(void)
{
    return (in_idx < in_len) ? (char)in_data[in_idx++] : 0;
}

static uint16_t inCount(void) { return in_len - in_idx; }

const streamIn_t emicbus2_in_stream = { inGet, inCount };

void emicbus2_in_bind(const uint8_t *data, uint16_t len)
{
    in_data = data;
    in_len = len;
    in_idx = 0;
}

/*==================[esclavo GC: captura de transaccion (spec 3.2)]=========*/
#define EMICBUS2_RX_MAX 280
static uint8_t  rxbuf[EMICBUS2_RX_MAX];
static uint16_t rxlen = 0;
static uint8_t  in_txn = 0;

static void slaveDrain(void)
{
    uint8_t d;
    if (I2C2STATbits.I2COV) I2C2STATbits.I2COV = 0;
    while (I2C2STATbits.RBF) {
        d = I2C2RCV;
        if (!I2C2STATbits.D_A) {     /* byte de direccion: nueva txn */
            in_txn = 1;
            rxlen = 0;
        } else if (in_txn && rxlen < EMICBUS2_RX_MAX) {
            rxbuf[rxlen++] = d;
        }
    }
}

static void rxReset(void)
{
    rxlen = 0;
    in_txn = 0;
}

/*==================[I2C: init, bus-clear (D1) y master TX]=================*/
static void i2cHwInit(void)
{
    I2C2BRG = (uint16_t)(16000000UL / 100000UL - 1);
    I2C2ADD = (my_id != 0xFF) ? my_id : 0;
    I2C2CONbits.STREN = 0;
    I2C2CONbits.GCEN = 1;
    I2C2CONbits.DISSLW = 1;
    I2C2CONbits.I2CEN = 1;
}

static void busRecovery(void)
{
    uint8_t i;
    uint16_t d;
    I2C2CON = 0;
    TRIS_SDA = 1;
    LAT_SCL = 0;
    TRIS_SCL = 0;
    for (i = 0; i < 9; i++) {
        LAT_SCL = 0; for (d = 0; d < 80; d++) asm volatile ("nop");
        LAT_SCL = 1; for (d = 0; d < 80; d++) asm volatile ("nop");
    }
    LAT_SDA = 0; TRIS_SDA = 0;
    for (d = 0; d < 80; d++) asm volatile ("nop");
    TRIS_SDA = 1;
    TRIS_SCL = 1;
    i2cHwInit();
    rxReset();
    emicbus2_bus_reset++;
}

#define TXR_OK   0
#define TXR_BCL  1
#define TXR_FAIL 2

static uint8_t i2cWriteByteM(uint8_t d)
{
    uint16_t guard = 0;
    I2C2TRN = d;
    while (I2C2STATbits.TRSTAT && ++guard < 50000)
        slaveDrain();
    if (I2C2STATbits.BCL)
        return TXR_BCL;
    if (guard >= 50000)
        return TXR_FAIL;
    slaveDrain();
    return TXR_OK;
}

static uint8_t busWrite(const uint8_t *frame, uint16_t n)
{
    uint16_t i, guard = 0;
    uint8_t r;

    while ((I2C2STATbits.S || in_txn) && ++guard < 50000)
        slaveDrain();
    if (I2C2STATbits.S) { busRecovery(); return TXR_FAIL; }

    if (I2C2STATbits.BCL) I2C2STATbits.BCL = 0;
    if (I2C2STATbits.IWCOL) I2C2STATbits.IWCOL = 0;
    I2C2CONbits.SEN = 1;
    guard = 0;
    while (I2C2CONbits.SEN && ++guard < 50000);
    if (I2C2STATbits.IWCOL) { I2C2STATbits.IWCOL = 0; return TXR_BCL; }
    if (!I2C2STATbits.S) { busRecovery(); return TXR_FAIL; }

    r = i2cWriteByteM(0x00);                /* general call + W */
    for (i = 0; r == TXR_OK && i < n; i++)
        r = i2cWriteByteM(frame[i]);

    if (r == TXR_BCL) {
        I2C2STATbits.BCL = 0;
        return TXR_BCL;
    }
    if (r == TXR_FAIL) { busRecovery(); return TXR_FAIL; }

    I2C2CONbits.PEN = 1;
    guard = 0;
    while (I2C2CONbits.PEN && ++guard < 50000);
    while (!I2C2STATbits.P && ++guard < 60000);
    slaveDrain();
    rxReset();                              /* descarte por estado: era el eco */
    return TXR_OK;
}

/*==================[cola de TX: 2 slots, backoff y recovery-retry]=========*/
#define EMICBUS2_TX_SLOTS 2
#define EMICBUS2_TX_MAX   (EMICBUS2_STAGE_MAX + 24)
static uint8_t  txq[EMICBUS2_TX_SLOTS][EMICBUS2_TX_MAX];
static uint16_t txq_len[EMICBUS2_TX_SLOTS];
static uint8_t  txq_r, txq_w, txq_n;
static uint8_t  tx_tries;
static uint16_t tx_backoff;                 /* pasadas de poll a esperar */

uint8_t emicbus2_tx_idle(void) { return txq_n == 0; }

uint8_t emicbus2_tx(const uint8_t *frame, uint16_t n)
{
    if (txq_n >= EMICBUS2_TX_SLOTS || n > EMICBUS2_TX_MAX) {
        emicbus2_drop++;
        return 0;
    }
    memcpy(txq[txq_w], frame, n);
    txq_len[txq_w] = n;
    txq_w = (uint8_t)((txq_w + 1) % EMICBUS2_TX_SLOTS);
    txq_n++;
    return 1;
}

static void txPoll(void)
{
    uint8_t r;
    if (!txq_n || in_txn)
        return;
    if (tx_backoff) { tx_backoff--; return; }
    r = busWrite(txq[txq_r], txq_len[txq_r]);
    if (r == TXR_OK) {
        emicbus2_tx_ok++;
        txq_r = (uint8_t)((txq_r + 1) % EMICBUS2_TX_SLOTS);
        txq_n--;
        tx_tries = 0;
    } else if (r == TXR_BCL) {
        emicbus2_arb_lost++;
        tx_backoff = (uint16_t)(20 + (TMR1 & 0x7F));   /* re-arbitraje */
    } else {                                /* FAIL: recovery hecho */
        if (++tx_tries >= 2) {
            txq_r = (uint8_t)((txq_r + 1) % EMICBUS2_TX_SLOTS);
            txq_n--;
            tx_tries = 0;
            emicbus2_drop++;
        } else {
            tx_backoff = 500;
        }
    }
}

/*==================[validacion + selector de frames (plan 3.3)]============*/
static void dispatch(const uint8_t *b, uint16_t n)
{
    /* validacion estructural + CRC por familia (spec 2.2/2.3) */
    if (n >= 4 && b[0] == 0x01) {           /* corta */
        uint8_t len = b[1];
        if (len == 0 || n != (uint16_t)(4 + len)) { emicbus2_drop++; return; }
        if (emicbus2_crc16(b, 2 + len) !=
            ((uint16_t)b[2 + len] | ((uint16_t)b[3 + len] << 8))) {
            emicbus2_crc_err++; return;
        }
    } else if (n >= 9 && b[0] >= 0x10) {    /* larga */
        uint16_t plen = (uint16_t)b[5] | ((uint16_t)b[6] << 8);
        if ((b[1] & 0xE0) != 0x20 || n != 9 + plen || plen > 256) { emicbus2_drop++; return; }
        if (emicbus2_crc16(b, 7 + plen) !=
            ((uint16_t)b[7 + plen] | ((uint16_t)b[8 + plen] << 8))) {
            emicbus2_crc_err++; return;
        }
        if (b[2] != 0x00 && b[2] != my_id)
            return;
    } else {
        emicbus2_drop++;
        return;
    }

    emicbus2_rx_ok++;
    switch (b[0]) {
    case 0x01:
        emicbus2_rx_MSG(b, n);
        break;
    case 0x12:
        emicbus2_rx_CTRL(b, n);
        break;
    default:
        emicbus2_drop++;                    /* TYPE sin frame registrado */
        break;
    }
}

/*==================[respuesta ACK extendido (spec: [seq][datos...])]=======*/
static uint8_t txseq;

void emicbus2_reply_ack_ext(uint8_t dst, uint8_t seq_confirmado,
                            const uint8_t *data, uint8_t n)
{
    uint8_t f[9 + 1 + 48];
    uint16_t crc, plen = (uint16_t)(1 + n);
    uint8_t i;
    if (n > 48)
        return;
    f[0] = 0x10;                            /* TYPE_ACK */
    f[1] = 0x20;                            /* VER v2 */
    f[2] = dst;
    f[3] = my_id;                           /* 0xFF si no hay identidad */
    f[4] = txseq++;
    f[5] = (uint8_t)plen; f[6] = 0;
    f[7] = seq_confirmado;
    for (i = 0; i < n; i++) f[8 + i] = data[i];
    crc = emicbus2_crc16(f, (uint16_t)(7 + plen));
    f[7 + plen] = (uint8_t)(crc & 0xFF);
    f[8 + plen] = (uint8_t)(crc >> 8);
    emicbus2_tx(f, (uint16_t)(9 + plen));
}

/*==================[init + poll]===========================================*/
void EMICBus2_init(void)
{
    uint8_t m = metaModuleId();
    if (m >= 0x08 && m <= 0x77)
        my_id = m;                          /* lo deployado manda */
    else if ((EMICBUS2_COMPILED_ID) >= 0x08 && (EMICBUS2_COMPILED_ID) <= 0x77)
        my_id = (uint8_t)(EMICBUS2_COMPILED_ID);
    i2cHwInit();
}

void poll_EMICBus2(void)
{
    slaveDrain();
    if (in_txn && I2C2STATbits.P) {  /* STOP: transaccion completa */
        in_txn = 0;
        if (rxlen) dispatch(rxbuf, rxlen);
        rxlen = 0;
    }
    txPoll();
}

