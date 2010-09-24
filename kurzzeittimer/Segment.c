//----------------------------------------------------------------------------------------
// Segment.c 7-Segment-Anzeige
//
// Copyright (c) 2010, Bjoern Heller <tec@hellercom.de>. All rights reserved
// This code is licensed under GNU/ GPL
// 
//----------------------------------------------------------------------------------------
#include "Segment.h"

//----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//    a
//    --
//  f|g_|b
//  e|  |c
//    --
//    d
//-----------------------------------------------------------------------------
void ShowOneZiff(BYTE Ziffer, BYTE s, BYTE z)
{
BYTE set;

  if ((Ziffer==0) || (Ziffer==2) || (Ziffer==6) || (Ziffer==8)) { set=1; }
  else                                                          { set=0; }
  Segment_e(s,z,set);

  if ((Ziffer!=1) && (Ziffer!=2) && (Ziffer!=3) && (Ziffer!=7)) { set=1; }
  else                                                          { set=0; }
  Segment_f(s,z,set);

  if ((Ziffer!=1) && (Ziffer!=4)) { set=1; }
  else                            { set=0; }
  Segment_a(s,z,set);
  
  if ((Ziffer!=1) && (Ziffer!=4) && (Ziffer!=7)) { set=1; }
  else                                           { set=0; }
  Segment_d(s,z,set);

  if ((Ziffer!=5) && (Ziffer!=6)) { set=1; }
  else                            { set=0; }
  Segment_b(s,z,set);

  if (Ziffer!=2) { set=1; }
  else           { set=0; }
  Segment_c(s,z,set);

  if ((Ziffer!=0) && (Ziffer!=1) && (Ziffer!=7)) { set=1; }
  else                                           { set=0; }
  Segment_g(s,z,set);
}


//-----------------------------------------------------------------------------
// Segment a
//-----------------------------------------------------------------------------
void Segment_a(BYTE y, BYTE x, BYTE s)
{
  PaintSquare(x+5,y+0,x+13,y+3,s);       // Rechteck

  write_window((x+7+2)/2,y+0,(x+7+2)/2+1,y+2);   // linke Schraege
  if (s) {
    write_dat(0xFF); write_dat(0xFF);
    write_dat(0x0F); write_dat(0xFF);
    write_dat(0x00); write_dat(0xFF);
  }
  else {
    write_dat(0x33); write_dat(0x33);
    write_dat(0x03); write_dat(0x33);
    write_dat(0x00); write_dat(0x33);
  }

  write_window((x+7+15)/2,y+0,(x+7+15)/2+1,y+2); // rechte Schraege
  if (s) {
    write_dat(0xFF); write_dat(0xF0);
    write_dat(0xFF); write_dat(0x00);
    write_dat(0xF0); write_dat(0x00);
  }
  else {
    write_dat(0x33); write_dat(0x30);
    write_dat(0x33); write_dat(0x00);
    write_dat(0x30); write_dat(0x00);
  }
}


//-----------------------------------------------------------------------------
//   Segment b
//-----------------------------------------------------------------------------
void Segment_b(BYTE y, BYTE x, BYTE s) {

  write_window((x+7+18)/2,y+1,(x+7+18)/2+1,y+3); // obere Schraege
  if (s) {
    write_dat(0x00); write_dat(0xF0);
    write_dat(0x0F); write_dat(0xF0);
    write_dat(0xFF); write_dat(0xF0);
  }
  else {
    write_dat(0x00); write_dat(0x30);
    write_dat(0x03); write_dat(0x30);
    write_dat(0x33); write_dat(0x30);
  }

  PaintSquare(x+17,y+4,x+18,y+13,s);   // Rechteck

  write_window((x+7+18)/2,y+14,(x+7+18)/2+1,y+16);         // untere Schraege 
  if (s) {
    write_dat(0xFF); write_dat(0xF0);
    write_dat(0x0F); write_dat(0xF0);
    write_dat(0x00); write_dat(0xF0);
  }
  else {
    write_dat(0x33); write_dat(0x30);
    write_dat(0x03); write_dat(0x30);
    write_dat(0x00); write_dat(0x30);
  }
}


