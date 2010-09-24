//----------------------------------------------------------------------------------------
// Oled.c SSD1325
//
// Copyright (c) 2010, Bjoern Heller <tec@hellercom.de>. All rights reserved
// This code is licensed under GNU/ GPL
//----------------------------------------------------------------------------------------
#include "Oled.h"

//----------------------------------------------------------------------------------------
#define BYTE unsigned char


//----------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------
// OLED-Display initialisieren SSD1325
//----------------------------------------------------------------------------------------
void OledInit(void) 
{
  write_cmd(0x15);           // Set Column Address
  write_cmd(0x07);           // Begin  7 Offset!
  write_cmd(0x39);           // End   57 

  write_cmd(0x75);           // Set Row Address
  write_cmd(0x00);           // Begin  0
  write_cmd(0x4F);           // End   79

  write_cmd(0x86);           // Set Current Range 84h:Quarter, 85h:Half, 86h:Full
  write_cmd(0x81);           // Set Contrast Control
  write_cmd(0x2C);
  write_cmd(0xBE);           // Set VCOMH Voltage
  write_cmd(0x00);           // write_cmd(0x11);  --> bei Streifenfehlern ?

  write_cmd(0xBC);           // Set Precharge Voltage
  write_cmd(0x0F);

  write_cmd(0xA0);           // Set Re-map
  write_cmd(0x41);           //od. 0x43

  write_cmd(0xA6);           // Entire Display OFF, all pixels turns OFF
  write_cmd(0xA8);           // Set Multiplex Ratio
  write_cmd(0x4F);           // multiplex ratio N from 16MUX-80MUX // 80!
  write_cmd(0xB1);           // Set Phase Length
  write_cmd(0x22);
  write_cmd(0xB2);           // Set Row Period
  write_cmd(0x46);
  write_cmd(0xB0);           // Set Pre-charge Compensation Enable
  write_cmd(0x08);
  write_cmd(0xB4);           // Set Pre-charge Compensation Level
  write_cmd(0x00);
  write_cmd(0xB3);           // Set Display Clock
  write_cmd(0xA0);
  write_cmd(0xBF);           // Set Segment Low Voltage (VSL)
  write_cmd(0x0D);

  write_cmd(0xB8);           // Set Gray Scale Table
  write_cmd(0x01);
  write_cmd(0x11);
  write_cmd(0x22);
  write_cmd(0x32);
  write_cmd(0x43);
  write_cmd(0x54);
  write_cmd(0x65);
  write_cmd(0x76);
                             
  write_cmd(0xAE);           // Set Display OFF
  write_cmd(0xAD);           // Set Master Configuration
  write_cmd(0x02);
  _delay_ms(75);              // dannach 75ms Pause!? 7365us
  write_cmd(0xA4);           // Normal Display
  //write_cmd(0xAF);           // Set Display ON
  write_cmd(0xE3);           //NOP

//  OLED_12V_HI;
}


//----------------------------------------------------------------------------------------
// Display on   Datasheet Rev.2.1 Page 29: "8.9 Power ON and OFF sequence"
//----------------------------------------------------------------------------------------
void DisplayOn(void)
{
  DDRC  = 0b00000110;
  DDRB  = 0b00000110;

  OLED_RES_LO;               // Reset
  _delay_us(10);
  OLED_RES_HI;

  OledInit();

  OLED_LOAD_HI;              // 100 Ohm to GND
  OLED_12V_HI;               // Vcc 12V on
  _delay_ms(40);             // wait until VCC stable
  OLED_LOAD_LO;              // disconnect 100 Ohm to GND
  write_cmd(0xAF);           // Set Display ON
  DisplayOnOff = 1;
}


//----------------------------------------------------------------------------------------
// Display off   Datasheet Rev.2.1 Page 29: "8.9 Power ON and OFF sequence"
//----------------------------------------------------------------------------------------
void DisplayOff(void)
{
  write_cmd(0xAE);           // Set Display OFF
  OLED_12V_LO;               // Vcc 12V off
  OLED_LOAD_HI;              // 100 Ohm to GND
  _delay_ms(200);            // Wait until panel discharges completely
  OLED_LOAD_LO;              // disconnect 100 Ohm to GND
  DisplayOnOff = 0;
}


