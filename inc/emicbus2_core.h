#ifndef _EMICBUS2_CORE_H_
#define _EMICBUS2_CORE_H_
#include <stdint.h>
#include "streamOut.h"
#include "streamIn.h"

/* contadores */
extern uint16_t emicbus2_tx_ok, emicbus2_rx_ok, emicbus2_crc_err,
                emicbus2_arb_lost, emicbus2_bus_reset, emicbus2_drop;

uint16_t emicbus2_crc16(const uint8_t *d, uint16_t n);

#define EMICBUS2_STAGE_MAX 200
extern const streamOut_t emicbus2_stage_stream;
extern uint8_t  emicbus2_stage[EMICBUS2_STAGE_MAX];
extern uint16_t emicbus2_stage_len;
void emicbus2_stage_reset(void);

uint8_t emicbus2_tx(const uint8_t *frame, uint16_t n);

/* 1 = cola de TX vacia (para drenar antes de un reset). */
uint8_t emicbus2_tx_idle(void);

/* streamIn_t de lectura del payload recibido (para eI2C y $r) */
extern const streamIn_t emicbus2_in_stream;
void emicbus2_in_bind(const uint8_t *data, uint16_t len);

uint8_t emicbus2_my_id(void);

void emicbus2_reply_ack_ext(uint8_t dst, uint8_t seq_confirmado,
                            const uint8_t *data, uint8_t n);

void EMICBus2_init(void);
void poll_EMICBus2(void);

/* handlers de RX aportados por cada frame registrado (frames/frame_X.c) */
void emicbus2_rx_MSG(const uint8_t *b, uint16_t n);
void emicbus2_rx_CTRL(const uint8_t *b, uint16_t n);

#endif

