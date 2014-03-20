#include <stdio.h>
#include <stdlib.h>
#include <at89lp51rd2.h>

#include "common.h"
#include "TXRX.h"





void t0reset(void){
	TL0=RL0;
	TH0=RH0;
	datatime=0;
}

void xmtrOn(void){
	H1=1;
	H2=0;
	xOn=1;
	DATAOUT=1;
	
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
		return 0; //WIP
	}
}

void wait(unsigned long int time){
	t0reset();
	while(datatime<time);
}

void tByte(unsigned char dabyte){
	char i=0;
	xmtrOff();
	wait(BITTIME);
	for(i=0; i<8; i++){
		t0reset();
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
		t0reset();
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