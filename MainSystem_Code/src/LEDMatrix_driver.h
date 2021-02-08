/*
 * LEDMatrix_driver.h
 *
 * Created: 11/17/2020 18:13:46
 *  Author: Victor Espidol
 */ 


#ifndef LEDMATRIX_DRIVER_H_
#define LEDMATRIX_DRIVER_H_

#include "sam.h"
#include <stdbool.h>

#define MATRIX_HEIGHT 32
#define MATRIX_WIDTH 32

#define MATRIX_TABSPACE 4
#define MATRIX_NPLANES 4 //Bit depth per R,G,B (4 = (2^4)^3 = 4096 colors)
#define MATRIX_NROWS 16

/*------------------COLORS------------------*/
#define COLOR565_BLACK   0x0000
#define	COLOR565_BLUE    0x001F
#define	COLOR565_RED     0xF800
#define	COLOR565_GREEN   0x07E0
#define COLOR565_CYAN    0x07FF
#define COLOR565_MAGENTA 0xF81F
#define COLOR565_YELLOW  0xFFE0
#define COLOR565_WHITE   0xFFFF



/*-------------MY BITS--------------*/
//ALL Within PIOC controller
#define MATRIX_CLK PIO_PER_P6
#define MATRIX_LAT PIO_PER_P7  //When driving the LAT drive OE with same signal
#define MATRIX_OE PIO_PER_P18
#define MATRIX_A PIO_PER_P19
#define MATRIX_B PIO_PER_P20
#define MATRIX_C PIO_PER_P21
#define MATRIX_D PIO_PER_P22
#define MATRIX_R1 PIO_PER_P0 
#define MATRIX_G1 PIO_PER_P1
#define MATRIX_B1 PIO_PER_P2
#define MATRIX_R2 PIO_PER_P3
#define MATRIX_G2 PIO_PER_P4
#define MATRIX_B2 PIO_PER_P5


uint16_t _matrix_width, _matrix_height;
uint8_t matrix_rotation;

#define swap(a, b) { short t = a; a = b; b = t; }
#define pgm_read_byte(addr) (*(const unsigned char *)(addr))
#define pgm_read_word(addr) (*(const unsigned short *)(addr))

#ifndef false
#define false 0
#endif
#ifndef true
#define true 1
#endif

void matrix_init(bool dualbuffers);
void matrix_drawPixel(uint16_t x, uint16_t y, uint16_t c);
void matrix_fillScreen(uint16_t c);
void updateDisplay();
void matrix_swapBuffers(bool copy);
//void matrix_dumpMatrix();

uint16_t *matrix_backBuffer();

uint16_t matrix_colorHSV(long hue, uint8_t sat, uint8_t val, bool gflag);
uint16_t matrix_color333(uint8_t r, uint8_t g, uint8_t b);
uint16_t matrix_color444(uint8_t r, uint8_t g, uint8_t b);
uint16_t matrix_color888(uint8_t r, uint8_t g, uint8_t b, bool gflag);

void matrix_setRotation(unsigned char x);
void matrix_drawFastVLine(short x, short y, short h, unsigned short c);
void matrix_drawFastHLine(short x, short y, short w, unsigned short c);
void matrix_drawLine(short x0, short y0, short x1, short y1, unsigned short color);
void matrix_drawRect(short x, short y, short w, short h, unsigned short color);
void matrix_fillRect(short x, short y, short w, short h, unsigned short color);


void matrix_drawChar(short x, short y, unsigned char c, unsigned short color, unsigned short bg, unsigned char size);
//void matrix_draw3x5Char(short x, short y, unsigned char c, unsigned short color, unsigned short bg, unsigned char size);
void matrix_setCursor(short x, short y);

void matrix_write(unsigned char c);
void matrix_writeString(const char* str);    // This is the function to use to write a string
//void matrix_write3x5(unsigned char c);
//void matrix_write3x5String(const char* str);
void matrix_setTextColor(unsigned short c);
//void matrix_setTextColor2(unsigned short c, unsigned short bg);
void matrix_setTextSize(unsigned char s);
void matrix_setTextWrap(char w);

#endif /* LEDMATRIX_DRIVER_H_ */