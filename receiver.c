/*
 * File:   receiver.c
 * Author: Liam
 *
 * Created on April 14, 2017, 4:31 PM
 */

#include <stdint.h>
#include <xc.h>
#include "spi.h"
#include "NRF_consts.h"

// PIC16F1503 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1
#pragma config FOSC = INTOSC    // Oscillator Selection Bits (INTOSC oscillator: I/O function on CLKIN pin)
#pragma config WDTE = OFF       // Watchdog Timer Enable (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable (PWRT disabled)
#pragma config MCLRE = ON       // MCLR Pin Function Select (MCLR/VPP pin function is MCLR)
#pragma config CP = OFF         // Flash Program Memory Code Protection (Program memory code protection is disabled)
#pragma config BOREN = OFF      // Brown-out Reset Enable (Brown-out Reset disabled)
#pragma config CLKOUTEN = OFF   // Clock Out Enable (CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin)

// CONFIG2
#pragma config WRT = OFF        // Flash Memory Self-Write Protection (Write protection off)
#pragma config STVREN = ON      // Stack Overflow/Underflow Reset Enable (Stack Overflow or Underflow will cause a Reset)
#pragma config BORV = LO        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (Vbor), low trip point selected.)
#pragma config LPBOR = OFF      // Low-Power Brown Out Reset (Low-Power BOR is disabled)
#pragma config LVP = OFF        // Low-Voltage Programming Enable (High-voltage on MCLR/VPP must be used for programming)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.
// need to set TRIS bits:
/* SDIx must have TRIS bit set
 SDOx must have TRIS bit cleared
 * SCKx (master mode) must have TRIS bit cleared
 * SSx must have TRIS bit set
 
 */



#define _XTAL_FREQ 16000000UL

void configureRX(void);
void blink(int delay);
uint8_t readData(void);

void main(void) {
    uint8_t last = 1;
    SPI_init();
    LED = 0;
    configureRX();
    while(1) {
        NRF_CE = 1;
        while(IRQ); 
        NRF_CE = 0;
        uint8_t data = readData();
        resetIRQ();
        if(data == 0xCF)
        {
            blink(10);
        }
    }
}

void blink(int delay) {
    LED = 1;
    for(int i = 0; i < delay; ++i) {
        __delay_ms(5);
    }
    LED = 0;
    for(int i = 0; i < delay; ++i) {
        __delay_ms(5);
    }
}


void configureRX(void) {
    // 1. need to pull PRIM_RX bit low
    // 2. clock in address for receiving node (TX_ADDR) and payload data
    // (TX_PLD) via SPI. Must write TX_PLD continuously while holding CSN low.
    // TX_ADDR does not have to be rewritten if unchanged from last transmit!
    // If PTX device shall receive acknowledge, data pipe 0 has to be configured
    // to receive the acknowledgment
    // 3. A high pulse of CE starts the transmission. Minimum pulse width of
    // 10 microseconds
    NRF_CE = 0;
    __delay_ms(1);
    SS_PIN = 1;
    __delay_ms(10);
    uint8_t write[2];
    write[0] = (CONFIG & REGISTER_MASK) | W_MASK;
    write[1] = 0b00111011;  // config stuff, PRX, power up
    SPI_writeArray(write, 2);
    
   
    write[0] = (EN_AA & REGISTER_MASK) | W_MASK;
    write[1] = 0b00000001;
    SPI_writeArray(write, 2); // enable auto-acknowledgement on pipe 0
    
    write[0] = (EN_RX_ADDR & REGISTER_MASK) | W_MASK;
    write[1] = 0b00000001; // enable data pipe 0
    SPI_writeArray(write, 2);
    
    write[0] = (RX_PW_P[0] & REGISTER_MASK) | W_MASK;
    write[1] = 0b00000001;
    SPI_writeArray(write, 2); // 1 byte in RX payload in data pipe 0
    
    write[0] = (SETUP_AW & REGISTER_MASK) | W_MASK;
    write[1] = 0b0000011;
    SPI_writeArray(write, 2);
    
}


uint8_t readData(void) {
    SS_PIN = 0;
    SPI_write_byte(R_RX_PAYLOAD);
    uint8_t ret = SPI_write_byte(0xFF); // dummy
    SS_PIN = 1;
    return ret;
}