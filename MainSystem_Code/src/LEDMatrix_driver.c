/*
 * LEDMatrix_driver.c
 *
 * Created: 11/17/2020 18:13:18
 *  Author: vespi
 */ 

#include "LEDMatrix_driver.h"
#include <stdlib.h>
#include <string.h>
#include "gamma.h"
#include "font_5x7.h"

/*=====================Variable Declarations=======================*/
uint16_t *matrixbuff[2];
volatile bool swapflag;
volatile uint8_t backindex;
volatile int16_t *buffptr;
volatile int row, plane;

unsigned short matrix_cursor_y    = 0;
unsigned short matrix_cursor_x    = 0;
unsigned short matrix_textsize    = 1;
unsigned short matrix_textcolor   = COLOR565_WHITE;
unsigned short matrix_textbgcolor = COLOR565_BLACK;
unsigned short matrix_wrap        = 0;



void matrix_init(bool dualbuffers){

	// Allocate and initialize matrix buffer:
	int buffsize = MATRIX_WIDTH * MATRIX_NROWS * 3;
	int allocsize = (dualbuffers) ? buffsize * 2 : buffsize;
	if (NULL == (matrixbuff[0] = (int16_t *)malloc(allocsize * 2))) return;
	memset(matrixbuff[0], 0, allocsize);
	matrixbuff[1] = (dualbuffers) ? &matrixbuff[0][buffsize] : matrixbuff[0];
	
	backindex = 0;
	buffptr = matrixbuff[1-backindex];
	
	plane = MATRIX_NPLANES - 1;
	row = MATRIX_NROWS - 1;
	_matrix_height = MATRIX_HEIGHT;
	_matrix_width = MATRIX_WIDTH;
	
	//Enable Pins as PIO and as outputs
	REG_PIOC_PER |= (MATRIX_CLK | MATRIX_OE | MATRIX_LAT);
	REG_PIOC_OER |= (MATRIX_CLK | MATRIX_OE | MATRIX_LAT);
	
	REG_PIOC_PER |= (MATRIX_A | MATRIX_B | MATRIX_C | MATRIX_D);
	REG_PIOC_OER |= (MATRIX_A | MATRIX_B | MATRIX_C | MATRIX_D);
	REG_PIOC_CODR |= (MATRIX_A | MATRIX_B | MATRIX_C | MATRIX_D); //Clear A,B,C,D
	
	REG_PIOC_PER |= (MATRIX_R1 | MATRIX_G1 | MATRIX_B1 | MATRIX_R2 | MATRIX_G2 | MATRIX_B2);
	REG_PIOC_OER |= (MATRIX_R1 | MATRIX_G1 | MATRIX_B1 | MATRIX_R2 | MATRIX_G2 | MATRIX_B2);
	REG_PIOC_CODR |= (MATRIX_R1 | MATRIX_G1 | MATRIX_B1 | MATRIX_R2 | MATRIX_G2 | MATRIX_B2);

	//Enable timer with prescaler 1 and max value
	//Setup timer interupts to handle overflow
	//clear interupt flag
}


