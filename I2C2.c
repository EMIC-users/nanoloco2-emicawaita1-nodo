
 #ifndef __PIC_I2C2_C__
 #define __PIC_I2C2_C__
 
 /*==================[inclusions]=============================================*/
 #include <xc.h>
 #include "inc/I2C2.h"
 #include "inc/system.h"
 
 /*==================[function implementation]================================*/
 
 void Init_Master_I2C2(uint16_t frec,uint8_t address)
 {
    //seteo el baudrate
    I2C2BRG = FCY / ((uint32_t)((uint32_t)frec * 1000) + 1 + ((float)FCY / 10000000));
    I2C2CONbits.SEN = 0;         //Start condition is not in progress
    I2C2CONbits.PEN = 0;
    I2C2CONbits.RSEN = 0;        //Initiates Repeated Start condition on SDAx and SCLx pins. Hardware is clear at the end of master Repeated Start sequence
    I2C2CONbits.RCEN = 0;
    I2C2CONbits.ACKEN = 0;
    I2C2CONbits.ACKDT = 0;       //1 = Sends a NACK during Acknowledge
                                        //0 = Sends an ACK during Acknowledge
    I2C2CONbits.STREN = 0;	    //1 : Enables software or receive clock stretching
                                        //0 : Disables software or receive clock stretching
    I2C2CONbits.GCEN = 1;        // interrupt -> 0 - disabled / 1 - enable
    I2C2CONbits.SMEN = 0;
    I2C2CONbits.DISSLW = 1;      //Slew rate control is disabled
    I2C2CONbits.A10M = 0;
    I2C2CONbits.IPMIEN = 0;
    I2C2CONbits.SCLREL = 1;
    I2C2CONbits.I2CSIDL = 1;     // Discontinues module operation when device enters an Idle mode
    I2C2ADD = address;
    I2C2RCV = 0x0000;
    I2C2TRN = 0x0000;
    I2C2CONbits.I2CEN = 1;       // Enables the I2Cx module and configures the SDAx and SCLx pins as serial port pins
    _MI2C2IE = 1;            // interrupt -> 0 - disabled / 1 - enable
    _SI2C2IE = 1;            // interrupt -> 0 - disabled / 1 - enable
 
 }
 
 void CollisionReset_I2C2()
 {
    I2C2STATbits.BCL = 0;
 }
 
 void OverflowReset_I2C2()
 {
    I2C2STATbits.I2COV = 0;
 }
 
 uint8_t IsStartI2c2()
 {
    return I2C2STATbits.S;
 }
 
 uint8_t IsCollisionDetectI2c2()
 {
    return I2C2STATbits.BCL;
 }
 
 uint8_t IsStopI2c2()
 {
    return I2C2STATbits.P;
 }
 
 uint8_t IsReceiveBufferFullI2c2()
 {
    return I2C2STATbits.RBF;
 }
 
 uint8_t IsDataOrAddressI2c2()
 {
    return I2C2STATbits.D_A;
 }
 
 uint8_t IsReceiveOverflowI2c2()
 {
    return I2C2STATbits.I2COV;
 }
 
 void Start_I2C2()
 {
    //This function generates an I2C start condition and returns status
    //of the Start.
    I2C2CONbits.SEN = 1;     //Generate Start Condition
    while (I2C2CONbits.SEN);
 
 }
 
 void Stop_I2C2()
 {
    //This function generates an I2C stop condition and returns status
    //of the Stop.
    I2C2CONbits.PEN = 1;     //Generate Stop Condition
    while (I2C2CONbits.PEN);
 }
 void Write_I2C2(unsigned char byte)
 {
    //This function transmits the byte passed to the function
    I2C2TRN = byte;					//Load byte to I2C1 Transmit buffer
    if (!1)
    {
       while (I2C2STATbits.TRSTAT);	//Wait for bus to be idle
    }
 }
 
 uint8_t Read_I2C2(uint8_t Ack)
 {
    uint8_t data = 0;

    //TODO: Revisar la viabilidad de implementar la macro interrupt  en la api del I2C
    
    if (!1)
    {
       I2C2CONbits.RCEN = 1;			//Enable Master receive
       Nop();
       while (!I2C2STATbits.RBF);		//Wait for receive bufer to be full
       data = I2C2RCV;
       if(Ack)
           I2C2CONbits.ACKDT = 1;
       else
           I2C2CONbits.ACKDT = 0;
       I2C2CONbits.ACKEN = 1;
       while (I2C2CONbits.ACKEN);
    }
    else
    {
        data = I2C2RCV;
        I2C2CONbits.SCLREL = 1;
    }
    return(data);				//Return data in buffer
 }
 
 void __attribute__((interrupt(auto_psv))) _SI2C2Interrupt( void )
 {
    _SI2C2IF = 0;
    I2c_driver_callback_slave();
 }
 
 void __attribute__((interrupt(auto_psv))) _MI2C2Interrupt( void )
 {
    _MI2C2IF = 0;
    I2c_driver_callback_master();
 }
 
 /*==================[end of file]============================================*/
 #endif
