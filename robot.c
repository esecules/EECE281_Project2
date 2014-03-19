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
#define RETRACE			12
//Increment for distance
#define STEP 			20
#define MAX_DISTANCE	200
#define MIN_DISTANCE	10
//Car Dimensions (in centimeters)
#define WHEEL_CIRCUMFERENCE	21
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
void wait_bit_time (void)
{
	_asm
	;For a 22.1184MHz crystal one machine cycle 
	;takes 12/22.1184MHz=0.5425347us
	mov R2, #2
	N3: mov R1, #250
	N2: mov R0, #184
	N1: djnz R0, N1 ; 2 machine cycles-> 2*0.5425347us*184=200us
	djnz R1, N2 ; 200us*250=0.05s
	djnz R2, N3 ; 0.05s*20=1s
	ret
	_endasm;
}
void wait_one_and_half_bit_time (void)
{
	_asm
	;For a 22.1184MHz crystal one machine cycle 
	;takes 12/22.1184MHz=0.5425347us
	mov R2, #3
	M3: mov R1, #250
	M2: mov R0, #184
	M1: djnz R0, N1 ; 2 machine cycles-> 2*0.5425347us*184=200us
	djnz R1, N2 ; 200us*250=0.05s
	djnz R2, N3 ; 0.05s*20=1s
	ret
	_endasm;
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

void doRot180(void){

}

void moveCrane(char direction){
	
}

void doRot45CounterClockwise(void) {

}

void doRot45Clockwise(void){

}

//distance must be in centimeters
void moveDistance (double distance) {
}


void doPark(void){

	doRot45CounterClockwise();
	moveDistance(26.0);
	doRot45Clockwise();
}

unsigned char getCommand ( int min ){
	unsigned char j, val;
	int v;
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
	return val;
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
					rDirection = BACK;
					lDirection = FORWARD;
					rWheel = 1;
					lWheel = 1;
					break;
				case MOVE_RIGHT:
					rDirection = FORWARD;
					lDirection = BACK;
					rWheel = 1;
					lWheel = 1;
					break;
				case MOVE_BACK:
					rDirection = BACK;
					lDirection = BACK;
					rWheel = 1;
					lWheel = 1;
					break;
				case MOVE_FORWARD:
					rDirection = FORWARD;
					lDirection = FORWARD;
					rWheel = 1;
					lWheel = 1;
					break;
				default:
					rWheel = 0;
					lWheel = 0;
					break;
			}
			command = NONE;
		}
	}
}
void main(void){
	
	int rAmp = 0;
	int lAmp = 0;
	int tempR, tempL;
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
				case RETRACE:
					
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
			
			rWheel = tempR;
			lWheel = tempL;
		}	
	}
}