#include <stdio.h> 
#include <at89lp51rd2.h>
#include "libTXRX.c"
#define RXMODE 1

// ~C51~
#define REF 1
#define TEST 0
#define CLK 22118400L 
#define BAUD 115200L 
#define BRG_VAL (0x100-(CLK/(32L*BAUD)))

//We want timer 0 to interrupt every 100 microseconds ((1/10000Hz)=100 us)
#define FREQ 10000L
#define TIMER0_RELOAD_VALUE (65536L-(CLK/(12L*FREQ)))

#define T2FREQ 1000L
#define T2RLD (65536-CLK/12/T2FREQ)

unsigned char _c51_external_startup(void) 
{ 
	// Configure ports as a bidirectional with internal pull-ups.
	P0M0=0;	P0M1=0;
	P1M0=0;	P1M1=0;
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
	
	// Initialize timer 0 for ISR 'pwmcounter()' below
	//TR0=0; // Stop timer 0
	//TMOD=0x01; // 16-bit timer
	// Use the autoreload feature available in the AT89LP51RB2
	// WARNING: There was an error in at89lp51rd2.h that prevents the
	// autoreload feature to work.  Please download a newer at89lp51rd2.h
	// file and copy it to the crosside\call51\include folder.
	//TH0=RH0=TIMER0_RELOAD_VALUE/0x100;
	//TL0=RL0=TIMER0_RELOAD_VALUE%0x100;
	//TR0=1; // Start timer 0 (bit 4 in TCON)
	//ET0=1; // Enable timer 0 interrupt
	
	
	IPH0=0b00001000;
	IPL0=0b00100000;
	IPH1=0b00000100;
	IPL1=0b00000100;
	
	TR2=0;
	RCAP2H=T2RLD/256;
	RCAP2L=T2RLD%256;
	ET2=1;
	TR2=1;
	
    TXRXinit();
    EA=1;  // Enable global interrupts
    return 0;
}

void main(void)
{
	int right = 0;
	int left = 0;
	int command = 0;
	
	while (1)
	{
		right = GetADC(1);
		left = GetADC(0);
		
		if (right <= 3 && left <= 3)
			continue;
			
		command = rData();
		DATAPORTS = command;
		READYPORT = 1;
		while(READPORT == 0);
		READYPORT = 0;
	}
}
	
