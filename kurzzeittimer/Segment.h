// Copyright (c) 2010, Bjoern Heller <tec@hellercom.de>. All rights reserved
// This code is licensed under GNU/ GPL

#ifndef SEGMENT_H
#define SEGMENT_H

//--------------------------------------------------------------------------------------
#include <avr/io.h>
#include <compat/deprecated.h>
#include <avr/interrupt.h>
#include "Font.h"
#include "Oled.h"

//--------------------------------------------------------------------------------------
#define BYTE unsigned char
#define WORD unsigned short

//--------------------------------------------------------------------------------------
void Show4Digits(int, BYTE, BYTE);
void Segment_a(BYTE, BYTE, BYTE);
void Segment_b(BYTE, BYTE, BYTE);
void Segment_c(BYTE, BYTE, BYTE);
void Segment_d(BYTE, BYTE, BYTE);
void Segment_e(BYTE, BYTE, BYTE);
void Segment_f(BYTE, BYTE, BYTE);
void Segment_g(BYTE, BYTE, BYTE);
void ShowOneZiff(BYTE Ziffer, BYTE s, BYTE z);

void PaintSquare(BYTE x1, BYTE y1, BYTE x2, BYTE y2, BYTE set);
void DoppelPunkt(BYTE x, BYTE y, BYTE set);

//--------------------------------------------------------------------------------------
#endif
