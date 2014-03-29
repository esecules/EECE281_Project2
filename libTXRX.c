#include <stdio.h>
#include <stdlib.h>
#include <at89lp51rd2.h>
#include "TXRX.h"
#include "SPI_ADC.c"

// ~C51~ 
 
#define CLK 22118400L
#define BAUD 115200L
#define BRG_VAL (0x100-(CLK/(32L*BAUD)))


void TXRXinit(void){
	// all port 1 is belong to us`
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
	#ifndef RXMODE
	if(xOn){
		H1=!H1;
		H2=!H2;
	}
	#endif
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
	unsigned char temp;
	//DATAOUT = 0;
	if(RXTEST){
		//DATAOUT = 1;
		return RXTESTPIN?1:0;
	}else{
		temp = ((GetADC(0))>idealAmp/5)?1:0;
		//DATAOUT = 1;
		return temp;
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
	ET0 = 0;
	/*
	t1reset();
	xmtrOff();
	while(datatime<BITTIME*2);
	t1reset();
	xmtrOn();
	while(datatime<BITTIME*2);
	*/
	tByte(STARTBYTE);
	tByte(dadata);
	tByte(crc_table[dadata]);
	ET0 = 1;
}

unsigned char rByte(void){
	unsigned char rxdata=0;
	char i=0;
	t1reset();
	while(rcvr()){
		if(datatime>10*BITTIME)
			return 0xff;
	}
	wait(BITTIME*4/3);
	for(i=0; i<8; i++){
		t1reset();
		rxdata=(rxdata>>1)+(rcvr()<<7);
		while(datatime<BITTIME);
	}
	return rxdata;
}
	
unsigned char rData(void){
	unsigned char rxdata[2];
	
	//while(rcvr()==0);
	//while(rcvr()==1);
	rxdata[0]=rByte();
	if(rxdata[0]!=STARTBYTE){
		printf("NOT STARTBYTE (0x%x)\n",rxdata[0]);
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