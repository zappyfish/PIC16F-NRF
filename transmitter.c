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
 
#define _XTAL_FREQ 16000000
#define NRF_CSN LATCbits.LATC4 // spi chip select
#define NRF_CE LATCbits.LATC3 // chip enable activates RX or TX mode
#define IRQ PORTAbits.RA1 // interrupt request, goes low when valid address
#define SWITCH PORTAbits.RA5 // input
#define SCK LATCbits.LATC0 //output
#define SDO LATCbits.LATC2 // output
#define SDI LATCbits.LATC1 // input
// and payload received (RX). for (TX), goes low when transmission complete
const uint8_t TX_ADDR[] = { 0x0B, 0x3B, 0x4B, 0x5B, 0x60};
const uint8_t W_TX_PAYLOAD = 0b10100000;
const uint8_t WRITE_MASK = 0b00100000; // or this w/ address
const uint8_t READ_MASK = 0b00011111; // and this w/ address

// need to define SREG

void SPI_init(void);
void configureTX(void);
void configIOTransmitter(void);
uint8_t writeSPI(uint8_t data);
uint8_t readSPI(uint8_t address);
void transmitData(uint8_t data);
void blink(void);

// IRQ is active low. It is asserted when the packet is received and validated 
/*void main(void) {
    
    return;
}*/

int main(void) {
    NRF_CSN = 1;
    uint8_t last = 1;
    SPI_init();
    LATAbits.LATA0 = 0;
    configureTX();
    while(1) {
        NRF_CSN = 0;
        uint8_t sreg = writeSPI(0xFF);
        NRF_CSN = 1;
        NRF_CSN = 0;
        writeSPI(0x00 & READ_MASK);
        uint8_t data = writeSPI(0xFF);
        NRF_CSN = 1;
        if(data == 0b01011010) {
           LATAbits.LATA0 = !PORTAbits.RA0; 
           blink();
        }
        if(last!= SWITCH) {
        last = SWITCH;
        transmitData(0xFF);     
        }
        
    }
    
}
void configIOTransmitter(void) {
    TRISA = 0b11101110; // light output, 
    TRISC = 0b11100010;
    OPTION_REGbits.nWPUEN = 0; // enable pull ups
    WPUAbits.WPUA5 = 1; // pull up for switch
    WPUAbits.WPUA1 = 1; // enable pull up on IRQ
    
    
}
void blink(void) {
    LATAbits.LATA0 = 1;
    __delay_ms(20);
    LATAbits.LATA0 = 0;
    __delay_ms(20);
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
    NRF_CSN = 0;
    writeSPI(0x00 | WRITE_MASK);
    writeSPI(0b01011010);// write to config register. TX_DS IRQ active
    NRF_CSN = 1;
    NRF_CSN = 0;
    writeSPI(0x01 | WRITE_MASK);
    writeSPI(0b00000001); // auto acknowledgment on pipe 0
    NRF_CSN = 1;
    NRF_CSN = 0;
    writeSPI(0x02 | WRITE_MASK);
    writeSPI(0b00000001); // enable data pipe 0
    NRF_CSN = 1;
    NRF_CSN = 0;
    writeSPI(0x11 | WRITE_MASK);
    writeSPI(0b00000001);
    NRF_CSN = 1;
    NRF_CSN = 0;
    writeSPI(0x03 | WRITE_MASK);
    writeSPI(0b00000011);
    NRF_CSN = 1;
    
    NRF_CSN = 0;
    writeSPI(0x10 | WRITE_MASK); // write tx addr
    for(uint8_t i = 0; i < 5; ++i) {
        writeSPI(0xE7);
    }
    NRF_CSN = 1;
    
    NRF_CSN = 0;
    writeSPI(0x0A | WRITE_MASK); // write rx_addr_p0
    for(uint8_t i = 0; i < 5; ++i) {
        writeSPI(TX_ADDR[i]);
    }
    NRF_CSN = 1;
    /*
    NRF_CSN = 0;
    writeSPI(0x0F | WRITE_MASK);
    writeSPI(TX_ADDR[0]);
    
    NRF_CSN = 0;
    writeSPI(0x16 | WRITE_MASK); // 1 byte width in pipe 5
    writeSPI(0b00000001);
    NRF_CSN = 1;
    */
}



// address is 5 bits, 001 at the beginning
uint8_t writeSPI(uint8_t data) { // executable in power down or
    // standby modes only
    uint8_t x; // this holds the SREG
    SSP1BUF = data; // put data in buffer
    while(!PIR1bits.SSP1IF); // wait for read byte to come in
    PIR1bits.SSP1IF = 0;
    x = SSP1BUF; // read to clear buffer
    return x;
    
}

/*
uint8_t readSPI(uint8_t address) {
    address &= 0b00011111; // make sure first 3 bits are low for read
    NRF_CSN = 0;
    uint8_t ret_data;
    SSPBUF = address;
    while(!SSP1STATbits.BF); // wait for read byte to come in
    ret_data = SSPBUF;
    SSP1STATbits.BF = 0;
    NRF_CSN = 1;
    return ret_data;
}

*/
void transmitData(uint8_t data) {
    NRF_CSN = 0;
    writeSPI(W_TX_PAYLOAD);
    writeSPI(data);
    NRF_CSN = 1;
    NRF_CE = 1;
    __delay_us(50); // pulse to send data   
    //while(IRQ);
    NRF_CSN = 0;
    uint8_t sreg = writeSPI(0xFF);
    NRF_CSN = 1;
    while(!(sreg & 0b00100000)) {
        NRF_CSN = 0;
        sreg = writeSPI(0xFF);
        NRF_CSN = 1;
    }
    NRF_CE = 0;
    NRF_CSN = 0;
    uint8_t sreg = writeSPI(0x07 | WRITE_MASK);
    writeSPI((sreg | 0b01110000)); // clear interrupt
    NRF_CSN = 1;
    // pull IRQ high here again by getting SREG and writing interrupt low
    
    
}

void SPI_init(void) {
    SSP1CON1bits.SSPEN = 0;
    configIOTransmitter();
    SSP1CON1bits.CKP = 0; // clock polarity low
    SSP1STATbits.CKE = 1; // transmit high to low
    SSP1STATbits.SMP = 1; // input data sampled at end of data output time
    SSP1CON1bits.SSPM = 0; // SPI master mode, SCK = FOSC/4
    SSP1CON1bits.SSPEN = 1; // enable spi
    
    
    /*
    SSP1CON1 = 0x00;
    SSP1STAT = 0b11000000;
    SSP1CON1 = 0b00100010;
    PIR1bits.SSP1IF = 0;
    PIE1bits.SSP1IE = 1; // enable spi interrupts
    */
}

