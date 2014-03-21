#include <stdio.h>
#include <at89lp51rd2.h>
#include "TXRX.h"
#define RXMODE 0

#define CLK 22118400L
#define BAUD 115200L
#define BRG_VAL (0x100-(CLK/(32L*BAUD)))

/*
#define  NONE	  0
#define  UP	  2  
#define  DOWN   4  
#define  LEFT   6 
#define  RIGHT   8 
#define  UPRIGHT  10 
#define  UPLEFT   12 
#define  DOWNRIGHT   14 
#define  DOWNLEFT    16 
#define  R1	  18 
#define  R2	  20  
#define  X	  22 
#define  TRIANGLE   24 
#define  SQUARE   26 
#define  CIRCLE   28 
#define  START   30 
#define  SELECT   32 
#define command  0
*/

#define  NONE	  0
#define  UP	  1  
#define  DOWN   2  
#define  LEFT   3 
#define  RIGHT   4
#define  UPRIGHT  5 
#define  UPLEFT   6 
#define  DOWNRIGHT   7 
#define  DOWNLEFT    8 
#define  R1	  9 
#define  R2	  10  
#define  X	  11 
#define  TRIANGLE   12 
#define  SQUARE   13 
#define  CIRCLE   14 
#define  START   15 
#define  SELECT   16 

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

//Utilities file for functions like wait
void wait_bit_time (void)
{
	_asm
	;For a 22.1184MHz crystal one machine cycle 
	;takes 12/22.1184MHz=0.5425347us
	
	 mov R1, #250
	N2:
	 mov R0, #184
	N1:
	djnz R0, N1 ; //2 machine cycles-> 2*0.5425347us*184=200us
	djnz R1, N2 ; //200us*250=0.05s

	ret
	_endasm;
}
void wait_one_and_half_bit_time (void)
{
	_asm
	;For a 22.1184MHz crystal one machine cycle 
	;takes 12/22.1184MHz=0.5425347us

	 mov R2, #3
	M3:
	 mov R1, #125
	M2:
	 mov R0, #184
	M1: 
	djnz R0, M1 ; 2 machine cycles-> 2*0.5425347us*184=200us
	djnz R1, M2 ; 200us*125=0.025s
	DJNZ R2, M3 ; 3*0.025s= 0.075s

	ret
	_endasm;
}

unsigned char read4channels()
{
	unsigned char val;
	
	val = 0;

	
	if(P3_7 == 1) 
	{
		val|=(0x01<<0);
	}
	else if (P3_7 == 0) 
	{
		val|=0x00;
	}
	
	if(P3_6 == 1) 
	{
		val|=(0x01<<1);
	}
	else if (P3_6 == 0) 
	{
		val|=0x00;
	}
	if(P3_5 == 1) 
	{
		val|=(0x01<<2);
	}
	else if (P3_5 == 0) 
	{
		val|=0x00;
	}
	if(P3_4 == 1) 
	{
		val|=(0x01<<3);
	}
	else if (P3_4 == 0) 
	{
		val|=0x00;
	}
	return val;
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

//Read
/*
unsigned char rx_byte () 
{ 
	unsigned char j, val; 

	//Skip the start bit 
	val=0; 
	wait_one_and_half_bit_time(); 
	
	for(j=0; j<8; j++) 
	{
		if(P1_7==1) {
			val|=(0x01<<j);
		}
		else if (P1_7==0) {
			val|=0x00;
		}
		
		
	wait_bit_time(); 
	}
	
	wait_one_and_half_bit_time(); 
	return val;
}
*/



void main (void)
{	
	unsigned char command;
	
	while(1){
		//wait for signal to be zero 
		command = read4channels();
		//command = rx_byte(); 
		printf("%u", (unsigned) command);
		
		tData(command);
				
		if(command == DOWNRIGHT)
		{
			printf("DOWNRIGHT\r\n");
			command=0;
		}
		else if(command == DOWNLEFT)
		{
			printf("DOWNLEFT\r\n");
			command=0;
		}
		else if(command == UPRIGHT)
		{
			printf("UPRIGHT\r\n");
			command=0;
		}
		else if(command == UPLEFT)
		{
			printf("UPLEFT\r\n");
			command=0;
		}		
		else if(command == UP)
		{	
			printf("UP\r\n");
			command=0;
		}
		else if(command == DOWN)
		{
			printf("DOWN\r\n");
			command=0;
		}
		else if(command == LEFT)
		{
			printf("LEFT\r\n");
			command=0;
		}
		else if(command == RIGHT)
		{
			printf("RIGHT\r\n");
			command=0;
		}
		else if(command == SELECT)
		{
			printf("Select Pressed - Change Modes\r\n");
			command=0;
		}
		else if(command == START)
		{
			printf("START\r\n");
			command=0;
		}
		else if(command == X)
		{
			printf("X\r\n");
			command=0;
		}
		else if(command == TRIANGLE)
		{
			printf("TRIANGLE\r\n");
			command=0;
		}
		else if(command == SQUARE)
		{
			printf("SQUARE\r\n");
			command=0;
		}
		else if(command == CIRCLE)
		{
			printf("CIRCLE\r\n");
			command=0;
		}
		else if(command == R1)
		{
			printf("R1\r\n");
			command=0;
		}
		else if(command == R2)
		{
			printf("R2\r\n");
			command=0;
		}
		else
		{
			command=0;
		}
	}
}
