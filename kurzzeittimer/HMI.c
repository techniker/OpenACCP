// Copyright (c) 2010, Bjoern Heller <tec@hellercom.de>. All rights reserved
// This code is licensed under GNU/ GPL

//----------------------------------------------------------------------------------------
// HMI.c - Human Machine Interface
//----------------------------------------------------------------------------------------
#include "HMI.h"

//----------------------------------------------------------------------------------------
volatile WORD EEMEM eeTimeEi;     // Eierkochzeit
volatile WORD EEMEM eeTimeZahn;   // Zahnputzdauer
volatile WORD EEMEM eeTimeCD;     // freier CountDown
volatile BYTE EEMEM eeEmergencyTime;
volatile BYTE EEMEM eeStartScreen;

//-----------------------------------------------------------------------------
// Prueft, ob eine Taste betaetigt wurde (linke, mittlere und rechte Taste!)
//-----------------------------------------------------------------------------
BYTE CheckKey(void)
{
BYTE r;
  
  r = 0;
  if (Tastensperre > 0) { Tastensperre--; }
  else
  {
    if ((PINC & (1<<PINC4))==0)        // linke Taste
    {
      ShutdownCnt = _30sec;						          // Shutdown-Timer aufziehen!
      if (!DisplayOnOff) { DisplayOn(); }
      PressLeftKey();
      Tastensperre = 30;                         // 300ms Tastensperre
      Tast_cnt++;
      if (Tast_cnt != Tast_cnt_old) 
      {
        Tast_cnt_old = Tast_cnt;
        //ClearDisplay();      
        r = 1;      
      }
    }

    if ((PIND & (1<<PIND3))==0)        // mittlere Taste
    {
      ShutdownCnt = _30sec;								      // Shutdown-Timer aufziehen!
      if (!DisplayOnOff) { DisplayOn(); }
      PressCenterKeyXX();
      Tastensperre = 30;                         // 300ms Tastensperre
      Tast_cnt++;
      if (Tast_cnt != Tast_cnt_old) 
      {
        Tast_cnt_old = Tast_cnt;
        //ClearDisplay();      
        r = 1;      
      }
    }  

    if ((PINC & (1<<PINC5))==0)        // rechte Taste
    {
      ShutdownCnt = _30sec;								      // Shutdown-Timer aufziehen!
      if (!DisplayOnOff) { DisplayOn(); }
      PressRightKey();
      Tastensperre = 30;                         // 300ms Tastensperre
      Tast_cnt++;
      if (Tast_cnt != Tast_cnt_old) 
      {
        Tast_cnt_old = Tast_cnt;
        //ClearDisplay();      
        r = 1;      
      }
    }  

  }
  return r;
}


//-----------------------------------------------------------------------------
// rechte Taste
//-----------------------------------------------------------------------------
void PressRightKey(void)
{
  switch (activeScreen)
  {
    case 0:                  // MenueScreen
            if (markingS00<3) { markingS00++;   }
            else              { markingS00 = 0; }
            break;

    case 1:                  // Countdown / Eiertimer / Zahnputztimer
            if (markingS01<5) { markingS01++;   }
            else              { markingS01 = 0; }
            break;

    case 2:                  // active CounterScreen
            if (CountDownTime>0) {
              if (markingS02<1) { markingS02++;   }
              else              { markingS02 = 0; }
            }
            else
            {
              alarm = 0;
              AlarmTon(0);
              StopCounter();
              activeScreen = 0; //Screen01();
              alarm = 0;
            }
            break;

    case 3:                  // SetupScreen

            if (markingS03<3) { markingS03++;   }
            else              { markingS03 = 0; }
            break;

   default: break;
  }
}


//-----------------------------------------------------------------------------
// linke Taste
//-----------------------------------------------------------------------------
void PressLeftKey(void)
{
  switch (activeScreen)
  {
    case 0:                  // MenueScreen
            if (markingS00>0) { markingS00--;   }
            else              { markingS00 = 3; }
            break;

    case 1:                  // Countdown / Eiertimer / Zahnputztimer
            if (markingS01>0) { markingS01--;   }
            else              { markingS01 = 5; }
            break;

    case 2:                  // active CounterScreen
            if (CountDownTime>0) {
              if (markingS02>0) { markingS02--;   }
              else              { markingS02 = 1; }
            }
            else
            {
              alarm = 0;
              AlarmTon(0);
              StopCounter();
              activeScreen = 0; //Screen01();
              alarm = 0;
            }
            break;

    case 3:                  // SetupScreen
            if (markingS03>0) { markingS03--;   }
            else              { markingS03 = 3; }
            break;

   default: break;
  }
}


