/*
 * File:   newmain.c
 * Author: Liam
 *
 * Created on April 14, 2017, 4:18 PM
 */

#include <stdint.h>
#include <xc.h>
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
 
#define _XTAL_FREQ 8000000
#define NRF_CSN LATAbits.LATA2 // spi chip select
#define NRF_CE LATAbits.LATA0 // chip enable activates RX or TX mode
#define IRQ LATAbits.LATA1 // interrupt request, goes low when valid address
// and payload received (RX). for (TX), goes low when transmission complete
#define TX_ADDR

// need to define SREG

void configureTX(void);
void configIO(void);
void writeSPI(uint8_t address, uint8_t data);
void transmitData(uint8_t data);

// IRQ is active low. It is asserted when the packet is received and validated 
/*void main(void) {
    
    return;
}*/


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
    writeSPI(0x00, 0b00001010); // write to CONFIG register, RX_DR interrupt
    // active, TX_DS interrupt active, PRIM_RX set to 0 for PTX, PWR_UP high
}

void configIO(void) {
    TRISAbits.TRISA2 = 0;
}

// address is 5 bits, 001 at the beginning
void writeSPI(uint8_t address, uint8_t data) { // executable in power down or
    // standby modes only
    address |= 0b00100000;
    NRF_CSN = 0;
    uint8_t x; // this holds the SREG
    SSPBUF = address; // put data in buffer
    while(!SSP1STATbits.BF); // wait for read byte to come in
    x = SSPBUF; // read to clear buffer
    SSPBUF = data;
    while (!SSP1STATbits.BF); 
    x = SSPBUF;
    NRF_CSN = 1;
    return x;
}

void readSPI(uint8_t address) {
    address &= 0b00011111; // make sure first 3 bits are low for read
    NRF_CSN = 0;
    uint8_t ret_data;
    SSPBUF = address;
    while(!SSP1STATbits.BF); // wait for read byte to come in
    ret_data = SSPBUF;
    SSPBUF = 0xFF; // write dummy
    while(!SSP1STATbits.BF);
    ret_data = SSPBUF;
    NRF_CSN = 1;
    return ret_data;
}


void transmitData(uint8_t data) {
    SPI_write(TX_ADDR, data);
    NRF_CE = 1;
    __delay_us(50);
    NRF_CE = 0;
    while(!IRQ); // wait for transmission to complete
    // pull IRQ high here again by getting SREG and writing interrupt low
    
    
}

void SPI_init(void) {
    SSPCON1bits.SSPEN = 0; // disable spi
    SSPSTAT = 0b01000000; // I don't know where these bits come from
    SSPCON1 = 0b00100010; // or these bits
    SSPCON1bits.SSPEN = 1; // enable spi
}

