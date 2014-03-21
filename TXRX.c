#include <stdio.h>
#include <stdlib.h>
#include <at89lp51rd2.h>
#include <float.h>
#include "SPI_ADC.h"

// ~C51~ 
 
#define CLK 22118400L
#define BAUD 115200L
#define BRG_VAL (0x100-(CLK/(32L*BAUD)))

#define FREQ 30600L
#define TIMER1_RELOAD_VALUE (65536L-(CLK/(12L*FREQ)))

#define H1 P1_0
#define H2 P1_1
#define DATAOUT P1_2

#define RXMODE 1
#define RXTEST 1
#define RXTESTPIN P1_3

#define BITTIME 300
#define STARTBYTE 0xff

volatile unsigned long int datatime;
volatile unsigned char xOn;

/**
* A lookup table for CRC-8, poly = x^8 + x^2 + x^1 + x^0, init = 0.
* https://github.com/SvenTo/AndroidRCCar/blob/master/AndroidRCCar.Arduino/AndriodRCCar/CRC8.h
*/
const unsigned char code crc_table[] = {
0x00,0x07,0x0E,0x09,0x1C,0x1B,0x12,0x15,0x38,
0x3F,0x36,0x31,0x24,0x23,0x2A,0x2D,0x70,0x77,
0x7E,0x79,0x6C,0x6B,0x62,0x65,0x48,0x4F,0x46,
0x41,0x54,0x53,0x5A,0x5D,0xE0,0xE7,0xEE,0xE9,
0xFC,0xFB,0xF2,0xF5,0xD8,0xDF,0xD6,0xD1,0xC4,
0xC3,0xCA,0xCD,0x90,0x97,0x9E,0x99,0x8C,0x8B,
0x82,0x85,0xA8,0xAF,0xA6,0xA1,0xB4,0xB3,0xBA,
0xBD,0xC7,0xC0,0xC9,0xCE,0xDB,0xDC,0xD5,0xD2,
0xFF,0xF8,0xF1,0xF6,0xE3,0xE4,0xED,0xEA,0xB7,
0xB0,0xB9,0xBE,0xAB,0xAC,0xA5,0xA2,0x8F,0x88,
0x81,0x86,0x93,0x94,0x9D,0x9A,0x27,0x20,0x29,
0x2E,0x3B,0x3C,0x35,0x32,0x1F,0x18,0x11,0x16,
0x03,0x04,0x0D,0x0A,0x57,0x50,0x59,0x5E,0x4B,
0x4C,0x45,0x42,0x6F,0x68,0x61,0x66,0x73,0x74,
0x7D,0x7A,0x89,0x8E,0x87,0x80,0x95,0x92,0x9B,
0x9C,0xB1,0xB6,0xBF,0xB8,0xAD,0xAA,0xA3,0xA4,
0xF9,0xFE,0xF7,0xF0,0xE5,0xE2,0xEB,0xEC,0xC1,
0xC6,0xCF,0xC8,0xDD,0xDA,0xD3,0xD4,0x69,0x6E,
0x67,0x60,0x75,0x72,0x7B,0x7C,0x51,0x56,0x5F,
0x58,0x4D,0x4A,0x43,0x44,0x19,0x1E,0x17,0x10,
0x05,0x02,0x0B,0x0C,0x21,0x26,0x2F,0x28,0x3D,
0x3A,0x33,0x34,0x4E,0x49,0x40,0x47,0x52,0x55,
0x5C,0x5B,0x76,0x71,0x78,0x7F,0x6A,0x6D,0x64,
0x63,0x3E,0x39,0x30,0x37,0x22,0x25,0x2C,0x2B,
0x06,0x01,0x08,0x0F,0x1A,0x1D,0x14,0x13,0xAE,
0xA9,0xA0,0xA7,0xB2,0xB5,0xBC,0xBB,0x96,0x91,
0x98,0x9F,0x8A,0x8D,0x84,0x83,0xDE,0xD9,0xD0,
0xD7,0xC2,0xC5,0xCC,0xCB,0xE6,0xE1,0xE8,0xEF,
0xFA,0xFD,0xF4,0xF3
};

void xmtrOn(void);
void xmtrOff(void);
void TXRXinit(void);

