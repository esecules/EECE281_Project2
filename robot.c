//The main code for the car goes here
#include <stdio.h> 
#include <at89lp51rd2.h>
#include "utilities.c"

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
#define RWHEEL_B	P3_6
#define RWHEEL_R	P3_7
#define LWHEEL_B	P3_4
#define LWHEEL_R	P3_5
#define SENSE_LEFT 	0
#define SENSE_RIGHT 1
#define FORWARD 	1
#define BACK 		0
#define CLOCK		1
#define C_CLOCK		0

//Commands
#define NONE			0	
#define PARK			24
#define ROT180			28
#define CRANE_UP		18
#define CRANE_DOWN		20
#define MOVE_LEFT		6
#define MOVE_RIGHT		8
#define MOVE_BACK		4
#define MOVE_FORWARD	2
#define MOVE_FR			10
#define MOVE_FL			12
#define MOVE_BR			14
#define MOVE_BL			16
#define MANUAL_DRIVE	32
#define RETRACE			30
//Increment for distance
#define STEP 			20
#define MAX_DISTANCE	200
#define MIN_DISTANCE	10
//Car Dimensions (in centimeters)
#define WHEEL_CIRCUMFERENCE	21.0
#define SEC_ROT			0.96
//These variables are used in the ISR
volatile unsigned char pwmcount;
volatile unsigned char pwmL=0;
volatile unsigned int lDirection=0;
volatile unsigned char pwmR=0;
volatile unsigned int rDirection=0;
volatile unsigned char lWheel = 0;
volatile unsigned char rWheel = 0;
int distance = 10;
int command = 0;
int sensativity = 0;
volatile unsigned long timer = 0;
volatile unsigned int timercount = 0;
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
	if(++timercount > 999){
		timercount=0;
		timer++;
	}
	
	if(lWheel){
		if(lDirection==FORWARD){
			LWHEEL_R=(pwmL>pwmcount)?0:1;
			LWHEEL_B=1;
		}
	
		if(lDirection==BACK){
			LWHEEL_B=(pwmL>pwmcount)?0:1;
			LWHEEL_R=1;
		}
	}
	if(rWheel){
		if(rDirection==FORWARD){
			RWHEEL_R=(pwmR>pwmcount)?0:1;
			RWHEEL_B=1;
		}
	
		if(rDirection==BACK){
			RWHEEL_B=(pwmR>pwmcount)?0:1;
			RWHEEL_R=1;
		}
	}
}

void SPIWrite( unsigned char value) 
{ 
	SPSTA&=(~SPIF); // Clear the SPIF flag in SPSTA 
	SPDAT=value; 
	while((SPSTA & SPIF)!=SPIF); //Wait for transmission to end 
}
unsigned int getADC(unsigned char channel) 
{ 
	unsigned int adc;
	
	// initialize the SPI port to read the MCP3004 ADC attached to it. 
	SPCON&=(~SPEN); // Disable SPI 
	SPCON=MSTR|CPOL|CPHA|SPR1|SPR0|SSDIS; 
	SPCON|=SPEN; // Enable SPI
	P1_4=0; // Activate the MCP3004 ADC. 
	SPIWrite(channel|0x18); // Send start bit, single/diff* bit, D2, D1, and D0 bits. 
	for(adc=0; adc<10; adc++){}; // Wait for S/H to setup 
	SPIWrite(0x55); // Read bits 9 down to 4 
	adc=((SPDAT&0x3f)*0x100); 
	SPIWrite(0x55); // Read bits 3 down to 0 
	P1_4=1; // Deactivate the MCP3004 ADC. 
	adc+=(SPDAT&0xf0); // SPDR contains the low part of the result. 
	adc>>=4;
	
	return adc;
}



void moveCrane(char direction){
	
}

void rotate(char direction, int angle){
	timer = 0;
	switch (direction){
		case CLOCK:
			rDirection = BACK;
			lDirection = FORWARD;
			pwmL = 50;
			pwmR = 50;
			rWheel = 1;
			lWheel = 1;
			break;
		case C_CLOCK:
			rDirection = FORWARD;
			lDirection = BACK;
			pwmL = 50;
			pwmR = 50;
			rWheel = 1;
			lWheel = 1;
			break;
		default:
			rWheel = 0;
			lWheel = 0;
			break;
		}
	while( timer < (angle/360.0)*WHEEL_CIRCUMFERENCE*SEC_ROT*2.35){} ;
		rWheel = 0;
		lWheel = 0;
	
}

//distance must be in centimeters
void moveDistance (double distance, char direction) {
	rDirection = direction;
	lDirection = direction;
	pwmL = 50;
	pwmR = 50;
	timer = 0;
	rWheel = 1;
	lWheel = 1;
	while(timer < (4.0/3.0*21.0*distance/WHEEL_CIRCUMFERENCE)/SEC_ROT){};
	rWheel = 0;
	lWheel = 0;
}