//-----------------------------------------------------------------------------
// Taste mitte Verteilung
//-----------------------------------------------------------------------------
void PressCenterKeyXX(void)
{

  //if (activeScreen!=activeScreen_old) { ClearDisplay(); }
  switch (activeScreen)
  {
    case 0:  PressCenterKey00();  break;
    case 1:  PressCenterKey01();  break;
    case 2:  PressCenterKey02();  break;
    case 3:  PressCenterKey03();  break;
   default:  break;
  }
}


//-----------------------------------------------------------------------------
// Taste mitte im Screen00
//-----------------------------------------------------------------------------
void PressCenterKey00(void)
{
  switch (markingS00)
  {   
    case 0:
    case 1:
    case 2:  RestoreSetup(); markingS01 = 4; activeScreen = 1;  break;
    //case 2:  markingS01 = 4; activeScreen = 1;  break;
    case 3:  activeScreen = 3; break; // RestoreSetup();
   default: break;
  }
}


//-----------------------------------------------------------------------------
// Taste mitte im Screen01
//-----------------------------------------------------------------------------
void PressCenterKey01(void)
{
  switch (markingS01)
  {   
    case 0:
            if (z0<9) { z0++;   } 
            else      { z0 = 0; }            
            break;

    case 1:
            if (z1<9) { z1++;   } 
            else      { z1 = 0; }            
            break;

    case 2:
            if (z2<5) { z2++;   } 
            else      { z2 = 0; }            
            break;

    case 3:
            if (z3<9) { z3++;   } 
            else      { z3 = 0; }            
            break;

    case 4:
            StartCountDown();
            SaveSetup();
            activeScreen = 2;
            break;

    case 5:
            activeScreen = 0; //Screen00();
            break;

   default: break;
  }
}


//-----------------------------------------------------------------------------
// Taste mitte im Screen02
//-----------------------------------------------------------------------------
void PressCenterKey02(void)
{
  switch (markingS02)
  {   
    case 0:
            if (CountDownTime>0) {
              if (CntDown==1) PauseCounter();
              else {
                if (CntDown==2) ContinueCounter();
              }
            }
            else
            {
              alarm = 0;
              AlarmTon(0);
              StopCounter();
              activeScreen = 0; //Screen01();
              alarm = 0;
            }
            break;

    case 1:
            StopCounter();
            activeScreen = 0; //Screen01();
            break;

   default: break;
  }
}


//-----------------------------------------------------------------------------
// Taste mitte im Screen03
//-----------------------------------------------------------------------------
void PressCenterKey03(void)
{
  switch (markingS03)
  {   
    case 0:
            if (StartScreen<3) { StartScreen++;   } 
            else               { StartScreen = 0; }            
            break;

    case 1:
            if (EmergencyTime<9) { EmergencyTime++;   } 
            else                 { EmergencyTime = 0; }            
            break;

    case 2:
            activeScreen = 0;
            break;

    case 3:
            SaveSetup();
            activeScreen = 0;
            SaveSetup();
            break;

   default: break;
  }
}


//-----------------------------------------------------------------------------
// zeigt den aktuellen Screen
//-----------------------------------------------------------------------------
void DisplayScreen(void)
{
static BYTE aScreen_old=5;
BYTE r;

  r = 1;
  if (aScreen_old!=activeScreen)
  { 
    aScreen_old=activeScreen;
    r = 0;                             // 0..Screen: komplett beschreiben 
  }                                    // 1..Screen: nur Teile refreshen

  switch (activeScreen)
  {
    case 0:  Screen00(r);  break;
    case 1:  Screen01(r);  break;
    case 2:  Screen02(r);  break;
    case 3:  Screen03(r);  break;
   default:  break;
  }
}


