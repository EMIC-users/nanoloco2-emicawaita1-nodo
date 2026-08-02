
/*==================[inclusions]=============================================*/
#include <xc.h>
#include "inc/gpio.h"
#include "inc/streamOut.h"
#include "inc/streamIn.h"
#include "inc/UART1.h"
#include "inc/system.h"


/*==================[internal data definition]===============================*/
const streamIn_t  streamIn_Uart1  = {UART1_IN_pop, UART1_IN_count};
const streamOut_t streamOut_Uart1 = {UART1_OUT_push, UART1_OUT_count};

uint8_t     UART1_flowControlVar = 0;
uint8_t     UART1_inverted       = 0;

UART_FIFO   UART1_IN_fifo;
UART_FIFO   UART1_OUT_fifo;

/*==================[public functions definition]==========================*/
void UART1_init_fifo(UART_FIFO *fifo) 
{
    fifo->start = 0;
    fifo->end = 0;
    fifo->count = 0;
}

uint16_t UART1_is_empty(UART_FIFO *fifo)
{
    return (fifo->count == 0);
}

uint8_t UART1_is_full(UART_FIFO *fifo) 
{
    return (fifo->count == UART1_MAX_BUFFER_SIZE);
}

uint16_t UART1_count(UART_FIFO *fifo)
{
	return fifo->count;
}

void UART1_push(UART_FIFO *fifo, char data)
{
    _U1RXIE = 0;
    _U1TXIE = 0;
    if (fifo==&UART1_OUT_fifo && fifo->count == 0) {
        _U1TXIF = 1;
    }
    fifo->buffer[fifo->end] = data;
    fifo->end = (fifo->end + 1) % UART1_MAX_BUFFER_SIZE;
    fifo->count++;
    if(fifo->count >= UART1_MAX_BUFFER_SIZE)
    {
        fifo->count = UART1_MAX_BUFFER_SIZE;
    }
    _U1RXIE = 1;
    _U1TXIE = 1;
}

char UART1_pop(UART_FIFO *fifo) 
{
    _U1RXIE = 0;
    _U1TXIE = 0;
    char data = 0;
    data = fifo->buffer[fifo->start];
    fifo->start = (fifo->start + 1) % UART1_MAX_BUFFER_SIZE;
    fifo->count--;
    _U1RXIE = 1;
    _U1TXIE = 1;
    return data;
}

char UART1_peek(UART_FIFO *fifo)
{
    // Calculate the index of the last element in the FIFO
    uint16_t last_index = (fifo->start + fifo->count - 1) % UART1_MAX_BUFFER_SIZE;
    // Return the last received value in the FIFO
    return fifo->buffer[last_index];
}

void UART1_simplexMode(void)
{
	U1MODEbits.RTSMD=1;
	U1MODEbits.UEN1=1;
	U1MODEbits.UEN0=0;
}

void UART1_flowControl(uint8_t enabled)
{
    U1MODEbits.RTSMD         =   0;
    U1MODEbits.UEN0          =   0;
    UART1_flowControlVar    =   enabled;
    U1MODEbits.UEN1          =   enabled;
}

void UART1_invert(uint8_t invert)
{
	U1STAbits.UTXINV =   invert;
    U1MODEbits.RXINV =   invert;
    UART1_inverted =   invert;
}

void UART1_bd(uint32_t uartSpeed)
{
    switch(uartSpeed)
    {
        case 600:
        case 1200:
        case 2400:
        case 4800:
        case 9600:
        case 14400:
        case 19200:
        case 28800:
        case 38400:
        case 56000:
        case 57600:
            U1BRG = ((FCY / (16 * uartSpeed)) - 1);
            break;

    #ifdef SYSTEM_FREQ
    #if SYSTEM_FREQ > 8000000
        case 115200:
        case 128000:
        case 153600:
        case 230400:
        case 256000:
            U1BRG = ((FCY / (16 * uartSpeed)) - 1);
            break;
    #endif
    #endif
        default:
            U1BRG = ((FCY / ((uint16_t)16 * 9600)) - 1);
        break;
    }
}

void UART1_init(void)
{
	__builtin_write_OSCCONL(OSCCON & 0xBF);
	/* Configure Input Functions (Table 10-2)) */

    RPOUT_MCP2200_TX =_RPOUT_U1TX;
    _U1RXR = RPIN_MCP2200_RX;

	#ifdef RPIN_MCP2200_CTS
		_U1CTSR = RPIN_MCP2200_CTS;
	#endif

	#ifdef RPOUT_MCP2200_RTS
		RPOUT_MCP2200_RTS = _RPOUT_U1RTS;
	#endif

	/* Configure Output Functions (Table 10-3) */

	/* Lock Registers */
	__builtin_write_OSCCONL(OSCCON | 0x40);

	/* Bit3 16 clocks per bit period*/
	U1MODEbits.BRGH = 0;
    
    UART1_bd((uint32_t)9600);

 	U1MODE = 0;                     //8-bit data, no parity, 1 stop bit
    U1MODEbits.UARTEN = 1;          //enable the module
	U1STAbits.UTXISEL0=0;
	U1STAbits.UTXISEL1=0;

	/* Enable transmission*/
 	U1STAbits.UTXEN = 1; 

    _U1RXIE = 1;
    _U1TXIE = 1;
    UART1_init_fifo(&UART1_IN_fifo);
    UART1_init_fifo(&UART1_OUT_fifo);
}

void UART1_OFF(void)
{
    U1MODEbits.UARTEN = 0;
 	U1STAbits.UTXEN = 0; 
    _U1RXIE = 0;
    _U1TXIE = 0;
}

void UART1_ON(void)
{
    U1MODEbits.UARTEN = 1;
 	U1STAbits.UTXEN = 1;
    _U1RXIE = 1;
    _U1TXIE = 1;
	UART1_init_fifo(&UART1_IN_fifo);
    UART1_init_fifo(&UART1_OUT_fifo);
}

void __attribute__((interrupt(auto_psv))) _U1TXInterrupt( void )
{
	/* Clear TX interrupt flag */
	_U1TXIF = 0;

	while(UART1_OUT_fifo.count)
	{
		if(!U1STAbits.UTXBF)
			U1TXREG = UART1_pop(&UART1_OUT_fifo);
		else
        {
            return;
        }
	}
}

void __attribute__((interrupt(auto_psv))) _U1RXInterrupt( void )
{
	char d;
   /* Clear RX interrupt flag */
    _U1RXIF = 0;
	if(U1STAbits.OERR)			//If an overflow occurred, clean the flag, otherwise RXREG doesn't update
    {
        U1STAbits.OERR = 0;	    //TODO: generar evento de error
    } 

	while (U1STAbits.URXDA)	    //While there is still data in the register
	{
		d = U1RXREG ;  
        ISR_UART1_CALLBACK(d);
	return;
	}
}

char UART1_IN_pop(void)
{
    return UART1_pop(&UART1_IN_fifo);
}

uint16_t UART1_IN_count(void)
{
    return UART1_IN_fifo.count;
}

void UART1_OUT_push(char d)
{
    UART1_push(&UART1_OUT_fifo, d);
}

uint16_t UART1_OUT_count(void)
{
    return UART1_MAX_BUFFER_SIZE - UART1_OUT_fifo.count;
}
