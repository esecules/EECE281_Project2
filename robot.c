//The main code for the car goes here
#include <stdio.h> 
#include <at89lp51rd2.h>


// ~C51~
#define REF 1
#define TEST 0
#define CLK 22118400L 
#define BAUD 115200L 
#define BRG_VAL (0x100-(CLK/(32L*BAUD)))

//We want timer 0 to interrupt every 100 microseconds ((1/10000Hz)=100 us)
#define FREQ 10000L
#define TIMER0_RELOAD_VALUE (65536L-(CLK/(12L*FREQ)))

//Motor Pins
#define LWHEEL_R P0_0
#define LWHEEL_B P0_1
#define RWHEEL_R P0_2
#define RWHEEL_B P0_3
#define SENSE_LEFT 0
#define SENSE_RIGHT 1
#define FORWARD 1
#define BACK 0
//These variables are used in the ISR
volatile unsigned char pwmcount;
volatile unsigned char pwmL=0;
volatile unsigned int lDirection=0;
volatile unsigned char pwmR=0;
volatile unsigned int rDirection=0;
volatile unsigned char lWheel = 0;
volatile unsigned char rWheel = 0;
int distance = 0;
char park = 0;
int sensativity = 0;

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
	TR0=0; // Stop timer 0
	TMOD=0x01; // 16-bit timer
	// Use the autoreload feature available in the AT89LP51RB2
	// WARNING: There was an error in at89lp51rd2.h that prevents the
	// autoreload feature to work.  Please download a newer at89lp51rd2.h
	// file and copy it to the crosside\call51\include folder.
	TH0=RH0=TIMER0_RELOAD_VALUE/0x100;
	TL0=RL0=TIMER0_RELOAD_VALUE%0x100;
	TR0=1; // Start timer 0 (bit 4 in TCON)
	ET0=1; // Enable timer 0 interrupt
	EA=1;  // Enable global interrupts
	
	pwmcount=0;
    
    return 0;
}
	
void pwmcounter (void) interrupt 1
{
	if(++pwmcount>99) pwmcount=0;
	
	if(lWheel){
		if(lDirection==FORWARD){
			P1_0=(pwmL>pwmcount)?0:1;
			P1_1=1;
		}
	
		if(lDirection==BACK){
			P1_1=(pwmL>pwmcount)?0:1;
			P1_0=1;
		}
	}
	if(rWheel){
		if(rDirection==FORWARD){
			P1_0=(pwmR>pwmcount)?0:1;
			P1_1=1;
		}
	
		if(rDirection==BACK){
			P1_1=(pwmR>pwmcount)?0:1;
			P1_0=1;
		}
	}
}

int getAmplitude(char channel){
	
	return channel;
}

void getCommand(void){
	
	park = 0;
}

void doPark(void){

	park = 0;
}

void main(void){
	
	int rAmp = 0;
	int lAmp = 0;
	while(1){
		rAmp = getAmplitude(SENSE_RIGHT);
		lAmp = getAmplitude(SENSE_LEFT);	
		if(rAmp == 0 && lAmp ==0){
			getCommand();
			if(park) doPark();
		}
		
		if(rAmp < distance + sensativity){
			rDirection = FORWARD;
			rWheel = 1;	
		}
		else if(rAmp > distance - sensativity){
			rDirection = BACK;
			rWheel = 1;	
		}
		else rWheel = 0;
		
		if(lAmp < distance + sensativity){
			lDirection = FORWARD;
			lWheel = 1;	
		}
		else if(lAmp > distance - sensativity){
			lDirection = BACK;
			lWheel = 1;	
		}
		else lWheel = 0;
				
	}
}