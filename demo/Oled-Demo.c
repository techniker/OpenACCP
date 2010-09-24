//--------------------------------------------------------------------------------------
// ATmega168PV 8MHz - OLED-Demo SSD1325  (im Geraet lauffaehig)
// Copyright (c) 2010, Bjoern Heller <tec@hellercom.de>. All rights reserved
// This code is licensed under GNU/ GPL
//
// - fuer Tastennutzung unbedingt den Atmel QFP100 ausbauen
//   und "#define QFP100 0"
//
// - die Fuse-Bits zur Vollstaendigkeit: !CKDIV8 unprogrammed!
//       efuse = 0x01
//       hfuse = 0xDE
//       lfuse = 0xC2
//
//--------------------------------------------------------------------------------------
 
#include <avr/io.h>
#include "Graphic.h"
#include "Font.h"
#include <compat/deprecated.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#define BaudRate 9600UL //19200UL

#define OLED_12V_HI sbi( PORTD, PORTD5) //PORTD.5
#define OLED_12V_LO cbi( PORTD, PORTD5) //PORTD.5

#define OLED_SDIN_HI  sbi( PORTD, PORTD1) //PORTD:xxxx.xxIx
#define OLED_SDIN_LO  cbi( PORTD, PORTD1);

#define OLED_SCLK_HI  sbi( PORTD, PORTD4); //PORTD:xxxx.xxIx
#define OLED_SCLK_LO  cbi( PORTD, PORTD4);

#define OLED_DC_HI  sbi( PORTC, PORTC1); //PORTD:xxxx.xxIx
#define OLED_DC_LO  cbi( PORTC, PORTC1);

#define OLED_RES_HI  sbi( PORTC, PORTC2); //PORTD:xxxx.xxIx
#define OLED_RES_LO  cbi( PORTC, PORTC2);

#define OLED_CS_HI  sbi( PORTB, PORTB1); //PORTD:xxxx.xxIx
#define OLED_CS_LO  cbi( PORTB, PORTB1);


#define BYTE unsigned char
//-----------------------------------------------------------------------------

#define QFP100 1 // 1: ATMEL QFP100 noch auf dem Board, also keine Tastennutzung!
                 // 0: ATMEL QFP100 entfernt,  Tastennutzung moeglich
//-----------------------------------------------------------------------------

void write_byte(unsigned char dat_or_cmd);
void write_cmd(unsigned char command);
void write_dat(unsigned char data);

void delay_ms(int ms);
void delay(unsigned char);
void OledInit(void);
void ClearDisplay();
void PaintChar(unsigned char sign, unsigned char column, unsigned char row);
void PaintPic(BYTE *Ptr, unsigned char column, unsigned char row);
void WriteString(BYTE xPos, BYTE yPos, BYTE *Str);
void Demo(void);
//void uart_init (void);

//-----------------------------------------------------------------------------

int main(void)
{
  DDRB  = 0b00000010;
  PORTB = 0b00000000;
    	
  DDRC  = 0b00000110;
  PORTC = 0b00110000;
  
  DDRD =  0b00110010;
  PORTD = 0b00001000;
  
//  uart_init();

  delay_ms(100);
  OledInit();

  OLED_12V_HI;
  delay_ms(100);
  ClearDisplay();
  
  while(1)                   // Endlosschleife!
  {
    if (QFP100)
	{  
      WriteString(0,5,"    Autostart!     ");
	  delay_ms(2000);
	  Demo();
	}
	else
	{
      WriteString(0,5,"     Taste !?      ");
      while ((PINC & (1<<PINC4)) && (PIND & (1<<PIND3)) && (PINC & (1<<PINC5)));
      //while ((Taste_links==1) && (Taste_mitte==1) && (Taste_rechts==1));
      Demo();
	}
  }  
  return 0;
}


