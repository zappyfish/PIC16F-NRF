/* 
 * File:   spi.h
 * Author: Liam
 *
 * Created on May 16, 2017, 5:34 PM
 */

#ifndef SPI_H
#define	SPI_H

#include <stdint.h>

#ifdef	__cplusplus
extern "C" {
#endif
    
#define SS_PIN LATCbits.LATC3 // this is NRF_CSN
#define SCK LATCbits.LATC0
#define SDO LATCbits.LATC2
#define SDI PORTCbits.RC1
#define SWITCH PORTAbits.RA5 // input
#define LED LATAbits.LATA1
#define NRF_CE LATCbits.LATC4 // chip enable activates RX or TX mode
#define IRQ PORTAbits.RA2 // interrupt request, goes low when valid address
    
#define _XTAL_FREQ 16000000UL
    
    
void configIO(void);
void SPI_init(void);
uint8_t SPI_write_byte(uint8_t data); // need to manually assert SS for this
uint8_t SPI_writeArray(uint8_t *data, uint8_t length);
uint8_t SPI_read_byte(uint8_t address);


#ifdef	__cplusplus
}
#endif

#endif	/* SPI_H */