//-----------------------------------------------------------------------------
// Menue
//-----------------------------------------------------------------------------
void Screen00(BYTE refresh)
{
  if (refresh==0)                                // alles neu
  { 
    WriteString(0,0,"                 ");
    WriteString(0,1,"     Eieruhr     ");
    WriteString(0,2,"                 ");
    WriteString(0,3,"    Countdown    ");
    WriteString(0,4,"                 ");
    WriteString(0,5,"  Zahnputztimer  ");
    WriteString(0,6,"                 ");
    WriteString(0,7,"                 ");
    WriteString(0,8,"           Setup ");
    WriteString(0,9,"                 ");
  }

  if (markingS00!=0)  { PaintChar(' ', 4,1); PaintChar(' ',12,1); }
  if (markingS00!=1)  { PaintChar(' ', 3,3); PaintChar(' ',13,3); }
  if (markingS00!=2)  { PaintChar(' ', 1,5); PaintChar(' ',15,5); }
  if (markingS00!=3)  { PaintChar(' ',10,8); PaintChar(' ',16,8); }

  if (markingS00==0)  { PaintChar('*', 4,1); PaintChar('*',12,1); }
  if (markingS00==1)  { PaintChar('*', 3,3); PaintChar('*',13,3); }
  if (markingS00==2)  { PaintChar('*', 1,5); PaintChar('*',15,5); }
  if (markingS00==3)  { PaintChar('*',10,8); PaintChar('*',16,8); }
}


//-----------------------------------------------------------------------------
// Countdown / Eiertimer / Zahnputztimer
//-----------------------------------------------------------------------------
void Screen01(BYTE refresh)
{
  if (refresh==0)                                // alles neu
  {
    switch (markingS00) 
    {
      case 0:  WriteString(1,0,"Eieruhr:");   break;
      case 1:  WriteString(1,0,"Countdown:"); break;
      case 2:  WriteString(1,0,"Zahn:");      break;          
     default:  WriteString(1,0,"                 "); break;
    }

    WriteString(0,1,"                 ");
    WriteString(0,2,"                 ");
    WriteString(0,3,"                 ");
    WriteString(0,4,"                 ");
    PaintChar(':', 8,4);
    WriteString(0,5,"     min:sec     ");
    WriteString(0,6,"                 ");
    WriteString(0,7,"                 ");
    WriteString(0,8,"                 ");
    WriteString(0,9," Exit      Start ");
  }

  PaintChar(z0+0x30, 6,4);
  PaintChar(z1+0x30, 7,4);
  PaintChar(z2+0x30, 9,4);
  PaintChar(z3+0x30,10,4);

  if (markingS01!=0)  { PaintChar(' ', 6,3); }
  if (markingS01!=1)  { PaintChar(' ', 7,3); }
  if (markingS01!=2)  { PaintChar(' ', 9,3); }
  if (markingS01!=3)  { PaintChar(' ',10,3); }
  if (markingS01!=4)  { PaintChar(' ',10,9); PaintChar(' ',16,9); }
  if (markingS01!=5)  { PaintChar(' ', 0,9); PaintChar(' ', 5,9); }

  if (markingS01==0)  { PaintChar('*', 6,3); }
  if (markingS01==1)  { PaintChar('*', 7,3); }
  if (markingS01==2)  { PaintChar('*', 9,3); }
  if (markingS01==3)  { PaintChar('*',10,3); }
  if (markingS01==4)  { PaintChar('*',10,9); PaintChar('*',16,9); }
  if (markingS01==5)  { PaintChar('*', 0,9); PaintChar('*', 5,9); }
}


//-----------------------------------------------------------------------------
// aktiver Timer
//-----------------------------------------------------------------------------
void Screen02(BYTE refresh)
{
  if (refresh==0)                                // alles neu
  {
    switch (markingS00) 
    {
      case 0:  WriteString(1,0,"Eieruhr:  ");  break;
      case 1:  WriteString(1,0,"Countdown:");  break;
      case 2:  WriteString(1,0,"Zahnputz..:"); break;          
     default:  break;
    }

    PaintChar(z0+0x30, 12,0);
    PaintChar(z1+0x30, 13,0);
    PaintChar(':', 14,0);
    PaintChar(z2+0x30, 15,0);
    PaintChar(z3+0x30,16,0);

  //WriteString(0,0,"                 ");
    WriteString(0,1,"                 ");
    WriteString(0,2,"                 ");
    WriteString(0,3,"                 ");
    WriteString(0,4,"                 ");
    WriteString(0,5,"                 ");
    WriteString(0,6,"                 ");
    WriteString(0,7,"                 ");
    WriteString(0,8,"                 ");
    WriteString(0,9,"     Abbruch     ");
  }

  Show4Digits(CountDownTime,8,20);

  if (CntDown==1) { WriteString(5,8," Pause  "); }
  else {
    if (CntDown==2) { WriteString(5,8,"Continue"); DoppelPunkt(8*8-2,20+11,1);}
  }

  if (markingS02==0)  { PaintChar('*', 3,8); PaintChar('*',14,8); }
  else                { PaintChar(' ', 3,8); PaintChar(' ',14,8); }
  if (markingS02==1)  { PaintChar('*', 3,9); PaintChar('*',14,9); }
  else                { PaintChar(' ', 3,9); PaintChar(' ',14,9); }
}