void matrix_drawPixel(uint16_t x, uint16_t y, uint16_t c){
	uint8_t r, g, b, curr_bit, limit;
	uint16_t *ptr;
	
	if ((x < 0) || (x >= _matrix_width) || (y < 0) || (y >= _matrix_height)) {
		return;
	}
	
	switch (matrix_rotation) {
		case 1:
		swap(x, y);
		x = MATRIX_WIDTH  - 1 - x;
		break;
		case 2:
		x = MATRIX_WIDTH  - 1 - x;
		y = MATRIX_HEIGHT - 1 - y;
		break;
		case 3:
		swap(x, y);
		y = MATRIX_HEIGHT - 1 - y;
		break;
	}
	
	// Adafruit_GFX uses 16-bit color in 5/6/5 format, while matrix needs
	// 4/4/4.  Pluck out relevant bits while separating into R,G,B:
	r =  c >> 12;        // RRRRrggggggbbbbb
	g = (c >>  7) & 0xF; // rrrrrGGGGggbbbbb
	b = (c >>  1) & 0xF; // rrrrrggggggBBBBb
	
	// Loop counter stuff
	curr_bit   = 2;
	limit = 1 << MATRIX_NPLANES;
	
	if (y < MATRIX_NROWS) {
		// Data for the upper half of the display is stored in the lower
		// bits of each byte.
		ptr = &matrixbuff[backindex][y * MATRIX_WIDTH * (MATRIX_NPLANES - 1) + x]; // Base addr
		// Plane 0 is a tricky case -- its data is spread about,
		// stored in least two bits not used by the other planes.
		*ptr &= ~0x700;           // Plane 0 R,G mask out in one op
		if (r & 1) *ptr |=  0x100; // Plane 0 R: 64 bytes ahead, bit 0
		if (g & 1) *ptr |=  0x200; // Plane 0 G: 64 bytes ahead, bit 1
		if (b & 1) *ptr |=  0x400; // Plane 0 B: 32 bytes ahead, bit 0
		// The remaining three image planes are more normal-ish.
		// Data is stored in the high 6 bits so it can be quickly
		// copied to the DATAPORT register w/6 output lines.
		for (; curr_bit < limit; curr_bit <<= 1) {
			*ptr &= ~0x7;            // Mask out R,G,B in one op
			if(r & curr_bit) *ptr |= 0x1; // Plane N R: bit 2
			if(g & curr_bit) *ptr |= 0x2; // Plane N G: bit 3
			if(b & curr_bit) *ptr |= 0x4; // Plane N B: bit 4
			ptr  += MATRIX_WIDTH;                 // Advance to next bit plane
		}
	}
	else {
		// Data for the lower half of the display is stored in the upper
		// bits, except for the plane 0 stuff, using 2 least bits.
		ptr = &matrixbuff[backindex][(y - MATRIX_NROWS) * MATRIX_WIDTH * (MATRIX_NPLANES - 1) + x];
		*ptr &= ~0xe000;           // Plane 0 R,G,B mask out in one op
		if(r & 1) *ptr |=  0x2000; // Plane 0 R:
		if(g & 1) *ptr |=  0x4000; // Plane 0 G:
		if(b & 1) *ptr |=  0x8000; // Plane 0 B:
		for(; curr_bit < limit; curr_bit <<= 1) {
			*ptr &= ~0x38;            // Mask out R,G,B in one op
			if(r & curr_bit) *ptr |= 0x08; // Plane N R: bit 5
			if(g & curr_bit) *ptr |= 0x10; // Plane N G: bit 6
			if(b & curr_bit) *ptr |= 0x20; // Plane N B: bit 7
			ptr  += MATRIX_WIDTH;                 // Advance to next bit plane
		}
	}
}



void matrix_fillScreen(uint16_t c){
	  if((c == 0x0000) || (c == 0xffff)) {
		  // For black or white, all bits in frame buffer will be identically
		  // set or unset (regardless of weird bit packing), so it's OK to just
		  // quickly memset the whole thing:
		  memset(matrixbuff[backindex], c, MATRIX_WIDTH * MATRIX_NROWS * 3 * 2);
	  }
	  else {
		  // Otherwise, need to handle it the long way:
		
		  int i;
		  for(i=0; i<_matrix_width; i++) {
		  	  matrix_drawFastVLine(i, 0, _matrix_height, c);
		  }		  
	  }
}

#define CALLOVERHEAD 30 // actual measured 27
#define LOOPTIME 360 // actual measured 292
void updateDisplay(){
	uint16_t *ptr;
	uint16_t *end_ptr;
	uint16_t i, duration;
	uint16_t dataPins = (MATRIX_R1 | MATRIX_G1 | MATRIX_B1 | MATRIX_R2 | MATRIX_G2 | MATRIX_B2);
	uint16_t r1g1b1 = (MATRIX_R1 | MATRIX_G1 | MATRIX_B1);
	uint16_t r2g2b2 = (MATRIX_R2 | MATRIX_G2 | MATRIX_B2);
	
	
	REG_PIOC_SODR |= (MATRIX_OE | MATRIX_LAT);
	
	duration = ((LOOPTIME + (CALLOVERHEAD * 2)) << plane) - CALLOVERHEAD;
	
	
	if (++plane >= MATRIX_NPLANES) {
		plane = 0;
		
		if (++row >= MATRIX_NROWS) {
			row = 0;
			if (swapflag) {
				backindex = 1 - backindex;
				swapflag = 0;
			}
			buffptr = matrixbuff[1-backindex];
		}
	}
	else if (plane == 1) {
		//Clear A,B,C,D
		REG_PIOC_CODR |= (MATRIX_A | MATRIX_B | MATRIX_C | MATRIX_D);
		
		if (row & 0x1) REG_PIOC_SODR |= MATRIX_A; 
		if (row & 0x2) REG_PIOC_SODR |= MATRIX_B; 
		if (row & 0x4) REG_PIOC_SODR |= MATRIX_C;
		if (row & 0x8) REG_PIOC_SODR |= MATRIX_D;
	}
	
	ptr = (uint16_t *)buffptr;
	end_ptr = ptr + 32;
	
	//WritePeriod2(duration);
	REG_TC0_RC0 = duration;
	//WriteTimer2(0);
	REG_PIOC_CODR |= (MATRIX_OE | MATRIX_LAT);


    if (plane > 0) {
	    for (; ptr< end_ptr; ptr++) {
		    REG_PIOC_CODR |= (MATRIX_R1 | MATRIX_G1 | MATRIX_B1 | MATRIX_R2 | MATRIX_G2 | MATRIX_B2);
		    REG_PIOC_SODR |= (*ptr & dataPins);
		    asm ("nop"); asm ("nop");
			REG_PIOC_SODR |= MATRIX_CLK;
			REG_PIOC_CODR |= MATRIX_CLK;
	    }
	    
	    buffptr = ptr;
    }
    else {
	    for (i=0; i < MATRIX_WIDTH; i++) {
			REG_PIOC_CODR |= (MATRIX_R1 | MATRIX_G1 | MATRIX_B1 | MATRIX_R2 | MATRIX_G2 | MATRIX_B2);
			REG_PIOC_SODR |= (((ptr[i] >> 10) & r2g2b2) | ((ptr[i]) & r1g1b1));
		    asm ("nop"); asm ("nop");
			REG_PIOC_SODR |= MATRIX_CLK;
			REG_PIOC_CODR |= MATRIX_CLK;
	    }
    }
    
    //mT2ClearIntFlag();
	
}

