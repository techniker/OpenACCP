# Copyright (c) 2010, Bjoern Heller <tec@hellercom.de>. All rights reserved
# This code is licensed under GNU/ GP

#include "OLED_Driver.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include "stdlib.h"
#include "stdio.h"
#include "string.h"


#define	  RESET_low				(PORTC &= ~(0x04))
#define   RESET_high    		(PORTC |=   0x04)
#define	  CS_low				(PORTB &= ~(0x02))
#define   CS_high				(PORTB |=   0x02)
#define   VCC_Display_enable   	(PORTD |=   0x20)
#define   VCC_Display_disable   (PORTD &= ~(0x20))
#define   DC_low				(PORTC &= ~(0x02))
#define   DC_high				(PORTC |=   0x02)



int main (void)
{

	SPI_Init();
	General_Init();
	OLED_Init();
	blank();
	anzeige();
	while(1);

}

void General_Init()
{
	
	
	DDRC	|=	0x06;
	DC_low;
	RESET_high;

	DDRB	|=	0x02;
	CS_high;

	DDRD 	|=  0x30;
	VCC_Display_enable;

	
	return;
		
}



void SPI_Init()
{
//	UBRR0 	= 	0x00;				//Baudratengenerator aus

	UCSR0C 	= 	0xC3;				//SPI enable, MSB first, SPI Mode 3

	UCSR0B 	= 	0x08;				//TxD enable

//	UBRR0 	=	0x0066;				//Baudrate einstellen
	
	return;
}

void OLED_Init()
{
		
	Send_Command(0xA4);				//0xA5 alle Pixel an, 0xA6 alle Pixel aus, 0xA4 Normal Mode, 0x47 Display invert
	Send_Command(0xAF);				//0xAF Display an, 0xAE Display aus





	return;
		
	
}

//Ram clearen
void blank(void)
{
unsigned int i,j;
for (j=0;j<80;j++) // 80 row
{
//CS_low;
//DC_high;
//WRT = 0;
for (i=0;i<64;i++) // 128 column (1 byte = 2col ) 64
{
Send_Data(0x00);
}
//WRT = 1;
}

// Column Address
Send_Command(0x15); /* Set Column Address */
Send_Command(0x00); /* Start = 0 */
Send_Command(0x3F); /* End = 127 */
// Row Address
Send_Command(0x75); /* Set Row Address */
Send_Command(0x00); /* Start = 0 */
Send_Command(0x3F); /* End = 63 */
// Contrast Control
Send_Command(0x81); /* Set Contrast Control (1) */
Send_Command(0x66); /* 0 ~ 127 */
// Current Range
Send_Command(0x86); /* Set Current Range 84h:Quarter, 85h:Half, 86h:Full*/
// Re-map
Send_Command(0xA0); /* Set Re-map */
Send_Command(0x41); /* [0]:MX, [1]:Nibble, [2]:H/V address [4]:MY, [6]:Com Split Odd/Even "1000010"*/
// Display Start Line
Send_Command(0xA1); /* Set Display Start Line */
Send_Command(0x00); /* Top */
// Display Offset
Send_Command(0xA2); /* Set Display Offset */
Send_Command(0x44); /* Offset 76 rows */
// Display Mode
Send_Command(0xA4); /* Set DisplaMode,A4:Normal, A5:All ON, A6: All OFF, A7:Inverse */
// Multiplex Ratio
Send_Command(0xA8); /* Set Multiplex Ratio */
Send_Command(0x3F); /* 64 mux*/
// Phase Length
Send_Command(0xB1); /* Set Phase Length */
Send_Command(0x22); /* [3:0]:Phase 1 period of 1~16 clocks */
/* [7:4]:Phase 2 period of 1~16 clocks  POR = 0111 0100 */
// Row Period
Send_Command(0xB2); /* Set Row Period */
Send_Command(0x46); /* [7:0]:18~255, K=P1+P2+GS15 (POR:4+7+29)*/
// Display Clock Divide
Send_Command(0xB3); /* Set Clock Divide (2) */
Send_Command(0xF1); /* [3:0]:1~16, [7:4]:0~16, 70Hz */
/* POR = 0000 0001 */
// VSL
Send_Command(0xBF); /* Set VSL */
Send_Command(0x0D); /* [3:0]:VSL */
// VCOMH
Send_Command(0xBE); /* Set VCOMH (3) */
Send_Command(0x02); /* [7:0]:VCOMH, (0.51 X Vref = 0.51 X 12.5 V = 6.375V)*/
// VP
Send_Command(0xBC); /* Set VP (4) */
Send_Command(0x10); /* [7:0]:VP, (0.67 X Vref = 0.67 X 12.5 V = 8.375V) */
// Gamma
Send_Command(0xB8); /* Set Gamma with next 8 bytes */
Send_Command(0x01); /* L1[2:1] */
Send_Command(0x11); /* L3[6:4], L2[2:0] 0001 0001 */
Send_Command(0x22); /* L5[6:4], L4[2:0] 0010 0010 */
Send_Command(0x32); /* L7[6:4], L6[2:0] 0011 1011 */
Send_Command(0x43); /* L9[6:4], L8[2:0] 0100 0100 */
Send_Command(0x54); /* LB[6:4], LA[2:0] 0101 0101 */
Send_Command(0x65); /* LD[6:4], LC[2:0] 0110 0110 */
Send_Command(0x76); /* LF[6:4], LE[2:0] 1000 0111 */
// Set DC-DC
Send_Command(0xAD); /* Set DC-DC */
Send_Command(0x02); /* 03=ON, 02=Off */
// Display ON/OFF
Send_Command(0xAF); /* AF=ON, AE=Sleep Mode */

return;
}
//++++++++++++++++++++++++++


void anzeige(void) {
unsigned int i,j;
for (j=0;j<70;j++) // 80 row 80   70
{
//CS_low;
//DC_high;
//WRT = 0;
for (i=0;i<64;i++) // 128 column (1 byte = 2col ) 64
{
Send_Data(0xFF); //50 dot leer dot
}
Send_Data(0x55);
}




}








void Send_Command(unsigned char data)
{

	CS_low;
	DC_low;

	while ( !( UCSR0A & (1<<UDRE0)) );
	UDR0 = data;


	
	return;
}

void Send_Data(unsigned char data)
{
	unsigned short j;

	CS_low;
	DC_high;

	for(j=0;j<0x00ff;j++);


	while ( !( UCSR0A & (1<<UDRE0)) );
	UDR0 = data;

	return;
}
		
