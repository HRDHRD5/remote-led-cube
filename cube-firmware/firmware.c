// Copyright(C) Tomas Uktveris 2015
// www.wzona.info
/*
This firmware.c file was created for a custom-made PCB that uses the STC12C5A60S2 to control an 8x8x8 monochrome LED Matrix.
This is a derivative of the 8x8x8 Firmware by Sliicy, which is based on the original firmware/v2-sdcc/firmware.c by Tomazas.
*/
#include <mcs51/stc12.h>

#define uchar unsigned char
#define uint unsigned int


#define MAX_BUFFER 130 // UART ring buffer size
#define CUBE_WIDTH 8 // Number of leds in one row
#define LED_COUNT CUBE_WIDTH*CUBE_WIDTH // Number of Leds in Cube
#define MAGIC_BYTE 0xf2 // Byte triggering UART
#define UPDATE_CYCLES 50 // Number of cycles to wait at every cube update


__xdata volatile uchar display1[LED_COUNT] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
__xdata volatile uchar display2[LED_COUNT] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

volatile uchar z = 0; // Z-layer being re-painted
volatile uchar y = 0; // Z-layer being re-painted

__xdata volatile uchar *temp;
__xdata volatile uchar *uart_buffer = display1;
__xdata volatile uchar *display_buffer = display2;

volatile int rx_write = -1;

void refresh_screen(__xdata volatile uchar buffer[LED_COUNT]);
void delay(uint i);

// interrupt driven uart with ring buffer
void uart_isr() __interrupt(4)
{
    if (RI) // received a byte
    {
        RI = 0; // Clear receive interrupt flag

        if (rx_write < 0 && SBUF == MAGIC_BYTE)
        {
            rx_write = 0;
        }
        else if (rx_write >= 0)
        {
            uart_buffer[rx_write] = SBUF;
            rx_write += 1;
        }

        if (rx_write >= LED_COUNT)
        {
            // switching between buffers
            temp = uart_buffer;
            uart_buffer = display_buffer;
            display_buffer = temp;
            rx_write = -1;
        }
    }
}

// Some magic wait - as in original code:
volatile inline void delaynop(int a)
{
    while (a--){
        __asm nop __endasm;
    }
}

void delay(uint i)
{
    while (i--)
    {
        delaynop(1);
    }
}

void refresh_screen(__xdata volatile uchar buffer[LED_COUNT])
{
    /*
      P0 range: 255 = all off; 0 = all on; 254 = rightmost on; 127 = leftmost on (Y)
      P1 range: 255 = all off; 0 = all on; 254 = bottommost on; 127 = topmost on (Z)
      P2 range: 1 = frontmost on; 128 = backmost on (X)
    */
    P1 = 0xFF;
    P0 = 0xFF;

    // P2 controls the X-Axis:
    P2 = buffer[(z*CUBE_WIDTH) + y];

    // P1 controls the Z-Axis:
    P1 = (0xFF ^ (0x1 << z));

    // P0 controls the Y-Axis:
    P0 = (0xFF ^ (0x1 << y));

    // Longer the delay = brighter the LEDs shine:
    delay(UPDATE_CYCLES);

    P1 = 0xFF;
    P0 = 0xFF;

    y += 1;
    if (y % CUBE_WIDTH == 0)
    {
        y = 0;
        z = (z + 1) % CUBE_WIDTH; // Rewind and draw for each vertical layer.
    }
}

void main()
{
    __bit uart_detected = 0;
    __bit frame_started = 0;

    __bit paint_last_frame = 0; // Controls whether or not to paint the last frame (this should happen once every refresh).

    // init uart - 19200bps@24.000MHz MCU
    PCON &= 0x7F; // Baudrate no doubled
    SCON = 0x50;  // 8bit and variable baudrate, 1 stop __bit, no parity
    AUXR |= 0x04; // BRT's clock is Fosc (1T)
    BRT = 0xD9;   // Set BRT's reload value
    AUXR |= 0x01; // Use BRT as baudrate generator
    AUXR |= 0x10; // BRT running

    ES = 1; // enable UART interrupt

    // setup timer0
    //TMOD &= 0xE7; // setting timer mode bits
    //reset_timer();
    //TR0 = 1; // start timer0
    //TF0 = 0; // Clear Timer 0 overflow flag
    //ET0 = 1; // enable timer0 interrupt
    EA = 1; // enable global interrupts


    while(1)
    {
        refresh_screen(display_buffer);
    }
}
