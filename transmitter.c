/*
 * File:   newmain.c
 * Author: Liam
 *
 * Created on April 14, 2017, 4:18 PM
 */

#include <stdint.h>
#include <xc.h>
#include "spi.h"
#include "NRF_consts.h"

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
 
#define _XTAL_FREQ 16000000UL



void configureTX(void);
void transmitData(uint8_t data);
void blink(void);
void bigBlink(uint8_t x);

// IRQ is active low. It is asserted when the packet is received and validated 
/*void main(void) {
    
    return;
}*/

int main(void) {
    uint8_t last = 1;
    SPI_init();
    LED = 0;
    configureTX();
    while(1) {
        if(SWITCH != last) {
            last = SWITCH;
            LED = last;
            transmitData(0xCF);
        }
        if(SPI_read_byte(0x00) == 0b01011010) {
            bigBlink(5);
        }
    }
    
}

void blink(void) {
    LED = 1;
    __delay_ms(10);
    LED = 0;
    __delay_ms(10);
}
void bigBlink(uint8_t x) {
    LED = 1;
    for(uint8_t i = 0; i<x; ++i) {
        __delay_ms(10);
    }
    LED = 0;
    for(uint8_t i = 0; i<x; ++i) {
        __delay_ms(10);
    }
    
}
void configureTX(void) {
    // 1. need to pull PRIM_RX bit low
    // 2. clock in address for receiving node (TX_ADDR) and payload data
    // (TX_PLD) via SPI. Must write TX_PLD continuously while holding CSN low.
    // TX_ADDR does not have to be rewritten if unchanged from last transmit!
    // If PTX device shall receive acknowledge, data pipe 0 has to be configured
    // to receive the acknowledgment
    // 3. A high pulse of CE starts the transmission. Minimum pulse width of
    // 10 microseconds
    // 4. stuff happens
    // 5. auto acknoledgment happens and TX_DS in the status register is
    // set high. IRQ will be active when TX_DS or MAX_RT is set high. to turn
    // of IRQ, interrupt source must be reset by writing to SREG
    NRF_CE = 0;
    __delay_ms(1);
    SS_PIN = 1;
    __delay_ms(10);
    uint8_t write[2];
    write[0] = (CONFIG & REGISTER_MASK) | W_MASK;
    write[1] = 0b01011010;  // config stuff
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



void transmitData(uint8_t data) {
    uint8_t write[2];
    write[0]= W_TX_PAYLOAD; 
    write[1] = data;// command and data. single byte data
    SPI_writeArray(write, 2);
    NRF_CE = 1;
    __delay_us(50);
    NRF_CE = 0;
    while(IRQ);
}



