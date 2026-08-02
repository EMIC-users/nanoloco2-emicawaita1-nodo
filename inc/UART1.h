#ifndef _UART1_H_
#define _UART1_H_

/*==================[inclusions]=============================================*/
#include <xc.h>
#include "streamOut.h"
#include "streamIn.h"

/*==================[macros and definitions]=================================*/
#define UART1_MAX_BUFFER_SIZE 512

typedef struct {
    char buffer[UART1_MAX_BUFFER_SIZE];
    uint16_t start;      // Pointer to the start of the queue
    uint16_t end;        // Pointer to the end of the queue
    uint16_t count;      // Count of data elements in the queue
} UART_FIFO;

/*==================[external data definition]===============================*/
extern uint8_t     UART1_flowControlVar;
extern uint8_t     UART1_inverted;

extern  UART_FIFO   UART1_IN_fifo;
extern  UART_FIFO   UART1_OUT_fifo;

/*==================[public functions definition]==========================*/

uint16_t UART1_count(UART_FIFO *fifo);

uint16_t UART1_is_empty(UART_FIFO *fifo);

uint8_t UART1_is_full(UART_FIFO *fifo);

void UART1_init_fifo(UART_FIFO *fifo);

void UART1_push(UART_FIFO *fifo, char data);

char UART1_pop(UART_FIFO *fifo);

char UART1_peek(UART_FIFO *fifo);

void UART1_simplexMode(void);

void UART1_flowControl(uint8_t enabled);

void UART1_invert(uint8_t invert);

void UART1_bd(uint32_t uartSpeed);

void UART1_init(void);

void UART1_OFF(void);

void UART1_ON(void);

char UART1_IN_pop(void);

uint16_t UART1_IN_count(void);

void UART1_OUT_push(char d);

uint16_t UART1_OUT_count(void);

extern const streamIn_t  streamIn_Uart1;
extern const streamOut_t streamOut_Uart1;

/*==================[external functions definition]==========================*/

extern void ISR_UART1_CALLBACK(char d);
#endif 