//----------------------------------------------------------------------------------------
// Display loeschen
//----------------------------------------------------------------------------------------
void ClearDisplay(void)
{
unsigned int a;

  write_cmd(0x15);
  write_cmd(0x07);
  write_cmd(0x39);
  write_cmd(0x75);
  write_cmd(0x00);
  write_cmd(0x4F);
  for (a=0; a<(102/2*80); a++) write_dat(0x00);        //clr
}


//----------------------------------------------------------------------------------------
// Display loeschen mit leerem Rechteck
//----------------------------------------------------------------------------------------
void ClearDisplay_fast(void)
{
  write_cmd(0x24);      // Rechteck
  write_cmd(0x07);      // Col Spalte X  0x07
  write_cmd(0x00);      // Row	Zeile Y
  write_cmd(0x39);      // Col Spalte X  0x39
  write_cmd(0x4F);      // Row	Zeile Y  0x4F
  write_cmd(0x00);      // 2 Leerpixel
  _delay_ms(5);         // anpassen! sonst Anzeigefehler vom Displaycontroller
}


//-----------------------------------------------------------------------------
// open a write window StartPoint(xStart,yStart) left up
//                     EndPoint(xEnd,yEnd) right down
//-----------------------------------------------------------------------------
void write_window(int xStart, int yStart, int xEnd, int yEnd)
{
  write_cmd(0x15);           // Set Column Address
  write_cmd(xStart);         // Begin  7
  write_cmd(xEnd);           // End   57 

  write_cmd(0x75);           // Set Row Address
  write_cmd(yStart);         // Begin  0
  write_cmd(yEnd);           // End   79
}


// ------------------------------------------------------------------
//  Bitmaps ans Oled-Display senden
// ------------------------------------------------------------------
void PaintPic(char *Ptr, char column, char row) 
{
unsigned char CharCol_1=0, CharCol_2=0;
unsigned char char_r_y_p12, char_r_y_p34, char_r_y_p56;
unsigned char pz, y,x,z,tmp;
unsigned int pos,bitmode,hoehe,breite;
tmp = column;
pos = 0;

  bitmode = pgm_read_byte(&Ptr[pos++]);
  hoehe = pgm_read_byte(&Ptr[pos++]);
  breite = pgm_read_byte(&Ptr[pos++]);
  if (bitmode==1) 
  {
    for (z=0; z<6; z++) 
    {
      for (x=0; x<42; x++) 
      {
        CharCol_1 = pgm_read_byte(&Ptr[pos++]);
        CharCol_2 = pgm_read_byte(&Ptr[pos++]);
        write_cmd(0x15);
        write_cmd(column+7);
        write_cmd(column+7);
        write_cmd(0x75);
        write_cmd(8*row);
        write_cmd(8*row+7);

        pz = 1;                                                  //pixelzeile
        for (y=0; y<8; y++) 
        {
          char_r_y_p12 = char_r_y_p34 = char_r_y_p56 = 0;
          if (CharCol_1 & pz) { char_r_y_p12 |= 0xF0; }
          if (CharCol_2 & pz) { char_r_y_p12 |= 0x0F; }
          write_dat(char_r_y_p12);
          pz = pz << 1;
        }
        column++;
      } // for < 42
      column=tmp;
      row++;
    } // z 10 Zeilen
  }
  else                       // bitmode: 4
  {
    pos = (hoehe * breite)/2-1+3;
    for (z=0; z<hoehe; z++)  // Zeilen
    {
      write_cmd(0x15);       // Spalte
      write_cmd(0+7);
      write_cmd((breite/2-1)+7);
      write_cmd(0x75);       // Reihe
      write_cmd(z);
      write_cmd(z);
      for (y=0; y<(breite/2); y++)
      {
        tmp = pgm_read_byte(&Ptr[pos--]);
        x = tmp>>4;
        tmp = tmp<<4;
        tmp = tmp+x;
        write_dat(tmp);     
      }
    }
  }
}