//-----------------------------------------------------------------------------
// Setup
//-----------------------------------------------------------------------------
void Screen03(BYTE refresh)
{
  if (refresh==0)                                // alles neu
  {
    WriteString(0,0,"      Setup      ");
    WriteString(0,1,"                 ");
    WriteString(0,2,"                 ");
    WriteString(0,3,"Start:           ");

    WriteString(0,4,"                 ");
    WriteString(0,5,"                 ");
    WriteString(0,6,"Alarm:           ");

    WriteString(9,6,"sec");
    WriteString(0,7,"                 ");
    WriteString(0,8,"                 ");
    WriteString(0,9," OK      Abbruch ");
  }

  switch (StartScreen) 
  {
    case 0:  WriteString(7,3,"Menue     ");  break;
    case 1:  WriteString(7,3,"Eieruhr   ");  break;
    case 2:  WriteString(7,3,"Countdown ");  break;
    case 3:  WriteString(7,3,"Zahnputz..");  break;          
   default:  break;
  }

  PaintChar(EmergencyTime+0x30,7,6);

  if (markingS03!=0)  { PaintChar(' ', 7,2); }
  if (markingS03!=1)  { PaintChar(' ', 7,5); }
  if (markingS03!=2)  { PaintChar(' ', 8,9); PaintChar(' ',16,9); }
  if (markingS03!=3)  { PaintChar(' ', 0,9); PaintChar(' ', 3,9); }

  if (markingS03==0)  { PaintChar('*', 7,2); }
  if (markingS03==1)  { PaintChar('*', 7,5); }
  if (markingS03==2)  { PaintChar('*', 8,9); PaintChar('*',16,9); }
  if (markingS03==3)  { PaintChar('*', 0,9); PaintChar('*', 3,9); }
}


//-----------------------------------------------------------------------------
// 4x 7-Segment-Anzeige
//-----------------------------------------------------------------------------
void Show4Digits(int digit, BYTE x, BYTE y)
{
BYTE m10, m01, s10,s01;
//static BYTE i=1;

  m10 = digit/60/10;
  m01 = digit/60 - m10*10;
  s10 = (digit%60)/10;
  s01 = (digit%60) - s10*10;

  if (m10>0) { ShowOneZiff(m10,y,x); }
  else {
    Segment_e(y,x,0);    
    Segment_f(y,x,0);
    Segment_a(y,x,0);
    Segment_d(y,x,0);
    Segment_b(y,x,0);
    Segment_c(y,x,0);
    Segment_g(y,x,0);

  }
  if ((m10>0) || (m01>0)) { ShowOneZiff(m01,y,x*4); }
  else {
    Segment_e(y,x*4,0);    
    Segment_f(y,x*4,0);
    Segment_a(y,x*4,0);
    Segment_d(y,x*4,0);
    Segment_b(y,x*4,0);
    Segment_c(y,x*4,0);
    Segment_g(y,x*4,0);
  }

  ///DoppelPunkt(8*x-2,y+11,i);
  ShowOneZiff(s10,y,8*x-2);
  ShowOneZiff(s01,y,10*x+6);
}


//-----------------------------------------------------------------------------
// CountDown Start
//-----------------------------------------------------------------------------
void StartCountDown(void)
{
  CountDownTime = (z0*10+z1)*60;
  CountDownTime+= (z2*10+z3);
  CntDown = 1;               // CountDown ist aktiv
  if (markingS00==0) TimeEi = CountDownTime;
  if (markingS00==1) TimeCD = CountDownTime;
  if (markingS00==2) TimeZahn = CountDownTime;
}


//-----------------------------------------------------------------------------
// CountDown Stop
//-----------------------------------------------------------------------------
void StopCounter(void)
{
  CntDown = 0;             // CountDown stop
  if (markingS00==0) { CountDownTime = TimeEi; }
  if (markingS00==1) { CountDownTime = TimeCD; }
  if (markingS00==2) { CountDownTime = TimeZahn; }
}


//-----------------------------------------------------------------------------
// CountDown Pause
//-----------------------------------------------------------------------------
void PauseCounter(void)
{
  CntDown = 2;             // CountDown pausieren
}