unsigned char _c51_external_startup(void)
{
	// Configure ports as a bidirectional with internal pull-ups.
	P0M0=0;	P0M1=0;
	P1M0=0;	P1M1=0x07;
	P2M0=0;	P2M1=0;
	P3M0=0;	P3M1=0;
	AUXR=0B_0001_0001; // 1152 bytes of internal XDATA, P4.4 is a general purpose I/O
	P4M0=0;	P4M1=0;
    
    // Initialize the serial port and baud rate generator
    PCON|=0x80;
	SCON = 0x52;
    BDRCON=0;
    BRL=BRG_VAL;
    BDRCON=BRR|TBCK|RBCK|SPD;
	
	// Init H Bridge control pins
	H1=0;
	H2=1;
	
	TR1=0; // Stop timer 0
	TMOD&=0x1f; // 16-bit timer
	TMOD|=0x10;
	// Use the autoreload feature available in the AT89LP51RB2
	// WARNING: There was an error in at89lp51rd2.h that prevents the
	// autoreload feature to work.  Please download a newer at89lp51rd2.h
	// file and copy it to the crosside\call51\include folder.
	TH1=RH1=TIMER1_RELOAD_VALUE/0x100;
	TL1=RL1=TIMER1_RELOAD_VALUE%0x100;
	TR1=1; // Start timer 0 (bit 4 in TCON)
	ET1=1; // Enable timer 0 interrupt
	EA=1;  // Enable global interrupts
    
    datatime=0;
    xmtrOn();
    
    return 0;
}

void TXRXinit(void){
	P1M0=0;	P1M1=0x07;
	H1=0;
	H2=1;
	
	TR1=0; // Stop timer 0
	TMOD&=0x1f; // 16-bit timer
	TMOD|=0x10;
	// Use the autoreload feature available in the AT89LP51RB2
	// WARNING: There was an error in at89lp51rd2.h that prevents the
	// autoreload feature to work.  Please download a newer at89lp51rd2.h
	// file and copy it to the crosside\call51\include folder.
	TH1=RH1=TIMER1_RELOAD_VALUE/0x100;
	TL1=RL1=TIMER1_RELOAD_VALUE%0x100;
	TR1=1; // Start timer 0 (bit 4 in TCON)
	ET1=1; // Enable timer 0 interrupt
	EA=1;  // Enable global interrupts
    
    datatime=0;
    xmtrOn();
}

void isr1 (void) interrupt 3
{
	if(xOn){
		H1=!H1;
		H2=!H2;
	}
	
	datatime++;
}

void t1reset(void){
	TL1=RL1;
	TH1=RH1;
	datatime=0;
}

void xmtrOn(void){
	if(!xOn){
		H1=1;
		H2=0;
		xOn=1;
		DATAOUT=1;
	}	
}

void xmtrOff(void){
	xOn=0;
	H1=0;
	H2=0;
	DATAOUT=0;
}

unsigned char rcvr(void){
	if(RXTEST){
		return RXTESTPIN?1:0;
	}else{
		return (GetADC(0)>20)?1:0;
	}
}

void wait(unsigned long int time){
	t1reset();
	while(datatime<time);
}

void tByte(unsigned char dabyte){
	char i=0;
	xmtrOff();
	wait(BITTIME);
	for(i=0; i<8; i++){
		t1reset();
		if(dabyte&1){
			xmtrOn();
		}else{
			xmtrOff();
		}
		dabyte=dabyte>>1;
		while(datatime<BITTIME);
	}
	xmtrOn();
	wait(BITTIME);
	wait(BITTIME);
}

void tData(unsigned char dadata){
	tByte(STARTBYTE);
	tByte(dadata);
	tByte(crc_table[dadata]);
}

unsigned char rByte(void){
	unsigned char rxdata=0;
	char i=0;
	while(rcvr());
	wait(BITTIME*3/2);
	for(i=0; i<8; i++){
		t1reset();
		rxdata=(rxdata>>1)+(rcvr()<<7);
		while(datatime<BITTIME);
	}
	return rxdata;
}
	
unsigned char rData(void){
	unsigned char rxdata[2];
	if(rByte()!=STARTBYTE){
		printf("NOT STARTBYTE\n");
		return STARTBYTE;
	}
	
	rxdata[0]=rByte();
	rxdata[1]=rByte();
	if(rxdata[1]==crc_table[rxdata[0]]){
		return rxdata[0];
	}else{
		printf("CHECKSUM WRONG %x SHOULD BE %x\n", rxdata[1], crc_table[rxdata[0]]);
		return STARTBYTE;
	}
}
		

void main(void){
	unsigned char dadata=0;
	unsigned char i=0;
	char code teststr[]="A QUICK BROWN FOX JUMPED OVER THE LAZY DOG\na quick brown fox jumped over the lazy dog\n`1234567890-=~!@#$%^&*()_+[]\\{}|;':\",./<>?\n";
	
	
	if(RXMODE){
		printf("RX MODE");
		while(1){
			if(!(int)rcvr()){
			dadata=rData();
			printf("%c",dadata);
			}
		}
	}else{
		while(1){
				/*
				printf("%x %x %x\n", STARTBYTE, dadata, crc_table[dadata]);
				tData(dadata);
				dadata++;
				*/
				while(teststr[i]!=0){
					tData(teststr[i]);
					i++;
				}
				i=0;
		}
	}
}