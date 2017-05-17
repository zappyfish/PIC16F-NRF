# PIC16F-NRF

Hi all! Interested in cheap, wireless projects? Look no further! This code will get you going on whatever project you are working on.

#Hardware
I picked up 10 nrf24l01+ modules for about $10, so they're about a dollar each. I got PIC16F1503's for $0.90
each, and I got a picKit3 for programming for $25. The nrf's run on 3.3V, so I used 3.3V regulators (shout out to my microcontrollers professor
for giving a couple of them to me). For testing, I used a couple of LEDs, a switch, and a breadboard. 

You should be able to easily find the nrf's and a picKit 3 on amazon (that's where I got them). Here's a link
to where I got the MCUs: https://www.microchipdirect.com/ProductSearch.aspx?keywords=pic16f1503 (scroll down
to the DIP package)

#Software
I used MPLab X IDE to write the code. This code implements the simplest version of communications (I use
the default pipe 0 address, only send one byte, etc.) That being said, the hardest part of this was 
communicating with and configuring the nrf's over SPI. It probably shouldn't have been as difficult as it
ended up being for me, but nonetheless it was annoying. 
There is code for sending data when a switch is flipped in transmitter.c and receiving that data and
light up an LED if it's the right data. It's pretty straightforward, but if you have any questions let me know. 