//-----------------------------------------------------------------------------
// Segment c
//-----------------------------------------------------------------------------
void Segment_c(BYTE y, BYTE x, BYTE s)
{

  write_window((x+7+18)/2,y+18,(x+7+18)/2+1,y+20);         // obere Schraege
  if (s) {
    write_dat(0x00); write_dat(0xF0);
    write_dat(0x0F); write_dat(0xF0);
    write_dat(0xFF); write_dat(0xF0);
  }
  else {
    write_dat(0x00); write_dat(0x30);
    write_dat(0x03); write_dat(0x30);
    write_dat(0x33); write_dat(0x30);
  }

  PaintSquare(x+17,y+21,x+18,y+30,s);  // Rechteck

  write_window((x+7+18)/2,y+31,(x+7+18)/2+1,y+33);         // untere Schraege
  if (s) {
    write_dat(0xFF); write_dat(0xF0);
    write_dat(0x0F); write_dat(0xF0);
    write_dat(0x00); write_dat(0xF0);
  }
  else {
    write_dat(0x33); write_dat(0x30);
    write_dat(0x03); write_dat(0x30);
    write_dat(0x00); write_dat(0x30);
  }
}


//-----------------------------------------------------------------------------
// Segment d
//-----------------------------------------------------------------------------
void Segment_d(BYTE y, BYTE x, BYTE s)
{

  PaintSquare(x+5,y+31,x+13,y+34,s);   // Rechteck
  
  write_window((x+7+2)/2,y+32,(x+7+2)/2+1,y+34); // linke Schraege
  if (s) {
    write_dat(0x00); write_dat(0xFF);
    write_dat(0x0F); write_dat(0xFF);
    write_dat(0xFF); write_dat(0xFF);
  }
  else {
    write_dat(0x00); write_dat(0x33);
    write_dat(0x03); write_dat(0x33);
    write_dat(0x33); write_dat(0x33);
  }

  write_window((x+7+15)/2,y+32,(x+7+15)/2+1,y+34);         // rechte Schraege
  if (s) {
    write_dat(0xF0); write_dat(0x00);
    write_dat(0xFF); write_dat(0x00);
    write_dat(0xFF); write_dat(0xF0);
  }
  else {
    write_dat(0x30); write_dat(0x00);
    write_dat(0x33); write_dat(0x00);
    write_dat(0x33); write_dat(0x30);
  }
}


//-----------------------------------------------------------------------------
// Segment e
//-----------------------------------------------------------------------------
void Segment_e(BYTE y, BYTE x, BYTE s)
{
  write_window((x+7)/2,y+18,(x+7)/2+1,y+20);     // obere Schraege
  if (s) {
    write_dat(0xF0); write_dat(0x00);
    write_dat(0xFF); write_dat(0x00);
    write_dat(0xFF); write_dat(0xF0);
  }
  else {
    write_dat(0x30); write_dat(0x00);
    write_dat(0x33); write_dat(0x00);
    write_dat(0x33); write_dat(0x30);
  }
  
  PaintSquare(x+0,y+21,x+1,y+30,s);    // Rechteck

  write_window((x+7)/2,y+31,(x+7)/2+1,y+33);     // untere Schraege
  if (s) {
    write_dat(0xFF); write_dat(0xF0);
    write_dat(0xFF); write_dat(0x00);
    write_dat(0xF0); write_dat(0x00);
  }
  else {
    write_dat(0x33); write_dat(0x30);
    write_dat(0x33); write_dat(0x00);
    write_dat(0x30); write_dat(0x00);
  }
}


//-----------------------------------------------------------------------------
//  Segment f
//-----------------------------------------------------------------------------
void Segment_f(BYTE y, BYTE x, BYTE s) {

  write_window((x+7)/2,y+1,(x+7)/2+1,y+3);       // obere Schraege
  if (s) {
    write_dat(0xF0); write_dat(0x00);
    write_dat(0xFF); write_dat(0x00);
    write_dat(0xFF); write_dat(0xF0);
  }
  else {
    write_dat(0x30); write_dat(0x00);
    write_dat(0x33); write_dat(0x00);
    write_dat(0x33); write_dat(0x30);
  }

  PaintSquare(x+0,y+4,x+1,y+13,s);     // Rechteck

  write_window((x+7)/2,y+14,(x+7)/2+1,y+16);     // untere Schraege
  if (s) {
    write_dat(0xFF); write_dat(0xF0);
    write_dat(0xFF); write_dat(0x00);
    write_dat(0xF0); write_dat(0x00);
  }
  else {
    write_dat(0x33); write_dat(0x30);
    write_dat(0x33); write_dat(0x00);
    write_dat(0x30); write_dat(0x00);
  }
}


