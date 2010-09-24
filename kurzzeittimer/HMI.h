// Copyright (c) 2010, Bjoern Heller <tec@hellercom.de>. All rights reserved
// This code is licensed under GNU/ GPL

#ifndef HMI_H
#define HMI_H

//--------------------------------------------------------------------------------------
#include <avr/io.h>
#include <compat/deprecated.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>

#include "Oled.h"
#include "Segment.h"
#include "Font.h"

//--------------------------------------------------------------------------------------
#define BYTE unsigned char
#define WORD unsigned short

BYTE CheckKey(void);

void PressLeftKey(void);
void PressRightKey(void);

void PressCenterKeyXX(void);
void PressCenterKey00(void);
void PressCenterKey01(void);
void PressCenterKey02(void);
void PressCenterKey03(void);

void DisplayScreen(void);
void Screen00(BYTE);         // Menue
void Screen01(BYTE);         // Countdown / Eieruhr / Zahnputztimer
void Screen02(BYTE);         // aktiver Timer
void Screen03(BYTE);         // Setup

void StartCountDown(void);
void StopCounter(void);
void PauseCounter(void);
void ContinueCounter(void);
void SaveSetup(void);
void RestoreSetup(void);
void Setup(void);

void SleepModeOn(void);
void AlarmTon(WORD);

//--------------------------------------------------------------------------------------
BYTE activeScreen, activeScreen_old, StartScreen;
BYTE z0, z1, z2, z3;
volatile BYTE DisplayOnOff;
volatile BYTE Tastensperre,alarm;
volatile BYTE Tast_cnt, Tast_cnt_old;
BYTE markingS00, markingS01, markingS02, markingS03;
volatile int CountDownTime;
volatile BYTE CntDown;

BYTE EmTime;
BYTE EmergencyTime;

volatile WORD TimeEi;        // Eierkochzeit
volatile WORD TimeZahn;      // Zahnputzdauer
volatile WORD TimeCD;        // freier CountDown

#define _30sec 30
volatile BYTE ShutdownCnt;   // 30sec nach letztem Tastendruck und kein aktiver CountDown

//--------------------------------------------------------------------------------------
#endif