void doPark(void){

	
	rotate(C_CLOCK,45);
	moveDistance(23.0,BACK);
	rotate(CLOCK,40);
}

unsigned char getCommand ( int min ){
	unsigned char j, val;
	int v;
	EA = 0;
	//Skip the start bit
	val=0;
	wait_one_and_half_bit_time();
	for(j=0; j<8; j++)
	{
		v=getADC(0);
		val|=(v>min)?(0x01<<j):0x00;
		wait_bit_time();
	}
	//Wait for stop bits
	wait_one_and_half_bit_time();
	EA = 1;
	return val;
}
void test(void){
	int counter = 0;
	while( 1 )
	{
	
	moveDistance(25.0, BACK);
	moveDistance(25.0, FORWARD);
	
	}
	
	
}
void doManualDrive(){
	int rAmp = 0;
	int lAmp = 0;
	int command = NONE;
	while(1){
		rAmp = getADC(SENSE_RIGHT);
		lAmp = getADC(SENSE_LEFT);	
		if(rAmp == 0 && lAmp ==0){
			command = getCommand(0);
			
			switch(command){
				case PARK:
					doPark();
					break;
				case ROT180:
					rotate(180,CLOCK);
					break;
				case CRANE_UP:
					moveCrane(CRANE_UP);
					break;
				case CRANE_DOWN:
					moveCrane(CRANE_DOWN);
					break;
				case MANUAL_DRIVE:
					return;
				case MOVE_RIGHT:
					rDirection = BACK;
					lDirection = FORWARD;
					pwmL = 100;
					pwmR = 100;
					rWheel = 1;
					lWheel = 1;
					break;
				case MOVE_LEFT:
					rDirection = FORWARD;
					lDirection = BACK;
					pwmL = 100;
					pwmR = 100;
					rWheel = 1;
					lWheel = 1;
					break;
				case MOVE_BACK:
					rDirection = BACK;
					lDirection = BACK;
					pwmL = 100;
					pwmR = 100;
					rWheel = 1;
					lWheel = 1;
					break;
				case MOVE_FORWARD:
					rDirection = FORWARD;
					lDirection = FORWARD;
					pwmL = 100;
					pwmR = 100;
					rWheel = 1;
					lWheel = 1;
					break;
				case MOVE_FR:
					rDirection = FORWARD;
					lDirection = FORWARD;
					pwmL = 100;
					pwmR = 75;
					rWheel = 1;
					lWheel = 1;
				case MOVE_FL:
					rDirection = FORWARD;
					lDirection = FORWARD;
					pwmL = 75;
					pwmR = 100;
					rWheel = 1;
					lWheel = 1;
				case MOVE_BR:
					rDirection = BACK;
					lDirection = BACK;
					pwmL = 100;
					pwmR = 75;
					rWheel = 1;
					lWheel = 1;
				case MOVE_BL:
					rDirection = BACK;
					lDirection = BACK;
					pwmL = 75;
					pwmR = 100;
					rWheel = 1;
					lWheel = 1;
				default:
					rWheel = 0;
					lWheel = 0;
					break;
			}
		}
		else command = NONE;
	}
}
void main(void){
	
	int rAmp = 0;
	int lAmp = 0;
	int tempR, tempL;
	int command = NONE;
	doPark();
	while(1){
		rAmp = getADC(SENSE_RIGHT);
		lAmp = getADC(SENSE_LEFT);	
		printf("distance %d, sensitivity %d, ramp %d, lamp %d\n", distance, sensativity, rAmp, lAmp);
		if(rAmp == 0 && lAmp ==0){
			command = getCommand(0);
			
			switch(command){
				case PARK:
					doPark();
					break;
				case ROT180:
					rotate(180,CLOCK);
					break;
				case MOVE_FORWARD:
					distance -= STEP;
					if(distance < 0) distance = 0;
					break;
				case MOVE_BACK:
					distance += STEP;
					if(distance > MAX_DISTANCE) distance = MAX_DISTANCE;
					break;
				case CRANE_UP:
					moveCrane(CRANE_UP);
					break;
				case CRANE_DOWN:
					moveCrane(CRANE_DOWN);
					break;
				case MANUAL_DRIVE:
					doManualDrive();
					break;
						
			}
			command = NONE;
		}
		
		else{
			if(rAmp < distance + sensativity){
				rDirection = FORWARD;
				tempR = 1;	
			}
			else if(rAmp > distance - sensativity){
				rDirection = BACK;
				tempR = 1;	
			}
			else tempR = 0;
			
			if(lAmp < distance + sensativity){
				lDirection = FORWARD;
				tempL = 1;	
			}
			else if(lAmp > distance - sensativity){
				lDirection = BACK;
				tempL = 1;	
			}
			else tempL = 0;
			pwmR = 50;
			pwmL = 50;
			rWheel = tempR;
			lWheel = tempL;
		}	
	}
}