//-----------------------------------------------------------------------------
// Segment g
//-----------------------------------------------------------------------------
void Segment_g(BYTE y, BYTE x, BYTE s) {

  PaintSquare(x+4,y+15,x+14,y+19,s);   // Rechteck

  write_window((x+7+2)/2,y+16,(x+7+2)/2,y+18);    // linke Schraege
  if (s) {
    write_dat(0x0F);
    write_dat(0xFF);
    write_dat(0x0F);
  }
  else {
    write_dat(0x03);
    write_dat(0x33);
    write_dat(0x03);
  }

  write_window((x+7+16)/2,y+16,(x+7+16)/2+1,y+18);  // rechte Schraege
  if (s) {
    write_dat(0xFF); write_dat(0x00);
    write_dat(0xFF); write_dat(0xF0);
    write_dat(0xFF); write_dat(0x00);
  }
  else { 
    write_dat(0x33); write_dat(0x00);
    write_dat(0x33); write_dat(0x30);
    write_dat(0x33); write_dat(0x00);
  }
}


//----------------------------------------------------------------------------------------
//  zeichnet ein Rechteck
//              x1 0..101
//              y1 0...79
//
//----------------------------------------------------------------------------------------
void PaintSquare(BYTE x1, BYTE y1, BYTE x2, BYTE y2, BYTE set)
{
  x1+=7;
  x2+=7;

  if ((x1<=x2) && (y1<=y2))
  {
    if ((x1%2)==0) {
      write_cmd(0x24);                 // Rechteck
      write_cmd(x1/2-1);               // x1/2+7);
      write_cmd(y1);                   // y1
      write_cmd(x1/2-1);               // x2/2+7);
      write_cmd(y2);                   // y2
      if (set==1) { write_cmd(0x0F); } // Zeile zeichnen
      else        { write_cmd(0x03); } // Zeile loeschen
    }
    if ((x2%2)==1) {
      write_cmd(0x24);                 // Rechteck
      write_cmd(x2/2+1);               // x1/2+7);
      write_cmd(y1);                   // y1
      write_cmd(x2/2+1);               // x2/2+7);
      write_cmd(y2);                   // y2
      if (set==1) { write_cmd(0xF0); } // Zeile zeichnen
      else        { write_cmd(0x30); } // Zeile loeschen
      if (x2>x1) x2--;
    }

    write_cmd(0x24);                   // Rechteck
    write_cmd(x1/2);                   // x1/2+7);
    write_cmd(y1);                     // y1
    write_cmd(x2/2);                   // x2/2+7);
    write_cmd(y2);                     // y2
    if (set==1) { write_cmd(0xFF); }   // Zeile zeichnen
    else        { write_cmd(0x33); }   // Zeile loeschen
  }
}


//-----------------------------------------------------------------------------
// zeichnet Doppelpunkt
//-----------------------------------------------------------------------------
void DoppelPunkt(BYTE x, BYTE y, BYTE set)
{
    write_cmd(0x24);                   // Rechteck
    write_cmd(x/2);                    // x1/2+7);
    write_cmd(y);                      // y1
    write_cmd(x/2+1);                  // x2/2+7);
    write_cmd(y+3);                    // y2
    if (set==1) { write_cmd(0xFF); }   // Zeile zeichnen
    else        { write_cmd(0x33); }   // Zeile loeschen

    write_cmd(0x24);         // Rechteck
    write_cmd(x/2);          // x1/2+7);
    write_cmd(y+9);          // y1
    write_cmd(x/2+1);        // x2/2+7);
    write_cmd(y+9+3);        // y2
    if (set==1) { write_cmd(0xFF); }   // Zeile zeichnen
    else        { write_cmd(0x33); }   // Zeile loeschen
}


//----------------------------------------------------------------------------------------
//                      E n d   o f   S e g m e n t . c 
//----------------------------------------------------------------------------------------