//-----------------------------------------------------------------------------
// CountDown Pause beenden
//-----------------------------------------------------------------------------
void ContinueCounter(void)
{
  CntDown = 1;
}


//-----------------------------------------------------------------------------
// Daten in EEPROM sichern
//-----------------------------------------------------------------------------
void SaveSetup(void)
{
  eeprom_write_byte((uint8_t*)&eeEmergencyTime, EmergencyTime);   // Alarmdauer
  eeprom_write_byte((uint8_t*)&eeStartScreen, StartScreen);       // StartScreen

  eeprom_write_word((uint16_t*)&eeTimeEi, TimeEi);                // Eierkochzeit
  eeprom_write_word((uint16_t*)&eeTimeZahn, TimeZahn);            // Zahnputzdauer
  eeprom_write_word((uint16_t*)&eeTimeCD, TimeCD);                // freier CountDown
}


//-----------------------------------------------------------------------------
// Daten aus EEPROM lesen
//-----------------------------------------------------------------------------
void RestoreSetup(void)
{
  EmergencyTime = eeprom_read_byte((uint8_t*)&eeEmergencyTime);
  if (EmergencyTime>9) EmergencyTime = 5;

  StartScreen = eeprom_read_byte((uint8_t*)&eeStartScreen);
  if (StartScreen>3) StartScreen = 0;

  //activeScreen = StartScreen;

  TimeEi = eeprom_read_word((uint16_t*)&eeTimeEi);
  if (TimeEi>300) TimeEi = 300;                  // auf 5:00 begrenzen

  TimeZahn = eeprom_read_word((uint16_t*)&eeTimeZahn);
  if (TimeZahn>300) TimeZahn = 270;              // auf 4:30 begrenzen

  TimeCD = eeprom_read_word((uint16_t*)&eeTimeCD);
  if (TimeCD>5999) TimeCD = 0;                   // wenn > 99:59 dann auf 0 begrenzen

  if (markingS00==0)
  {
    z0 = TimeEi/60/10;
    z1 = TimeEi/60 - z0*10;
    z2 = (TimeEi%60)/10;
    z3 = (TimeEi%60) - z2*10;
  }

  if (markingS00==1)
  {
    z0 = TimeCD/60/10;
    z1 = TimeCD/60 - z0*10;
    z2 = (TimeCD%60)/10;
    z3 = (TimeCD%60) - z2*10;
  }

  if (markingS00==2)
  {
    z0 = TimeZahn/60/10;
    z1 = TimeZahn/60 - z0*10;
    z2 = (TimeZahn%60)/10;
    z3 = (TimeZahn%60) - z2*10;
  }

}


//-----------------------------------------------------------------------------
// SleepMode einschalten -> Aufwachen mit Int1 (mittlere Taste)
//-----------------------------------------------------------------------------
void SleepModeOn(void)
{
  TIMSK0 &= ~(1<<OCIE0A);
  OLED_CS_LO;
  PORTC = 0b00000000;
  DDRC  = 0b00000000;
  EIMSK |= (1 << INT1);                  // externen Interrupt freigeben
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_mode();
}


//-----------------------------------------------------------------------------
// AlarmTon xxxx Hz
//
// Die Frequenz errechnet sich:
// 8Mhz(Taktfrequenz)/64(Prescaler)/2(Vollwelle)/1000hz(Ausgabefrequenz)= 62.5(Vergleichswert)
//-----------------------------------------------------------------------------
void AlarmTon(WORD freq)
{
  if (freq>100) {

  cli();
  //DDRB &= ~0b00000100;       // PB2 als Eingang
  TCCR1A =0;                 // Toggle abschalten
  TCCR1B = 0;
  OCR1A = ICR1 = (BYTE)(62500/freq);
  TCCR1A = (1<<COM1B0);      // Toggle /OC1B->PB2/OCnC on compare match. 0b00010000;
  TCCR1B = (1<<CS10) | (1<<CS11) |(1<<WGM12); // clk/64, WGM Mode 4 0b00001011;
  DDRB  |= 0b00000100;       // PB2 als Ausgang 
  sei();

  }
  else
  {
  cli();
    DDRB &= ~0b00000100;     // PB2 als Eingang 
    TCCR1A &= ~(1<<COM1B0);  // Toggle abschalten
    TCCR1B &= ~((1<<CS10) | (1<<CS11) |(1<<WGM12));
  sei();
  }
}


//----------------------------------------------------------------------------------------
//                      E n d   o f   H M I . c 
//----------------------------------------------------------------------------------------
