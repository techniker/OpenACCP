// Copyright (c) 2010, Bjoern Heller <tec@hellercom.de>. All rights reserved
// This code is licensed under GNU/ GPL

#ifndef OLED_H
#define OLED_H

//--------------------------------------------------------------------------------------
#include <avr/io.h>
#include <compat/deprecated.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "Font.h"

#define BYTE unsigned char
#define WORD unsigned short


#define OLED_12V_HI sbi( PORTD, PORTD5);    // Vcc 12V
#define OLED_12V_LO cbi( PORTD, PORTD5);

#define OLED_SDIN_HI  sbi( PORTD, PORTD1);  // Data
#define OLED_SDIN_LO  cbi( PORTD, PORTD1);

#define OLED_SCLK_HI  sbi( PORTD, PORTD4);  // Clock
#define OLED_SCLK_LO  cbi( PORTD, PORTD4);

#define OLED_DC_HI  sbi( PORTC, PORTC1);    // Data/Command
#define OLED_DC_LO  cbi( PORTC, PORTC1);

#define OLED_RES_HI  sbi( PORTC, PORTC2);   // Reset
#define OLED_RES_LO  cbi( PORTC, PORTC2);

#define OLED_CS_HI  sbi( PORTB, PORTB1);    // ChipSelect
#define OLED_CS_LO  cbi( PORTB, PORTB1);

#define OLED_LOAD_HI sbi( PORTB, PORTB0);   // connect 100 Ohm to GND
#define OLED_LOAD_LO cbi( PORTB, PORTB0);   // disconnect 100 Ohm to GND

//--------------------------------------------------------------------------------------
void write_byte(unsigned char dat_or_cmd);
void write_cmd(unsigned char command);
void write_dat(unsigned char data);
void OledInit(void);
void DisplayOn(void);
void DisplayOff(void);
void ClearDisplay();
void ClearDisplay_fast(void);
void write_window(int xStart, int yStart, int xEnd, int yEnd);
void PaintChar(unsigned char sign, unsigned char column, unsigned char row);
void PaintPic(char *Ptr, char column, char row);
void WriteString(unsigned char xPos, unsigned char yPos, char *Str);

//--------------------------------------------------------------------------------------
volatile BYTE DisplayOnOff;

//--------------------------------------------------------------------------------------
#endif

