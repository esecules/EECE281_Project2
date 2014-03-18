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
#define LWHEEL_R	P0_0
#define LWHEEL_B	P0_1
#define RWHEEL_R	P0_2
#define RWHEEL_B	P0_3
#define SENSE_LEFT 	0
#define SENSE_RIGHT 1
#define FORWARD 	1
#define BACK 		0

//Commands
#define NONE			0	
#define PARK			1
#define ROT180			2
#define AWAY			3
#define CLOSER			4
#define CRANE_UP		5
#define CRANE_DOWN		6
#define MOVE_LEFT		7
#define MOVE_RIGHT		8
#define MOVE_BACK		9
#define MOVE_FORWARD	10
#define MANUAL_DRIVE	11
//Increment for distance
#define STEP 			20
#define MAX_DISTANCE	200
#define MIN_DISTANCE	10

//These variables are used in the ISR
volatile unsigned char pwmcount;
volatile unsigned char pwmL=0;
volatile unsigned int lDirection=0;
volatile unsigned char pwmR=0;
volatile unsigned int rDirection=0;
volatile unsigned char lWheel = 0;
volatile unsigned char rWheel = 0;
int distance = 0;
int command = 0;
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

int getAmplitude(char channel){
	
	return channel;
}

int getCommand(void){
	return NONE;
}

void doPark(void){
	
}

void doRot180(void){

}

void moveCrane(char direction){
	
}
void doManualDrive(){
	int rAmp = 0;
	int lAmp = 0;
	int command = NONE;
	while(1){
		rAmp = getAmplitude(SENSE_RIGHT);
		lAmp = getAmplitude(SENSE_LEFT);	
		if(rAmp == 0 && lAmp ==0){
			command = getCommand();
			
			switch(command){
				case PARK:
					doPark();
					break;
				case ROT180:
					doRot180();
					break;
				case CRANE_UP:
					moveCrane(CRANE_UP);
					break;
				case CRANE_DOWN:
					moveCrane(CRANE_DOWN);
					break;
				case MANUAL_DRIVE:
					return;
				case MOVE_LEFT:
					break;
				case MOVE_RIGHT:
					break;
				case MOVE_BACK:
					break;
				case MOVE_FORWARD:
					break;
			}
			command = NONE;
		}
	}
}
void main(void){
	
	int rAmp = 0;
	int lAmp = 0;
	int command = NONE;
	while(1){
		rAmp = getAmplitude(SENSE_RIGHT);
		lAmp = getAmplitude(SENSE_LEFT);	
		if(rAmp == 0 && lAmp ==0){
			command = getCommand();
			
			switch(command){
				case PARK:
					doPark();
					break;
				case ROT180:
					doRot180();
					break;
				case CLOSER:
					distance -= STEP;
					if(distance < 0) distance = 0;
					break;
				case AWAY:
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
}