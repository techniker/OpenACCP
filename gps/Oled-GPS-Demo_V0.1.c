//--------------------------------------------------------------------------------------
// ATmega168PV 8MHz - GPS-OLED-Demo SSD1325  (im Geraet lauffaehig)
// NMEA-Daten 9600 8N1 an RXD
//
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
//#include "Graphic.h"
#include "Font.h"
#include <compat/deprecated.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <inttypes.h>
#include <avr/interrupt.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#define F_CPU 8000000
#define BaudRate 9600UL //19200UL

#define OLED_12V_HI sbi( PORTD, PORTD5)
#define OLED_12V_LO cbi( PORTD, PORTD5)

#define OLED_SDIN_HI  sbi( PORTD, PORTD1)
#define OLED_SDIN_LO  cbi( PORTD, PORTD1);

#define OLED_SCLK_HI  sbi( PORTD, PORTD4);
#define OLED_SCLK_LO  cbi( PORTD, PORTD4);

#define OLED_DC_HI  sbi( PORTC, PORTC1);
#define OLED_DC_LO  cbi( PORTC, PORTC1);

#define OLED_RES_HI  sbi( PORTC, PORTC2);
#define OLED_RES_LO  cbi( PORTC, PORTC2);

#define OLED_CS_HI  sbi( PORTB, PORTB1);
#define OLED_CS_LO  cbi( PORTB, PORTB1);

//PORTB |= (1<<PB2);  // alt:  sbi(PORTB, PB2);  mit include <compat/deprecated.h>
//PORTC &= ~(1<<PC1); // alt: cbi(PORTC, PC1);

#define BYTE unsigned char
#define WORD unsigned short
#define max 82                         // Puffergroesse
//-----------------------------------------------------------------------------

#define QFP100 0 // 1: ATMEL QFP100 noch auf dem Board, also keine Tastennutzung!
                 // 0: ATMEL QFP100 entfernt,  Tastennutzung moeglich
//-----------------------------------------------------------------------------

void Timer0_Init(void);
void Uart_Init (void);
//ISR(TIMER0_COMPA_vect);
//ISR(USART_RX_vect);

//--sollte in Oled.c
void write_byte(unsigned char dat_or_cmd);
void write_cmd(unsigned char command);
void write_dat(unsigned char data);
void delay_ms(int ms);
void delay(unsigned char);
void OledInit(void);
void DisplayOn(void);
void DisplayOff(void);
void ClearDisplay();
void ClearDisplay_fast(void);
void PaintChar(unsigned char sign, unsigned char column, unsigned char row);
void PaintPic(char *Ptr, char column, char row);
void WriteString(BYTE xPos, BYTE yPos, char *Str);

//--sollte in Screen.c
void DisplayScreen(void);
void Screen00(void);
void Screen01(void);
void Screen02(void);
void Screen03(void);
BYTE CheckKey(void);

//--sollte in GPS.c
void DecodeNMEA(const char *);
void Decode_GPGSV(void);
void Decode_GPGGA(void);
void Decode_GPGSA(void);
void Decode_GPRMC(void);
BYTE NMEA_CSum(const char *ptrRX);
BYTE searchPos1(char *pt, unsigned char anzahlkommas);
void Paint_db_Balken(BYTE, BYTE, BYTE);
void Paint_Line(void);
BYTE isSat_in_usedSat_Tab(BYTE number);

//--

BYTE ScreenMax = 3;			    // Anzahl der Screens
volatile BYTE DisplayOnOff = 1;
volatile BYTE ShutdownCnt = 9;        // 10min nach letztem Tastendruck: Display off / OLED_12V_LO

WORD freemilli=0;
volatile BYTE z001ms=0, z010ms=0, z100ms=0, z01sec=0, z10sec=0, buffer;
volatile BYTE Ovl10ms=0, Ovl100ms=0, Ovl1sec=0, Ovl10sec=0, Ovl1min=0;

volatile char puffer0[max+1];
volatile char puffer1[max+1];
volatile BYTE buff_0or1=0,buff_0or1_old=0;
volatile char *lesezeiger;
volatile char *schreibzeiger;
volatile BYTE GPS_Start=0, GPS_End=0;
BYTE fix,sat01,sat10;
BYTE GPS_day01, GPS_day01old, GPS_day10, GPS_mon01, GPS_mon10, GPS_year01, GPS_year10;
volatile BYTE Tast_cnt, Tast_cnt_old;
volatile BYTE sat_visible, sat_visible_old, sat_used, sat_used_old;
volatile BYTE Tastensperre;
volatile BYTE GPS_NorthSouth, GPS_EastWest, GPS_Alt_Unit;

