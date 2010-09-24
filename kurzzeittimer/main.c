//--------------------------------------------------------------------------------------
// ATmega168PV 8MHz - Kurzzeitwecker-OLED-Demo SSD1325  (im Geraet lauffaehig)
//
// // Copyright (c) 2010, Bjoern Heller <tec@hellercom.de>. All rights reserved
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
//#include <avr/io.h>
#include <avr/sleep.h>
#include <compat/deprecated.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <inttypes.h>
#include <avr/interrupt.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "HMI.h"
#include "Font.h"
#include "Oled.h"
#include "Segment.h"

//--------------------------------------------------------------------------------------
#define BYTE unsigned char
#define WORD unsigned short
//#define F_CPU 8000000

//-----------------------------------------------------------------------------

#define QFP100   0      // 1: ATMEL QFP100 noch auf dem Board, also keine Tastennutzung!
                        // 0: ATMEL QFP100 entfernt,  Tastennutzung moeglich

#define SW_SPI   0      // 0: Hardware SPI
                        // 1: Software SPI
//-----------------------------------------------------------------------------
void Timer0_Init(void);
void Init_USART(void);
void delay_ms(int ms);
void delay(unsigned char);

//-----------------------------------------------------------------------------
BYTE ScreenMax = 3;			    // Anzahl der Screens
BYTE Ziffer;
WORD freemilli=0;
volatile BYTE z001ms=0, z010ms=0, z100ms=0, z01sec=0, z10sec=0, buffer;
volatile BYTE Ovl10ms=0, Ovl100ms=0, Ovl1sec=0, Ovl10sec=0, Ovl1min=0;


//-------------------------------------------------------------------------------------------------
//  - main - main - main - main - main - main - main - main - main - main - main - main - main -
//-------------------------------------------------------------------------------------------------
int main(void)
{
  DDRB  = 0b00000110;
  PORTB = 0b00000000;
    	
  DDRC  = 0b00000110;
  PORTC = 0b00110000;        // PORTC.4 linke Taste / PORTC.5 rechte Taste
  
  DDRD =  0b00110010;
  PORTD = 0b00001000;        // PORTD.3 mittlere Taste

  static BYTE blink=1;

  Tast_cnt_old = Tast_cnt;
  CntDown=0;
  DisplayOnOff = 1;
  ShutdownCnt = _30sec;

  cli();
  Timer0_Init();
  Init_USART();
  sei();

  DisplayOn();               // incl. OledInit();
  ClearDisplay_fast();

//-------------------

  RestoreSetup();

  if (StartScreen<3)
  {
    activeScreen = 1;
    markingS00 = StartScreen;
    markingS01=4;
  }
  else activeScreen = 0;

  DisplayScreen();

  // ----------------------------------------------------------------------------------------------
  // --     while(1) -- while(1) -- while(1) -- while(1) -- while(1) -- while(1) -- while(1)     --
  // ----------------------------------------------------------------------------------------------
  while(1)
  {
    // --------------------------------------------------------------------------------------------
    // z e i t g e s t e u e r t e   F  u n k t i o n e n :
    //-----------------------10ms------------------------------------------------------------------
    if (Ovl10ms)
    {
      Ovl10ms = 0;
      if (CheckKey()) { DisplayScreen(); }
    }
    //-----------------------100ms-----------------------------------------------------------------
    if (Ovl100ms)
    {
      Ovl100ms = 0;
        if (alarm>0) {
        if ((z100ms!=5) && (z100ms!=7)) { AlarmTon(000); }
        else               { AlarmTon(5000); }
      }

      if (!(z100ms%5) && (CntDown==1)) 
      { 
        blink ^= 0x01;
        DoppelPunkt(8*8-2,20+11,blink);          // Doppelpunkt blinkt alle 500ms
      }
    }
    //-----------------------1s--------------------------------------------------------------------
    if (Ovl1sec) 
    {
      Ovl1sec = 0;

      if (alarm>0) {
        if ((EmTime-1)>0) { EmTime--; }
        else              { alarm=0; AlarmTon(0); }
      }

      if (CntDown==1)
      {
        if (CountDownTime>0)
        {
          CountDownTime--;
          DisplayScreen();
          if (CountDownTime==0)
          { 
            CntDown = 0;
            alarm = 1;
            EmTime = EmergencyTime;
          }
        }
      }
      
      if (CntDown!=1) {
        if (ShutdownCnt) { ShutdownCnt--; }
        else             { if (DisplayOnOff) { DisplayOff(); SleepModeOn(); } }
      }
    }
    //-----------------------10s-------------------------------------------------------------------
    if (Ovl10sec) 
    {
      Ovl10sec = 0;
    }
    //-----------------------1min------------------------------------------------------------------
    if (Ovl1min) 
    {
      Ovl1min = 0;
    }
  // ----------------------------------------------------------------------------------------------

  }
  // ----------------------------------------------------------------------------------------------
  // --         End of while(1) -- End of while(1) -- End of while(1) -- End of while(1)         --
  // ----------------------------------------------------------------------------------------------

  return 0;         // never!
}                   // end of main


//----------------------------------------------------------------------------------------
// Initialisierung Timer0A 1ms   8000000/64/1000 = 125
//----------------------------------------------------------------------------------------
void Timer0_Init(void)
{
  TCCR0A = (1<<WGM01); 
  TCCR0B = (1<<CS01)|(1<<CS00);       // Prescaler 64
  OCR0A = 125+1;                      // anpassen?
  TIMSK0 |= (1<<OCIE0A);
}


//----------------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------------
ISR(TIMER0_COMPA_vect)
{
  freemilli++;
  z001ms++;
  if (z001ms>=10) { z001ms = 0; z010ms++; Ovl10ms = 1;  }
  if (z010ms>=10) { z010ms = 0; z100ms++; Ovl100ms = 1; }
  if (z100ms>=10) { z100ms = 0; z01sec++; Ovl1sec = 1;  }
  if (z01sec>=10) { z01sec = 0; z10sec++; Ovl10sec = 1; }
  if (z10sec>= 6) { z10sec = 0; Ovl1min = 1;  }
}


//----------------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------------
ISR (INT1_vect)
{
  EIMSK &= ~(1 << INT1);     // External Interrupt Request Diable
  PORTC = 0b00110000;
  Tastensperre = 30;         // 300ms Tastensperre
  ShutdownCnt = _30sec;
  DisplayOn();
  cli();
  Timer0_Init();
  sei();
}


//----------------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------------
void Init_USART(void)
{
  UBRR0 = 0;
  DDRD |= 1<<4; //DDRD |= 1<<PD4; //XCK_DDR |= (1<<XCK);
  UCSR0C = (1<<UMSEL01)|(1<<UMSEL00)|(1<<UCPHA0)|(1<<UCPOL0);
  UCSR0B = (1<<TXEN0);
  UBRR0 = 2;
}


//-----------------------------------------------------------------------------
// Delay
//-----------------------------------------------------------------------------
void delay_ms(int ms)
{
int t;
  for (t=0; t<=ms; t++)
  _delay_ms(1);
}


//-----------------------------------------------------------------------------
//                      E n d   o f   m a i n . c 
//-----------------------------------------------------------------------------