void matrix_swapBuffers(bool copy){
	if (matrixbuff[0] != matrixbuff[1]) {
		// To avoid 'tearing' display, actual swap takes place in the interrupt
		// handler, at the end of a complete screen refresh cycle.
		swapflag = 1;                  // Set flag here, then...
		while(swapflag); // wait for interrupt to clear it
		if(copy) {
			memcpy(matrixbuff[backindex],
			matrixbuff[1-backindex],
			MATRIX_WIDTH * MATRIX_NROWS * 3 * 2);
		}
	}
}

//void matrix_dumpMatrix();

uint16_t *matrix_backBuffer(){
	return matrixbuff[backindex];
}

uint16_t matrix_colorHSV(long hue, uint8_t sat, uint8_t val, bool gflag){
	uint8_t  r, g, b, lo;
	uint16_t s1, v1;
	
	// Hue
	hue %= 1536;              // -1535 to +1535
	if (hue < 0)
		hue += 1536;		//     0 to +1535
	lo = hue & 255;           // Low byte  = primary/secondary color mix
	switch (hue >> 8) {        // High byte = sextant of color wheel
		case 0 : r = 255     ; g =  lo     ; b =   0     ; break; // R to Y
		case 1 : r = 255 - lo; g = 255     ; b =   0     ; break; // Y to G
		case 2 : r =   0     ; g = 255     ; b =  lo     ; break; // G to C
		case 3 : r =   0     ; g = 255 - lo; b = 255     ; break; // C to B
		case 4 : r =  lo     ; g =   0     ; b = 255     ; break; // B to M
		default: r = 255     ; g =   0     ; b = 255 - lo; break; // M to R
	}
	
	// Saturation: add 1 so range is 1 to 256, allowing a quick shift operation
	// on the result rather than a costly divide, while the type upgrade to int
	// avoids repeated type conversions in both directions.
	s1 = sat + 1;
	r  = 255 - (((255 - r) * s1) >> 8);
	g  = 255 - (((255 - g) * s1) >> 8);
	b  = 255 - (((255 - b) * s1) >> 8);
	
	// Value (brightness) & 16-bit color reduction: similar to above, add 1
	// to allow shifts, and upgrade to int makes other conversions implicit.
	v1 = val + 1;
	if (gflag) { // Gamma-corrected color?
		r = gamma[(r * v1) >> 8]; // Gamma correction table maps
		g = gamma[(g * v1) >> 8]; // 8-bit input to 4-bit output
		b = gamma[(b * v1) >> 8];
	}
	else { // linear (uncorrected) color
		r = (r * v1) >> 12; // 4-bit results
		g = (g * v1) >> 12;
		b = (b * v1) >> 12;
	}
	return (r << 12) | ((r & 0x8) << 8) | // 4/4/4 -> 5/6/5
	(g <<  7) | ((g & 0xC) << 3) |
	(b <<  1) | ( b        >> 3);
}