volatile BYTE usedSat_Tab[12];
char GPS_Lat[10]; char GPS_Lon[10]; char GPS_Alt[5];
//-----------------------------------------------------------------------------
typedef struct {
volatile BYTE sat_id;
volatile BYTE sat_db;
} sat_s;

sat_s sat_tab[16];

struct dop_s { char p[5]; char h[5]; char v[5]; };
struct dop_s dop;

struct date_s { int d; int m; int y; };
struct date_s date;

struct time_s { char h; char m; char s; };
struct time_s time;

//-------------------------------------------------------------------------------------------------
//  - main - main - main - main - main - main - main - main - main - main - main - main - main -
//-------------------------------------------------------------------------------------------------
int main(void)
{
  DDRB  = 0b00000010;
  PORTB = 0b00000000;
    	
  DDRC  = 0b00000110;
  PORTC = 0b00110000;
  
  DDRD =  0b00110010;
  PORTD = 0b00001000;


  // Initialisierung der Zeiger
  lesezeiger=&puffer0[0];
  schreibzeiger=&puffer0[0];
  buff_0or1=buff_0or1_old=0;
  time.h = time.m = time.s = 0;
  GPS_day01=GPS_day01old=GPS_day10=GPS_mon01=GPS_mon10=GPS_year01=GPS_year10='x'; 
  Tast_cnt_old = Tast_cnt;
  sat_visible = sat_visible_old = 0;
  sat_used = sat_used_old = 0;

  cli();
  Uart_Init();
  Timer0_Init();
  sei();

  delay_ms(100);
  OledInit();

  OLED_12V_HI;
  delay_ms(100);
  ClearDisplay();
  
  WriteString(0,5,"  GPS-9600 8N1 ! ");
  delay_ms(1000);
  WriteString(0,5,"                 ");

// ------------------------------------------------------------------------------------------------
// - while(1) -- while(1) -- while(1) -- while(1) -- while(1) -- while(1) -- while(1) -- while(1) -
// ------------------------------------------------------------------------------------------------
  while(1)
  {
    if (buff_0or1_old!=buff_0or1) 
    {
      if (buff_0or1==0) lesezeiger=&puffer1[0];
      else              lesezeiger=&puffer0[0];

      if (NMEA_CSum((const char*)lesezeiger)==1)       // falls Checksumme ok
      {
        DecodeNMEA((const char*)lesezeiger);
        buff_0or1_old = buff_0or1;
        if (Tast_cnt==3) Screen03();      // Test fuer Debugausgaben
        if (Tast_cnt==3) PaintChar(' ',0,6);
      }
      else
      {
        if (Tast_cnt==3) PaintChar(*lesezeiger,0,6);
        buff_0or1_old = buff_0or1;
      }
    }
    // ------------------------------------------------------------------------
    // zeitgesteuerte Funktionen:
    // ------------------------------------------------------------------------
    //-----------------------10ms----------------------------------------------
    if (Ovl10ms)
    {
      Ovl10ms = 0;
      if (CheckKey()) { DisplayScreen(); }
    }
    //-----------------------100ms---------------------------------------------
    if (Ovl100ms)
    {
      Ovl100ms = 0;
    }
    //-----------------------1s------------------------------------------------
    if (Ovl1sec) 
    {
      Ovl1sec = 0;
      DisplayScreen();
    }
    //-----------------------10s-----------------------------------------------
    if (Ovl10sec) 
    {
      Ovl10sec = 0;
    }
    //-----------------------1min----------------------------------------------
    if (Ovl1min) 
    {
      if (ShutdownCnt) { ShutdownCnt--; }
      else             { if (DisplayOnOff) { DisplayOff(); } }
      Ovl1min = 0;
    }
  // --------------------------------------------------------------------------

  }  // --> while(1) {}

  return 0;         // never!
}                   // end of main


