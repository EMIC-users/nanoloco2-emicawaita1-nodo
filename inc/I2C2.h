
#ifndef __PIC_I2C2_H__
#define __PIC_I2C2_H__

/*==================[inclusions]=============================================*/

#include <xc.h>

/*==================[macros]=================================================*/


/*==================[typedef]================================================*/


/*==================[internal function declaration]==========================*/

void Init_Master_I2C2(uint16_t frec,uint8_t address);

void CollisionReset_I2C2();

void OverflowReset_I2C2();
void Start_I2C2();
void Stop_I2C2();
void Write_I2C2(unsigned char byte);

uint8_t Read_I2C2(uint8_t Ack);

uint8_t IsStartI2c2();

uint8_t IsCollisionDetectI2c2();

uint8_t IsStopI2c2();

uint8_t IsReceiveBufferFullI2c2();

uint8_t IsDataOrAddressI2c2();

uint8_t IsReceiveOverflowI2c2();

extern void I2c_driver_callback_slave(void);
extern void I2c_driver_callback_master(void);

/*==================[end of file]============================================*/
#endif