uint16_t matrix_color333(uint8_t r, uint8_t g, uint8_t b){
	// RRRrrGGGgggBBBbb
	return ((r & 0x7) << 13) | ((r & 0x6) << 10) |
	((g & 0x7) <<  8) | ((g & 0x7) <<  5) |
	((b & 0x7) <<  2) | ((b & 0x6) >>  1);
}


uint16_t matrix_color444(uint8_t r, uint8_t g, uint8_t b){
	// RRRRrGGGGggBBBBb
	return ((r & 0xF) << 12) | ((r & 0x8) << 8) |
	((g & 0xF) <<  7) | ((g & 0xC) << 3) |
	((b & 0xF) <<  1) | ((b & 0x8) >> 3);
}

uint16_t matrix_color888(uint8_t r, uint8_t g, uint8_t b, bool gflag){
	if(gflag) { // Gamma-corrected color?
		r = gamma[r]; // Gamma correction table maps
		g = gamma[g]; // 8-bit input to 4-bit output
		b = gamma[b];
		return ((uint16_t)r << 12) | ((uint16_t)(r & 0x8) << 8) | // 4/4/4->5/6/5
		((uint16_t)g <<  7) | ((uint16_t)(g & 0xC) << 3) |
		(          b <<  1) | (           b        >> 3);
	} // else linear (uncorrected) color
	return ((uint16_t)(r & 0xF8) << 8) | ((uint16_t)(g & 0xFC) << 3) | (b >> 3);
}

void matrix_setRotation(unsigned char x){
/* Set display matrix_rotation in 90 degree steps
 * Parameters:
 *      x: dictate direction of matrix_rotation
 *          0 = no matrix_rotation (0 degree matrix_rotation)
 *          1 = rotate 90 degree clockwise
 *          2 = rotate 180 degree
 *          3 = rotate 90 degree anticlockwise
 * Returns: Nothing
 */
    matrix_rotation = (x & 3);
    switch(matrix_rotation) {
        case 0:
        case 2:
            _matrix_width  = MATRIX_HEIGHT;
            _matrix_height = MATRIX_WIDTH;
            break;
        case 1:
        case 3:
            _matrix_width  = MATRIX_HEIGHT;
            _matrix_height = MATRIX_WIDTH;
            break;
    }
}

void matrix_drawFastVLine(short x, short y, short h, unsigned short c){
	if (x >= _matrix_width || y >= _matrix_height) return;
	
	if ((y+h-1) >= _matrix_width) {
		h = _matrix_height-y;
	}
	
	while (h--) {
		matrix_drawPixel(x, y+h, c);
	}
}

void matrix_drawFastHLine(short x, short y, short w, unsigned short c){
	if (x >= _matrix_width || y >= _matrix_height) return;
	
	if ((x+w-1) >= _matrix_width) {
		w = _matrix_width-x;
	}
	
	while (w--) {
		matrix_drawPixel(x+w, y, c);
	}
}



void matrix_drawLine(short x0, short y0, short x1, short y1, unsigned short color){	
/* Draw a straight line from (x0,y0) to (x1,y1) with given color
    * Parameters:
    *      x0: x-coordinate of starting point of line. The x-coordinate of
    *          the top-left of the screen is 0. It increases to the right.
    *      y0: y-coordinate of starting point of line. The y-coordinate of
    *          the top-left of the screen is 0. It increases to the bottom.
    *      x1: x-coordinate of ending point of line. The x-coordinate of
    *          the top-left of the screen is 0. It increases to the right.
    *      y1: y-coordinate of ending point of line. The y-coordinate of
    *          the top-left of the screen is 0. It increases to the bottom.
    *      color: 16-bit color value for line
    */

    short steep = abs(y1 - y0) > abs(x1 - x0);
    if (steep) {
	    swap(x0, y0);
	    swap(x1, y1);
    }

    if (x0 > x1) {
	    swap(x0, x1);
	    swap(y0, y1);
    }

    short dx, dy;
    dx = x1 - x0;
    dy = abs(y1 - y0);

    short err = dx / 2;
    short ystep;

    if (y0 < y1) {
	    ystep = 1;
    }
    else {
	    ystep = -1;
    }

    for (; x0<=x1; x0++) {
	    if (steep) {
		    matrix_drawPixel(y0, x0, color);
	    }
	    else {
		    matrix_drawPixel(x0, y0, color);
	    }
	    err -= dy;
	    if (err < 0) {
		    y0 += ystep;
		    err += dx;
	    }
    }
}