//----------------------------------------------------------------------------------------
// Initialisierung USART
//----------------------------------------------------------------------------------------
void Uart_Init (void)
{
  UBRR0H=(F_CPU/16/BaudRate-1) >> 8;
  UBRR0L=(F_CPU/16/BaudRate-1) & 0xFF;

  UCSR0A=0x00;
  UCSR0B=0x90;
  //UCSR0B = (1<<RXCIE0);
  UCSR0C=0x06;
}


//----------------------------------------------------------------------------------------
// Initialisierung Timer0A 1ms
//----------------------------------------------------------------------------------------
void Timer0_Init(void)
{
  TCCR0A = (1<<WGM01); 
  TCCR0B = (1<<CS01)|(1<<CS00);
  OCR0A = 125;
  TIMSK0 |= (1<<OCIE0A);
}


//----------------------------------------------------------------------------------------
// Interrupt USART
//----------------------------------------------------------------------------------------
ISR (USART_RX_vect) 
{
  *schreibzeiger=UDR0;

  if (buff_0or1==buff_0or1_old) 
  {
    if (*schreibzeiger == '\n')
    {
      if (buff_0or1==0) { buff_0or1=1; schreibzeiger=&puffer1[0];}
	     else              { buff_0or1=0; schreibzeiger=&puffer0[0];}
	     GPS_Start=0;
    }

	   if ((*schreibzeiger=='$') || (GPS_Start==1)) 
	   {
	     GPS_Start=1;
      if ((puffer0[0]=='$') && (puffer0[1]!='$') && (buff_0or1==0)) schreibzeiger++;
      if ((puffer1[0]=='$') && (puffer1[1]!='$') && (buff_0or1!=0)) schreibzeiger++;

      if ((buff_0or1==0) && (schreibzeiger>=&puffer0[max])) { GPS_Start=0; }
      if ((buff_0or1==1) && (schreibzeiger>=&puffer1[max])) { GPS_Start=0; }
	   } 
  }
}


