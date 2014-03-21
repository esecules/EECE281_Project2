#include <stdio.h>
#include <stdlib.h>
#include <at89lp51rd2.h>
#include <float.h>
#include "SPI_ADC.h"
#include "TXRX.h"

// ~C51~ 
 
#define CLK 22118400L
#define BAUD 115200L
#define BRG_VAL (0x100-(CLK/(32L*BAUD)))

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
	
	TXRXinit();
    
    return 0;
}

void TXRXinit(void){
	// all port 1 is belong to us
	P1M0=0;	P1M1=0x07;
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