void matrix_drawRect(short x, short y, short w, short h, unsigned short color){
	/* Draw a rectangle outline with top left vertex (x,y), width w
     * and height h at given color
     * Parameters:
     *      x:  x-coordinate of top-left vertex. The x-coordinate of
     *          the top-left of the screen is 0. It increases to the right.
     *      y:  y-coordinate of top-left vertex. The y-coordinate of
     *          the top-left of the screen is 0. It increases to the bottom.
     *      w:  width of the rectangle
     *      h:  height of the rectangle
     *      color:  16-bit color of the rectangle outline
     * Returns: Nothing
     */
    matrix_drawFastHLine(x, y, w, color);
    matrix_drawFastHLine(x, y+h-1, w, color);
    matrix_drawFastVLine(x, y, h, color);
    matrix_drawFastVLine(x+w-1, y, h, color);
}


void matrix_fillRect(short x, short y, short w, short h, unsigned short color){
	/* Draw a filled rectangle with starting top-left vertex (x,y),
    *  width w and height h with given color
    * Parameters:
    *      x:  x-coordinate of top-left vertex; top left of screen is x=0
    *              and x increases to the right
    *      y:  y-coordinate of top-left vertex; top left of screen is y=0
    *              and y increases to the bottom
    *      w:  width of rectangle
    *      h:  height of rectangle
    *      color:  16-bit color value
    * Returns:     Nothing
    */
    int i;
    for(i=x; i<=x+w; i++) {
        matrix_drawFastVLine(i, y, h, color);
    }
}






void matrix_drawChar(short x, short y, unsigned char c, unsigned short color, unsigned short bg, unsigned char size) {
	char i, j;
	if ((x >= _matrix_width)      || // Clip right
	(y >= _matrix_height)     || // Clip bottom
	((x + 6 * size - 1) < 0)  || // Clip left
	((y + 8 * size - 1) < 0)) {  // Clip top
		return;
	}
	
	for (i=0; i<6; i++ ) {
		unsigned char line;
		if (i == 5) {
			line   = 0x0;
		}
		else {
			line = pgm_read_byte(font_5x7+(c*5)+i);
			for ( j = 0; j<8; j++) {
				if (line & 0x1) {
					if (size == 1) {// default size
						matrix_drawPixel(x+i, y+j, color);
					}
					else {  // big size
						matrix_fillRect(x+(i*size), y+(j*size),
						size, size, color);
					}
				}
				else if (bg != color) {
					if (size == 1) { // default size
						matrix_drawPixel(x+i, y+j, bg);
					}
					else {  // big size
						matrix_fillRect(x+i*size, y+j*size, size, size, bg);
					}
				}
				line >>= 1;
			}
		}
	}
}



void matrix_write(unsigned char c) {
	if (c == '\n') {
		matrix_cursor_y += matrix_textsize*8;
		matrix_cursor_x  = 0;
	}
	else if (c == '\r') {
		// skip em
	}
	else if (c == '\t'){
		int new_x = matrix_cursor_x + MATRIX_TABSPACE;
		if (new_x < _matrix_width){
			matrix_cursor_x = new_x;
		}
	}
	else {
		matrix_drawChar(matrix_cursor_x, matrix_cursor_y, c, matrix_textcolor, matrix_textbgcolor, matrix_textsize);
		matrix_cursor_x += matrix_textsize*6;
		if (matrix_wrap && (matrix_cursor_x > (_matrix_width - matrix_textsize*6))) {
			matrix_cursor_y += matrix_textsize*8;
			matrix_cursor_x = 0;
		}
	}
}

inline void matrix_writeString(const char* str){
    /* Print text onto screen
     * Call matrix_setCursor(), matrix_setTextColor(), matrix_setTextSize()
     *  as necessary before printing
     */
    while (*str){
        matrix_write(*str++);
    }
}

inline void matrix_setCursor(short x, short y) {
    /* Set cursor for text to be printed
     * Parameters:
     *      x = x-coordinate of top-left of text starting
     *      y = y-coordinate of top-left of text starting
     * Returns: Nothing
     */
    matrix_cursor_x = x;
    matrix_cursor_y = y;
}

inline void matrix_setTextSize(unsigned char s) {
    /*Set size of text to be displayed
     * Parameters:
     *      s = text size (1 being smallest)
     * Returns: nothing
     */
    matrix_textsize = (s > 0) ? s : 1;
}

inline void matrix_setTextColor(unsigned short c) {
	// For 'transparent' background, we'll set the bg
	// to the same as fg instead of using a flag
	matrix_textcolor = matrix_textbgcolor = c;
}

inline void matrix_setTextWrap(char w) {
	matrix_wrap = w;
}