// ------------------------------------------------------------------
// Funktion stellt ein Zeichen dar 5x7
// column 00..16 Spalten 17 X
// row     0...9 Zeilen  10 Y Reihe
// ------------------------------------------------------------------
void PaintChar(unsigned char sign, unsigned char column, unsigned char row) 
{
unsigned char CharCol_1, CharCol_2, CharCol_3, CharCol_4, CharCol_5;
unsigned char char_r_y_p12, char_r_y_p34, char_r_y_p56;
unsigned char pz, y;
unsigned int pos;
  
  if ((sign<0x20) || (sign>0x7F)) { sign = 0x20; } 
  pos = 5*(sign-0x20);

  CharCol_1 = pgm_read_byte(&code5x7[pos++]);
  CharCol_2 = pgm_read_byte(&code5x7[pos++]);
  CharCol_3 = pgm_read_byte(&code5x7[pos++]);
  CharCol_4 = pgm_read_byte(&code5x7[pos++]);
  CharCol_5 = pgm_read_byte(&code5x7[pos++]);

  write_cmd(0x15);
  write_cmd(7+3*column);
  write_cmd(7+3*column+2);
  write_cmd(0x75);
  write_cmd(8*row);
  write_cmd(8*row+7);
  
  pz = 1;                                                  //pixelzeile
  for (y=0; y<8; y++) 
  {
    char_r_y_p12 = char_r_y_p34 = char_r_y_p56 = 0;
    if (CharCol_1 & pz) { char_r_y_p12 |= 0xF0; }
    if (CharCol_2 & pz) { char_r_y_p12 |= 0x0F; }
    write_dat(char_r_y_p12);
    if (CharCol_3 & pz) { char_r_y_p34 |= 0xF0; }
    if (CharCol_4 & pz) { char_r_y_p34 |= 0x0F; }
    write_dat(char_r_y_p34);
    if (CharCol_5 & pz) { char_r_y_p56 = 0xF0; }           // 6.Spalte bleibt immer leer
    write_dat(char_r_y_p56);
    pz = pz << 1;
  }
}


//-----------------------------------------------------------------------------
// send a data byte to oled controller
//-----------------------------------------------------------------------------
void write_dat(unsigned char data)
{
  OLED_CS_LO;
  OLED_DC_HI;
  write_byte(data);
  OLED_CS_HI;
OLED_DC_HI;
}


//-----------------------------------------------------------------------------
// send a command byte to oled controller
//-----------------------------------------------------------------------------
void write_cmd(unsigned char command) 
{
  OLED_CS_LO;
  OLED_DC_LO;
  write_byte(command);
OLED_CS_HI;
  OLED_DC_HI;
}


//-----------------------------------------------------------------------------
// send a data or command byte
//-----------------------------------------------------------------------------
void write_byte(unsigned char dat_or_cmd)
{

#if SW_SPI

BYTE i;

  for (i=0; i<8; i++)
  {
    if (dat_or_cmd & 0x80) { OLED_SDIN_HI; } 
    else                   { OLED_SDIN_LO; }
    dat_or_cmd = dat_or_cmd << 1;
    OLED_SCLK_LO;    
    OLED_SCLK_HI;
  }
  OLED_SDIN_LO;

#else 

    UDR0 = dat_or_cmd;
    while ( !( UCSR0A & (1<<TXC0)) );
    UCSR0A |= (1<<TXC0);               // clear TXC0

#endif
}


//-----------------------------------------------------------------------------
// send a string to oled
//
// xPos: 0..16 (17 Spalten a 6 Pixel breit)
// yPos: 0.. 9 (10 Zeilen a 8 Pixel breit)
// Str:  max 17 Zeiche a 
//-----------------------------------------------------------------------------
void WriteString(BYTE xPos, BYTE yPos, char *Str) 
{
BYTE c;

  while ((*Str!=0) && (xPos<17)) 
  { 
    c = *Str;
    PaintChar(c,xPos++,yPos);
    Str++;
  }
}


//----------------------------------------------------------------------------------------
//  3 horizontale Linien
//----------------------------------------------------------------------------------------
void Paint_Line(void)
{
BYTE y;
  for (y=3; y<10; y+=3)	    // 3 6 9 
  {
    write_cmd(0x24);         // Rechteck
    write_cmd(0x07);         // Col Spalte X	 0x07
    write_cmd(8*y-14-1);     // Row	Zeile Y
    write_cmd(0x39);         // Col Spalte X	 0x39
    write_cmd(8*y-14-1);     // Row	Zeile Y		 0x4F
    write_cmd(0xFF);
  }
}


//----------------------------------------------------------------------------------------
//                      E n d   o f   O l e d . c 
//----------------------------------------------------------------------------------------