//----------------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------------
ISR(TIMER0_COMPA_vect)
{
  freemilli++;
  z001ms++;
  if (z001ms==10) {
    z001ms = 0;
    z010ms++;
    Ovl10ms = 1;
  }
  if (z010ms==10) {
    z010ms = 0;
    z100ms++;
    Ovl100ms = 1;
  }
  if (z100ms==10) {
    z100ms = 0;
    z01sec++;
    Ovl1sec = 1;
  }
  if (z01sec==10) {
    z01sec = 0;
    z10sec++;
    Ovl10sec = 1;
  }
  if (z10sec==6) {
    z10sec = 0;
    Ovl1min = 1;
  }
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
// Display ein
//----------------------------------------------------------------------------------------
void DisplayOn(void)
{
  OLED_12V_HI;
  DisplayOnOff = 1;
}


//----------------------------------------------------------------------------------------
// Display aus
//----------------------------------------------------------------------------------------
void DisplayOff(void)
{
// ShutDown-Befehl an OLED Controller??
  OLED_12V_LO;
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
  write_cmd(0x07);      // Row	Zeile Y
  write_cmd(0x39);      // Col Spalte X  0x39
  write_cmd(0x4F);      // Row	Zeile Y  0x4F
  write_cmd(0x00);      // 2 Leerpixel
  delay_ms(30);         // anpassen! sonst Anzeigefehler vom Displaycontroller
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


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void delay_ms(int ms)
{
int t;
  for (t=0; t<=ms; t++)
  _delay_ms(1);
}


//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void delay(unsigned char d)
{
BYTE i, j;
  for (i=0; i<d; i++)
	for (j=0; j<255; j++);
}


//-------------------------------------------------------------------------------------------------
// eigenes Modul? Screen.c
//-----------------------------------------------------------------------------
// Prueft, ob Taste betaetigt (derzeit nur mittlere Taste!)
//-----------------------------------------------------------------------------
BYTE CheckKey(void)
{
BYTE r = 0;

  if (Tastensperre > 0) { Tastensperre--; }
  else
  {
    if ((PIND & (1<<PIND3))==0)
    {
      ShutdownCnt = 9;								            // 10min Shutdown-Timer aufziehen!
      if (!DisplayOnOff) { DisplayOn(); }

      Tast_cnt++;
      if (Tast_cnt>ScreenMax) { Tast_cnt=0; }
      Tastensperre = 30;                           // 300ms Tastensperre
      if (Tast_cnt != Tast_cnt_old) 
      {
        Tast_cnt_old = Tast_cnt;
        ClearDisplay();      
        r = 1;      
      }
    }  
  }
  return r;
}


//-----------------------------------------------------------------------------
// waehlt einen Screen - je nach Tastendruck
//-----------------------------------------------------------------------------
void DisplayScreen(void)
{
  if (DisplayOnOff) 
  {
    switch (Tast_cnt)
    {
      case 0:  Screen00();  break;
      case 1:  Screen01();  break;
      case 2:  Screen02();  break;
      case 3:  Screen03();  break;
     default:               break;
    }
  }
}


//-----------------------------------------------------------------------------
// Screen00: Startbildschirm...
//-----------------------------------------------------------------------------
void Screen00(void)
{
BYTE zeile, spalte;

  WriteString(0,2," NMEA 9600 8N1 ! ");

  zeile = 5; spalte = 5;
  PaintChar((time.h>>4) + '0',spalte++,zeile);
  PaintChar((time.h & 0x0F) + '0',spalte++,zeile);
  PaintChar(':',spalte++,zeile);
  PaintChar((time.m>>4) + '0',spalte++,zeile);
  PaintChar((time.m & 0x0F) + '0',spalte++,zeile);
  PaintChar('.',spalte++,zeile);    
  PaintChar((time.s>>4) + '0',spalte++,zeile);
  PaintChar((time.s & 0x0F) + '0',spalte++,zeile);

  zeile = 8; spalte = 5;
  PaintChar(GPS_day10,spalte++,zeile);
  PaintChar(GPS_day01,spalte++,zeile);
  PaintChar('.',spalte++,zeile);
  PaintChar(GPS_mon10,spalte++,zeile);
  PaintChar(GPS_mon01,spalte++,zeile);
  PaintChar('.',spalte++,zeile);    
  PaintChar(GPS_year10,spalte++,zeile);
  PaintChar(GPS_year01,spalte++,zeile);
}


//-----------------------------------------------------------------------------
// Screen01: Datum Uhrzeit lat lon...
//-----------------------------------------------------------------------------
void Screen01(void)
{
BYTE sat_cnt,i,y,x,n;

  PaintChar(0x30+z10sec,14,1);
  PaintChar(0x30+z01sec,15,1);
  PaintChar(0x30+z100ms,16,1);

  i=0;
  PaintChar((time.h>>4) + '0',i++,0);
  PaintChar((time.h & 0x0F) + '0',i++,0);
  PaintChar(':',i++,0);
  PaintChar((time.m>>4) + '0',i++,0);
  PaintChar((time.m & 0x0F) + '0',i++,0);
  PaintChar('.',i++,0);    
  PaintChar((time.s>>4) + '0',i++,0);
  PaintChar((time.s & 0x0F) + '0',i++,0);

  if (GPS_day01!=GPS_day01old) 
  {
    PaintChar(' ',i++,0);    
    PaintChar(GPS_day10,i++,0);
    PaintChar(GPS_day01,i++,0);
    PaintChar('.',i++,0);
    PaintChar(GPS_mon10,i++,0);
    PaintChar(GPS_mon01,i++,0);
    PaintChar('.',i++,0);    
    PaintChar(GPS_year10,i++,0);
    PaintChar(GPS_year01,i++,0);
    GPS_day01=GPS_day01old;
  }
  if ((z01sec==2) || (z01sec==7)) 
  {
    i=0;
    PaintChar(GPS_NorthSouth,i++,1);
    for (n=0; n<10; n++)
    {
      PaintChar(GPS_Lat[n],i++,1);               // Breitengrad
    }
    i=0;
    PaintChar(GPS_EastWest,i++,2);
    for (n=0; n<9; n++)
    {
      PaintChar(GPS_Lon[n],i++,2);               // Laengengrad
    }

    PaintChar(' ',i++,2);
    for (n=0; n<5; n++)
    {
      PaintChar(GPS_Alt[n],i++,2);               // Hoehe
    }
    PaintChar(GPS_Alt_Unit,i++,2);
    //WriteString(0,1,"E13.36888");
    //WriteString(0,2,"N52.52017 00.3kmh");
    i=0;
    PaintChar('v',i++,3);
    PaintChar(':',i++,3);
    PaintChar(((sat_visible>>4) + 0x30),i++,3);
    PaintChar(((sat_visible & 0x0F) + 0x30),i++,3);
    PaintChar(' ',i++,3);
    PaintChar('u',i++,3);
    PaintChar(':',i++,3);
    PaintChar(sat10,i++,3);
    PaintChar(sat01,i++,3);
    PaintChar(' ',i++,3);    
    PaintChar(fix,i++,3);
    if (fix<0x32) { PaintChar(' ',i++,3); }
    else          { PaintChar('d',i++,3); }
    i=0;
    PaintChar('p',i++,4);
    PaintChar(dop.p[0],i++,4);
    PaintChar(dop.p[1],i++,4);
    PaintChar(dop.p[2],i++,4);
    PaintChar(dop.p[3],i++,4);
    i++;
    PaintChar('h',i++,4);
    PaintChar(dop.h[0],i++,4);
    PaintChar(dop.h[1],i++,4);
    PaintChar(dop.h[2],i++,4);
    PaintChar(dop.h[3],i++,4);
    i++;
    PaintChar('v',i++,4);
    PaintChar(dop.v[0],i++,4);
    PaintChar(dop.v[1],i++,4);
    PaintChar(dop.v[2],i++,4);
    PaintChar(dop.v[3],i++,4);
  }

  i=0; y=5;
  if (sat_visible>0x15) { sat_visible = 0x15; }
  sat_cnt = ((sat_visible>>4)*10)+(sat_visible & 0x0F);
  for (x=0; x<sat_cnt; x++) 
  {
    if (x== 3) { i=0; y=6; }
    if (x== 6) { i=0; y=7; }
    if (x== 9) { i=0; y=8; }
    if (x==12) { i=0; y=9; }
    PaintChar(((sat_tab[x].sat_id>>4) + 0x30),i++,y);
    PaintChar(((sat_tab[x].sat_id & 0x0F) + 0x30),i++,y);
    PaintChar('-',i++,y);
    if ((sat_tab[x].sat_db>1) && (sat_tab[x].sat_db<99)) 
    {
      PaintChar(((sat_tab[x].sat_db>>4) + 0x30),i++,y);
      PaintChar(((sat_tab[x].sat_db & 0x0F) + 0x30),i++,y);
    }
    else 
    {
      PaintChar('_',i++,y);
      PaintChar('_',i++,y);
    }
    PaintChar(' ',i++,y);
  }
}


//-----------------------------------------------------------------------------
// Screen02: Sat-ID + Pegelbalken
//-----------------------------------------------------------------------------
void Screen02(void) 
{
BYTE sat_cnt,i,y,x;
// Zeile  y 0..9
// Spalte   0..16  3 - 6 - 9

  if ((z01sec==2) || (z01sec==7))     // alle 5 Sekunden
  { 
    if ((sat_visible<sat_visible_old) || (sat_used < sat_used_old)) 
    {
      ClearDisplay_fast(); 
    }
    sat_visible_old = sat_visible;
    sat_used_old = sat_used;
  }


  if (sat_visible>0x16) { sat_visible = 0x16; }
  i=0; y=0;
  PaintChar('v',i++,y);
  PaintChar('i',i++,y);
  PaintChar('s',i++,y);
  PaintChar(':',i++,y);
  PaintChar(((sat_visible>>4) + 0x30),i++,y);
  PaintChar(((sat_visible & 0x0F) + 0x30),i++,y);
  PaintChar(' ',i++,y);
  PaintChar('u',i++,y);
  PaintChar('s',i++,y);
  PaintChar('e',i++,y);
  PaintChar('d',i++,y);
  PaintChar(':',i++,y);
  PaintChar(sat10,i++,y);
  PaintChar(sat01,i++,y);
  PaintChar(' ',i++,y);    
  PaintChar(fix,i++,y);
  if (fix<0x32) { PaintChar(' ',i++,y); }
  else          { PaintChar('d',i++,y); }
  i=0; y=3;
  sat_cnt = ((sat_visible>>4)*10)+(sat_visible & 0x0F);

  for (x=0; x<sat_cnt; x++)
  {
    if (x== 6) { y=6; i=0; }
    if (x==12) { y=9; i=0; }
    PaintChar(((sat_tab[x].sat_id>>4) + 0x30),i++,y);
    PaintChar(((sat_tab[x].sat_id & 0x0F) + 0x30),i++,y);
    if ((sat_tab[x].sat_db>=0) && (sat_tab[x].sat_db<99)) 
    {
      Paint_db_Balken(x,i-2,y);
    }
    PaintChar(' ',i++,y);
  }

  Paint_Line();
}

//-----------------------------------------------------------------------------
// Screen03: GPS Statistik
//-----------------------------------------------------------------------------
void Screen03(void) 
{
  BYTE i=0;
  if (buff_0or1==0)
  {
    while ((*lesezeiger!='\n') && (lesezeiger<(&puffer1[8])))
    {
      PaintChar(*lesezeiger,i++,4);
      lesezeiger++; 
    }
  }
  else
  {
    while ((*lesezeiger!='\n') && (lesezeiger<(&puffer0[8])))
    {
      PaintChar(*lesezeiger,i++,5);
      lesezeiger++; 
    }
  }
}


//-------------------------------------------------------------------------------------------------
// eigenes Modul? GPS.c
//-----------------------------------------------------------------------------
// Decodierung der NMEA-Botschaften
//-----------------------------------------------------------------------------
void DecodeNMEA(const char *ptrbuff)
{
  if ( strncmp((char*)(ptrbuff+3),"GSV", 3) == 0 ) { Decode_GPGSV(); }
  else 
  {
    if ( strncmp((char*)(ptrbuff+3),"GGA", 3 ) == 0 ) { Decode_GPGGA(); }
    else 
    {
      if ( strncmp((char*)(ptrbuff+3),"GSA", 3 ) == 0 ) { Decode_GPGSA(); }
      else 
      {
        if ( strncmp((char*)(ptrbuff+3),"RMC", 3 ) == 0 ) { Decode_GPRMC(); }
      }
    }
  }
}


//----------------------------------------------------------------------------------------
//  $GPGSV: Satellites In View
//----------------------------------------------------------------------------------------
void Decode_GPGSV(void)
{
BYTE pos,n=0;  

  if (*(lesezeiger+9)=='1') { n = 0; }         //  0...3
  else
  {
    if (*(lesezeiger+9)=='2') { n = 4; }       //  4...7
    else 
    {
      if (*(lesezeiger+9)=='3') { n = 8; }     //  8..11
      else
      {
        if (*(lesezeiger+9)=='4') { n = 12; }  // 12..15
      }
    }
  }

//  for (i=num; i<(num+4); i++)
  {
    pos = searchPos1((char*)lesezeiger,3);
    sat_visible = (*(lesezeiger+pos))<<4;
    sat_visible += (0x0F & (*(lesezeiger+pos+1)));

    pos = searchPos1((char*)lesezeiger,4);
    sat_tab[n].sat_id = (*(lesezeiger+pos))<<4;
    sat_tab[n].sat_id += (0x0F & (*(lesezeiger+pos+1)));
    pos = searchPos1((char*)lesezeiger,7);
    sat_tab[n].sat_db = (*(lesezeiger+pos))<<4;
    sat_tab[n].sat_db +=(0x0F & (*(lesezeiger+pos+1)));

    pos = searchPos1((char*)lesezeiger,8);
    sat_tab[n+1].sat_id = (*(lesezeiger+pos))<<4;
    sat_tab[n+1].sat_id += (0x0F & (*(lesezeiger+pos+1)));
    pos = searchPos1((char*)lesezeiger,11);
    sat_tab[n+1].sat_db = (*(lesezeiger+pos))<<4;
    sat_tab[n+1].sat_db +=(0x0F & (*(lesezeiger+pos+1)));

    pos = searchPos1((char*)lesezeiger,12);
    sat_tab[n+2].sat_id = (*(lesezeiger+pos))<<4;
    sat_tab[n+2].sat_id += (0x0F & (*(lesezeiger+pos+1)));
    pos = searchPos1((char*)lesezeiger,15);
    sat_tab[n+2].sat_db = (*(lesezeiger+pos))<<4;
    sat_tab[n+2].sat_db +=(0x0F & (*(lesezeiger+pos+1)));

    pos = searchPos1((char*)lesezeiger,16);
    sat_tab[n+3].sat_id = (*(lesezeiger+pos))<<4;
    sat_tab[n+3].sat_id += (0x0F & (*(lesezeiger+pos+1)));
    pos = searchPos1((char*)lesezeiger,19);
    sat_tab[n+3].sat_db = (*(lesezeiger+pos))<<4;
    sat_tab[n+3].sat_db +=(0x0F & (*(lesezeiger+pos+1)));
  }
}


//----------------------------------------------------------------------------------------
//  $GPGGA: Global Positioning System Fix Data
//----------------------------------------------------------------------------------------
void Decode_GPGGA(void) 
{
BYTE pos,i;

  pos = searchPos1((char*)lesezeiger,1);
  time.h  = *(lesezeiger+pos+0)<<4;
  time.h += (0x0F & (*(lesezeiger+pos+1)));
  time.m  = *(lesezeiger+pos+2)<<4;
  time.m += (0x0F & (*(lesezeiger+pos+3)));
  time.s  = *(lesezeiger+pos+4)<<4;
  time.s += (0x0F & (*(lesezeiger+pos+5)));

  pos = searchPos1((char*)lesezeiger,2);
  for (i=0; i<10; i++)
  {
    if (*(lesezeiger+pos)==',') { break; }
    GPS_Lat[i] = *(lesezeiger+pos+i); 
  }
  pos = searchPos1((char*)lesezeiger,3);
  GPS_NorthSouth = *(lesezeiger+pos);
  pos = searchPos1((char*)lesezeiger,4);
  for (i=0; i<10; i++)
  {
    if (*(lesezeiger+pos)==',') { break; }
    GPS_Lon[i] = *(lesezeiger+pos+i); 
  }
  pos = searchPos1((char*)lesezeiger,5);
  GPS_EastWest = *(lesezeiger+pos);
  pos = searchPos1((char*)lesezeiger,9);
  for (i=0; i<5; i++)
  {
    if (*(lesezeiger+pos)==',') { break; }
    GPS_Alt[i] = *(lesezeiger+pos+i); 
  }
  pos = searchPos1((char*)lesezeiger,10);
  GPS_Alt_Unit = *(lesezeiger+pos);
  pos = searchPos1((char*)lesezeiger,7);
  sat10 = *(lesezeiger+pos);
  sat01 = *(lesezeiger+pos+1);
}


//----------------------------------------------------------------------------------------
//  $GPGSA: GPS DOP (Dilution of Precision) and Active Satellites
//----------------------------------------------------------------------------------------
void Decode_GPGSA(void)
{
BYTE pos,i;
  for (i=0;i<12;i++)
  {
    pos = searchPos1((char*)lesezeiger,3+i);
    if (*(lesezeiger+pos)!=',')
    {
      usedSat_Tab[i] = (*(lesezeiger+pos))<<4;
      usedSat_Tab[i] += (0x0F & (*(lesezeiger+pos+1)));
    }
    else 
    {
      usedSat_Tab[i] = 0;
    }
  }
  pos = searchPos1((char*)lesezeiger,2);
  fix =  *(lesezeiger+pos);
  pos = searchPos1((char*)lesezeiger,15);
  dop.p[0] = *(lesezeiger+pos+0);
  dop.p[1] = *(lesezeiger+pos+1);
  dop.p[2] = *(lesezeiger+pos+2);
  dop.p[3] = *(lesezeiger+pos+3);
  pos = searchPos1((char*)lesezeiger,16);
  dop.h[0] = *(lesezeiger+pos+0);
  dop.h[1] = *(lesezeiger+pos+1);
  dop.h[2] = *(lesezeiger+pos+2);
  dop.h[3] = *(lesezeiger+pos+3);
  pos = searchPos1((char*)lesezeiger,17);
  dop.v[0] = *(lesezeiger+pos+0);
  dop.v[1] = *(lesezeiger+pos+1);
  dop.v[2] = *(lesezeiger+pos+2);
  dop.v[3] = *(lesezeiger+pos+3);
}


//----------------------------------------------------------------------------------------
//  $GPRMC: Recommended Minimum Specific GPS/TRANSIT Data
//----------------------------------------------------------------------------------------
void Decode_GPRMC(void)
{
BYTE pos;
  pos = searchPos1((char*)lesezeiger,9);
  GPS_day10 = *(lesezeiger+pos);
  GPS_day01 = *(lesezeiger+pos+1);
  GPS_mon10 = *(lesezeiger+pos+2);
  GPS_mon01 = *(lesezeiger+pos+3);
  GPS_year10 = *(lesezeiger+pos+4);
  GPS_year01 = *(lesezeiger+pos+5);
}


//----------------------------------------------------------------------------------------
//  prueft, ob Sat zur Berechnung genutzt wird
//            1: ja   (gefuellter Balken)
//            0: nein (gestrichelter Balken)
//----------------------------------------------------------------------------------------
BYTE isSat_in_usedSat_Tab(BYTE number) 
{
BYTE j, r=0;

  for (j=0; j<12; j++)
  { 
    if (usedSat_Tab[j] == number) 
    {
      r = 1;
      break;
    }
  }
  return r;
}


//----------------------------------------------------------------------------------------
//  NMEA_CSum   0: false
//              1: true
//----------------------------------------------------------------------------------------
BYTE NMEA_CSum(const char *ptrRX)
{
BYTE i,ch,re=0;
uint16_t tmp,csum;
BYTE sec;
  sec = *ptrRX;
  tmp=csum=0;
  i=0;
  while ((*(ptrRX+i)!='*') && (i<max)) { i++; }
  if (i<max) 
  {
    if (*(ptrRX+i+1)>0x39) { csum = (*(ptrRX+i+1) - 0x37)<<4; }
    else                   { csum = (*(ptrRX+i+1) & 0x0F)<<4; }

    if (*(ptrRX+i+2)>0x39) { csum += (*(ptrRX+i+2) - 0x37); }
    else                   { csum += (*(ptrRX+i+2) & 0x0F); }  

    for (i=1; i<max; i++)
	{
	  ch = *(ptrRX+i);
	  if (ch == '*') { break; }
	  tmp ^= *(ptrRX+i);
	}
    if (csum==tmp) { re = 1; }
    else           { re = 0; }
  }
  return re;
}


//----------------------------------------------------------------------------------------
//  searchPos
//----------------------------------------------------------------------------------------
BYTE searchPos1(char *pt, BYTE kommaanz)
{
BYTE i;

  for (i=0; i<max; i++)
  {
    if (*pt==',') { kommaanz--; }
    if (kommaanz==0) { return i+1; }           // die position nach dem letzten komma
    pt++;
  }
  return -1;
}


//----------------------------------------------------------------------------------------
//  Balkengrafik
// hoehe: 13..0 'voll bis 0' = 13- [0 4 8 12 16 20 24 28 32 36 40 44 48 52]/4
//----------------------------------------------------------------------------------------
void Paint_db_Balken(BYTE _num, BYTE _x, BYTE _y) 
{
BYTE hoehe,_db,voll;

  _db = sat_tab[_num].sat_db;
  voll = isSat_in_usedSat_Tab(sat_tab[_num].sat_id);
//-- erst loeschen
  write_cmd(0x24);                     // Rechteck
  write_cmd(_x*3+7);                   // Col Spalte X
  write_cmd(8*_y-14-1+1);              // Row	Zeile Y
  write_cmd(_x*3+7+4);                 // Col Spalte X
  write_cmd(8*_y-2);                   // Row Zeile Y
  write_cmd(0x00);
//--
  if (_db>0x52) { _db=0x52; }
  hoehe = (_db>>4)*10 + (_db & 0x0F);
  hoehe = (13-(hoehe/4));
  if (hoehe>1) 
  {
    write_cmd(0x24);                   // Rechteck
    write_cmd(_x*3+7);                 // Col Spalte X
    write_cmd(8*_y-14-2+hoehe);        // Row	Zeile Y
    write_cmd(_x*3+7+4);               // Col Spalte X
    write_cmd(8*_y-2);                 // Row Zeile Y
    if (voll==1) { write_cmd(0xFF); }  // voller Balken
    else         { write_cmd(0x0F); }  // gestrichelter Balken
  }
}


//----------------------------------------------------------------------------------------
//  3 horizontale Linien
//----------------------------------------------------------------------------------------
void Paint_Line(void)
{
BYTE y;
  for (y=3; y<10; y+=3)	     // 3 6 9 
  {
    write_cmd(0x24);         // Rechteck
    write_cmd(0x07);         // Col Spalte X	 0x07
    write_cmd(8*y-14-1);     // Row	Zeile Y
    write_cmd(0x39);         // Col Spalte X	 0x39
    write_cmd(8*y-14-1);     // Row	Zeile Y		 0x4F
    write_cmd(0xFF);
  }
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