//----------------------------------------------------------------------------------------
//  Demo
//----------------------------------------------------------------------------------------
void Demo(void) 
{
  unsigned char i;
  int y;

  ClearDisplay();

  write_cmd(0x15);      // Spalte
  write_cmd(0x07);      //
  write_cmd(0x39);      //
  write_cmd(0x75);      // Zeile
  write_cmd(0x00);
  write_cmd(0x4F);
  for (y=0; y<4080; y++) { write_dat(0xF0); }
  delay_ms(2000);  
  write_cmd(0xA7); //A7h = Inverse Display
  delay_ms(1000);
    
  write_cmd(0xA4); 
  delay_ms(1000);
  write_cmd(0x15);      // Spalte
  write_cmd(0x07);      //
  write_cmd(0x39);      //
  write_cmd(0x75);      // Zeile
  write_cmd(0x00);
  write_cmd(0x4F);
  for (y=0; y<4080; y++) { write_dat(0xFF); }    
  delay_ms(3000);
    
  WriteString(0,0,"*****************");
  WriteString(0,1,"* 102x80 Pixel  *");
  WriteString(0,2,"*               *");
  WriteString(0,3,"*  - SSD1325 -  *");
  WriteString(0,4,"*               *");
  WriteString(0,5,"* 16 Graustufen *");
  WriteString(0,6,"*               *");
  WriteString(0,7,"* 0123456789ABC *");
  WriteString(0,8,"*               *");
  WriteString(0,9,"*****************");
  delay_ms(5000);

  ClearDisplay();

  PaintPic(Schiff,5,2); // Schiff  

  write_cmd(0x15);      // Spalte
  write_cmd(0x07);      //
  write_cmd(0x07);      //
  write_cmd(0x75);      // Zeile
  write_cmd(0x00);
  write_cmd(0x4F);
  for (i=0; i<80; i++) { write_dat(0xF0); }

  write_cmd(0x15);      // Spalte
  write_cmd(0x39);      //
  write_cmd(0x39);      //
  write_cmd(0x75);      // Zeile
  write_cmd(0x00);
  write_cmd(0x4F);
  for (i=0; i<80; i++) { write_dat(0x0F); }
  
  write_cmd(0x15);      // Spalte
  write_cmd(0x07);      //
  write_cmd(0x39);      //
  write_cmd(0x75);      // Zeile
  write_cmd(0x00);
  write_cmd(0x00);
  for (i=0; i<51; i++) { write_dat(0xFF); }
     
  write_cmd(0x15);      // Spalte
  write_cmd(0x07);      //
  write_cmd(0x39);      //
  write_cmd(0x75);      // Zeile
  write_cmd(0x4F);
  write_cmd(0x4F);
  for (i=0; i<51; i++) { write_dat(0xFF); }

  delay_ms(3000);

  ClearDisplay();
  PaintPic(QRCode,3,0);  // QRCode
  delay_ms(5000);

  ClearDisplay();
  PaintPic(Python,3,0);  // Python Foot
  delay_ms(5000);

  ClearDisplay();
  for(i=0;i<5;i++) 
  {
    PaintPic(Eye_01,3,0);
    delay_ms(100);
    PaintPic(Eye_02,3,0);
    delay_ms(100);
    PaintPic(Eye_03,3,0);
    delay_ms(100);  
    PaintPic(Eye_04,3,0);
    delay_ms(100);
    PaintPic(Eye_05,3,0);
    delay_ms(100);
    PaintPic(Eye_04,3,0);
    delay_ms(100);
    PaintPic(Eye_03,3,0);
    delay_ms(100);
    PaintPic(Eye_02,3,0);
    delay_ms(100);
    PaintPic(Eye_01,3,0);
    delay_ms(1000);
  }

  ClearDisplay();
  WriteString(0,5,"        5        ");
  delay_ms(1000);
  WriteString(0,5,"        4        ");
  delay_ms(1000);
  WriteString(0,5,"        3        ");
  delay_ms(1000);
  WriteString(0,5,"        2        ");
  delay_ms(1000);
  WriteString(0,5,"        1        ");
  delay_ms(1000);
  WriteString(0,5,"      Ende!      ");
  delay_ms(2000);
  WriteString(0,5,"                 ");
  
  for (i=0;i<5;i++) 
  {
    WriteString(0,0,"$ _");
    delay_ms(600);
    WriteString(0,0,"$  ");
    delay_ms(600);
  }
  ClearDisplay();
}


//----------------------------------------------------------------------------------------
// OLED-Display initialisieren SSD1325
//----------------------------------------------------------------------------------------
void OledInit(void) 
{
  OLED_RES_LO;
  delay(10);
  OLED_RES_HI;    
  delay(200);
  //OLED_12V_HI;

  write_cmd(0x15);           // Set Column Address
  write_cmd(0x07);           // Begin  7 Offset!
  write_cmd(0x39);           // End   57 

  write_cmd(0x75);           // Set Row Address
  write_cmd(0x00);           // Begin  0
  write_cmd(0x4F);           // End   79

  write_cmd(0x86);           // Set Current Range 84h:Quarter, 85h:Half, 86h:Full
  write_cmd(0x81);           // Set Contrast Control
  write_cmd(0x2D);
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
                             
  write_cmd(0xAE);           // Set Display ON
  write_cmd(0xAD);           // Set Master Configuration
  write_cmd(0x02);
  delay_ms(75);              // dannach 75ms Pause!? 7365us
  write_cmd(0xA4);           // Normal Display
  write_cmd(0xAF);           // Set Display OFF
  write_cmd(0xE3);           //NOP
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
  for (a=0;a<(102/2*80);a++) write_dat(0x00);        //clr
}


// ------------------------------------------------------------------
//  Bitmaps ans Oled-Display senden
// ------------------------------------------------------------------
void PaintPic(BYTE *Ptr, unsigned char column, unsigned char row) 
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
// column 00..16 Spalten 17
// row     0...9 Zeilen  10
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
}


//-----------------------------------------------------------------------------
// send a command byte to oled controller
//-----------------------------------------------------------------------------
void write_cmd(unsigned char command) 
{
  OLED_CS_LO;
  OLED_DC_LO;
  write_byte(command);
  OLED_DC_HI;
}


//-----------------------------------------------------------------------------
// send a data or command byte
//-----------------------------------------------------------------------------
void write_byte(unsigned char dat_or_cmd)
{
  int i;

  for(i=0; i<8; i++)
  {
    if (dat_or_cmd & 0x80) { OLED_SDIN_HI; } 
    else                   { OLED_SDIN_LO; }
    dat_or_cmd = dat_or_cmd << 1;
    OLED_SCLK_LO;    
    OLED_SCLK_HI;
  }
  OLED_SDIN_LO;
}


//-----------------------------------------------------------------------------
// send a string to oled
//
// xPos: 0..16 (17 Spalten a 6 Pixel breit)
// yPos: 0.. 9 (10 Zeilen a 8 Pixel breit)
// Str:  max 17 Zeiche a 
//-----------------------------------------------------------------------------
void WriteString(BYTE xPos, BYTE yPos, BYTE *Str) 
{
  BYTE c,i;

  i = xPos;
  while ((*Str!=0) && (i<17)) 
  { 
    c = *Str;
    PaintChar(c,i++,yPos);
    Str++;
  }
}


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void delay_ms(int ms)
{
  int t;
  for(t=0; t<=ms; t++)
  _delay_ms(1);
}


//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void delay(unsigned char d)
{
  unsigned char i, j;
  for (i=0; i<d; i++)
	for (j=0; j<255; j++);
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
