/*
 * File:   receiver.c
 * Author: Liam
 *
 * Created on April 14, 2017, 4:31 PM
 */

#include <stdint.h>
#include <xc.h>
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



#define _XTAL_FREQ 8000000
#define NRF_CSN LATCbits.LATC4 // spi chip select
#define NRF_CE LATCbits.LATC3 // chip enable activates RX or TX mode
#define IRQ LATAbits.LATA1 // interrupt request, goes low when valid address
// and payload received (RX). for (TX), goes low when transmission complete
const uint8_t R_RX_PAYLOAD = 0b01100001;
const uint8_t RX_ADDR_P5[] = { 0x0B, 0x3B, 0x4B, 0x5B, 0x60};

void configIOReceiver(void);
void configureRX(void);
void writeSPI(uint8_t address, uint8_t data); // for setting bits
uint8_t readSPI(uint8_t address);
uint8_t getData(void);
void SPI_init(void);
void blink(int delay);

void main(void) {
    configIOReceiver();
    LATAbits.LATA0 = 0;
    SPI_init();
    configureRX();
    uint8_t last = 0;
    while(1) {
        configureRX();
        uint8_t data = getData();
        uint8_t thingy = readSPI(0x00);
        if(thingy == 0b00111111) {
            blink(100);
        }
        if(data == 0xFF) {
            last = (last ? 0 : 1);
            LATAbits.LATA0 = last;
            blink(20);
        }
    }
}

void configIOReceiver(void) {
    TRISA = 0b11101110;
    TRISC = 0b11100010;
    OPTION_REGbits.nWPUEN = 0; // enable pull ups
    WPUAbits.WPUA1 = 1; // enable pull up on IRQ
    
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
    writeSPI(0x00, 0b00111111); // enable IRQ on RX received
    writeSPI(0x01, 0b00000001); // enable pipe 1
    writeSPI(0x02, 0b00000001); // auto acknowledgment on pipe 1
    writeSPI(0x11, 0b00000001); // width in RX payload is 1
    writeSPI(0x03, 0b00000011); // 5 byte addresses
    NRF_CSN = 0;
    SSPBUF = (0x0A | 0b00100000);
    while(!SSP1STATbits.BF);
    uint8_t dummy = SSPBUF;
    for(uint8_t i = 0; i < 5; ++i) {
        SSPBUF = (0xE7);
        while(!SSP1STATbits.BF);
        dummy = SSPBUF;
    }
    NRF_CSN = 1;
    //writeSPI(0x01, 0b00100000); // enable data pipe 5
    //writeSPI(0x02, 0b00100000); // enable auto acknowledgment pipe 5
    //writeSPI(0x16, 0b00000001); // 1 byte width in pipe 5
    //writeSPI(0x03, 0b00000011); // 5 byte addresses
    /*
    NRF_CSN = 0;
    SSPBUF = (0x0B | 0b00100000); // write ot rx_addr_p5
    uint8_t garbage;
    while(!SSP1STATbits.BF); // wait for read byte to come in
    SSP1STATbits.BF = 0;
    garbage = SSPBUF; // read to clear buffer
    for(uint8_t i = 0; i < 5; ++i) {
        SSPBUF = RX_ADDR_P5[i];
        while(!SSP1STATbits.BF); // wait for read byte to come in
        SSP1STATbits.BF = 0;
        garbage = SSPBUF;
    }
    NRF_CSN = 1;
    NRF_CSN = 0;
    SSPBUF = (0x0F | 0b00100000); // write the unique byte to pipe 5
    while(!SSP1STATbits.BF);
    SSP1STATbits.BF = 0;
    garbage = SSPBUF;
    SSPBUF = RX_ADDR_P5[0];
    while(!SSP1STATbits.BF);
    SSP1STATbits.BF = 0;
    garbage = SSPBUF;
    NRF_CSN = 1;
     */
    __delay_us(130);
    // after 130 microsecs, monitoring begins
}
void blink(int delay) {
    LATAbits.LATA0 = 1;
    __delay_ms(20);
    LATAbits.LATA0 = 0;
    __delay_ms(20);
}
// address is 5 bits, 001 at the beginning
void writeSPI(uint8_t data, uint8_t address) { // executable in power down or
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
    
}

uint8_t getData(void) {
    NRF_CE = 1;
    while(IRQ);
    // now the data is in the RX FIFO
    NRF_CE = 0; // enter standby
    uint8_t data;
    NRF_CSN = 0;
    SSPBUF = R_RX_PAYLOAD;
    while(!SSP1STATbits.BF);
    data = SSPBUF;
    SSPBUF = 0xFF; // dummy byte
    while(!SSP1STATbits.BF);
    data = SSPBUF; // this is the actual data we want from RX FIFO
    NRF_CSN = 1;
    writeSPI(0x07, 0b01110000); // clear irq
    return data;
}

uint8_t readSPI(uint8_t address) {
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

void SPI_init(void) {
    SSPCON1bits.SSPEN = 0; // disable spi
    SSPSTAT = 0b01000000; // I don't know where these bits come from
    SSPCON1 = 0b00100010; // or these bits
    SSPCON1bits.SSPEN = 1; // enable spi
}