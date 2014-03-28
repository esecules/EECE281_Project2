//The main code for the car goes here
#include <stdio.h> 
#include <at89lp51rd2.h>
#include "libTXRX.c"

// ~C51~
#define REF 1
#define TEST 1
#define CLK 22118400L 
#define BAUD 115200L 
#define BRG_VAL (0x100-(CLK/(32L*BAUD)))

//We want timer 0 to interrupt every 100 microseconds ((1/10000Hz)=100 us)
#define FREQ 10000L
#define TIMER0_RELOAD_VALUE (65536L-(CLK/(12L*FREQ)))

#define TEST 1
//Motor Pins
#define RWHEEL_B	P3_6
#define RWHEEL_R	P3_7
#define LWHEEL_B	P3_4
#define LWHEEL_R	P3_5
#define CRANE_R		P3_3
#define	CRANE_B		P3_2
#define UNDER_PIN	P0_3
#define SENSE_LEFT 	1
#define SENSE_RIGHT 0
#define FORWARD 	1
#define BACK 		0
#define CLOCK		1
#define C_CLOCK		0
#define UP			1
#define	DOWN		0


//Commands
#define NONE			0	
#define PARK			12
#define ROT180			14
#define CRANE_UP		9
#define CRANE_DOWN		10
#define MOVE_LEFT		3
#define MOVE_RIGHT		4
#define MOVE_BACK		2
#define MOVE_FORWARD	1
#define MOVE_FR			5
#define MOVE_FL			6
#define MOVE_BR			7
#define MOVE_BL			8
#define MANUAL_DRIVE	15
#define RETRACE			11
#define UNDER_GLOW		13
//Increment for amplitude80
#define STEP 			30
#define MAX_AMPLITUDE	500
#define MIN_AMPLITUDE	25
//Car Dimensions (in centimeters)
#define WHEEL_CIRCUMFERENCE	21.0
#define SEC_ROT			1.00
//These variables are used in the ISR
volatile unsigned char pwmcount;
volatile unsigned char pwmL=0;
volatile unsigned int lDirection=0;
volatile unsigned char pwmR=0;
volatile unsigned int rDirection=0;
volatile unsigned char backmode = 0;
volatile unsigned char lWheel = 0;
volatile unsigned char rWheel = 0;
volatile unsigned char pwmC = 0;
volatile unsigned char cDirection = 0;
volatile unsigned char crane = 0;
int idealAmp= 70;
int command = 0;
int sensativity = 0;
int park_status = 0;
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
	
	pwmcount=0;
    TXRXinit();
    EA=1;  // Enable global interrupts
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
		if(lDirection^backmode==FORWARD){
			LWHEEL_R=(pwmL>pwmcount)?0:1;
			LWHEEL_B=1;
		}
	
		else if(lDirection^backmode==BACK){
			LWHEEL_B=(pwmL>pwmcount)?0:1;
			LWHEEL_R=1;
		}
	}else{
		LWHEEL_B=1;
		LWHEEL_R=1;
	}
	
	if(rWheel){
		if(rDirection^backmode==FORWARD){
			RWHEEL_R=(pwmR>pwmcount)?0:1;
			RWHEEL_B=1;
		}
	
		else if(rDirection^backmode==BACK){
			RWHEEL_B=(pwmR>pwmcount)?0:1;
			RWHEEL_R=1;
		}
	}else{
		RWHEEL_B=1;
		RWHEEL_R=1;
	}
	
	if(crane){
		if(cDirection==UP){
			CRANE_R=(pwmC>pwmcount)?0:1;
			CRANE_B=1;
		}
	
		else if(cDirection==DOWN){
			CRANE_B=(pwmC>pwmcount)?0:1;
			CRANE_R=1;
		}
	}else{
		CRANE_B=1;
		CRANE_R=1;
	}
}


void moveCrane(char direction){
	cDirection = direction;
	pwmC = 50;
	crane = 1;
	timer = 0;
	while(timer < 5){} // NEEDS CALIBRATION!!
	crane = 0;
	
}

void rotate(char direction, int angle){
	timer = 0;
	switch (direction^backmode){
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

//idealAmpmust be in centimeters
void moveDistance(double amplitude, char direction) {
	rDirection = direction;
	lDirection = direction;
	pwmL = 50;
	pwmR = 50;
	timer = 0;
	rWheel = 1;
	lWheel = 1;
	while(timer < (4.0/3.0*21.0*amplitude/WHEEL_CIRCUMFERENCE)/SEC_ROT){};
	rWheel = 0;
	lWheel = 0;
}


void doPark(void){
	if ( park_status == 0 )
	{
		rotate(C_CLOCK,45);
		moveDistance(23.0,BACK);
		rotate(CLOCK,40);
		park_status = 1;
	}
	else
	{
		rotate(CLOCK,40);
		moveDistance(23.0,FORWARD);
		rotate(C_CLOCK,45);
		park_status = 0;
	}
	
}

void test(void){
	int counter = 0;
	while( 1 ){
	moveDistance(5.0, BACK);
	moveDistance(5.0, FORWARD);
	}
}
void doManualDrive(){
	int rAmp = 0;
	int lAmp = 0;
	int command = NONE;
	UNDER_PIN = 0;
	printf("---Entering Manual Drive---");
	while(1){
		//rAmp = GetADC(SENSE_RIGHT);
		//lAmp = GetADC(SENSE_LEFT);	
		if(rAmp == 0 && lAmp ==0){
			if(TEST) scanf("%d",&command);
			else{
				ET0 = 0;
				P3 = 0xFF;
				command = rData();
				ET0 = 1;
			}
			printf("manual cmd %d\n",command);
			switch(command){
				case PARK:
					doPark();
					break;
				case ROT180:
					printf("rot 180");
					rotate(CLOCK,45);
					backmode^=1;
					break;
				case CRANE_UP:
					moveCrane(CRANE_UP);
					break;
				case CRANE_DOWN:
					moveCrane(CRANE_DOWN);
					break;
				case MANUAL_DRIVE:
					printf("---Exiting Manual Drive---");
					return;
				case MOVE_RIGHT:
					printf("move right");
					if(backmode) goto move_left;
				move_right:
					rDirection = BACK;
					lDirection = FORWARD;
					pwmL = 50;
					pwmR = 50;
					rWheel = 1;
					lWheel = 1;
					break;
				case MOVE_LEFT:
					printf("move left");
					if(backmode) goto move_right;
				move_left:
					rDirection = FORWARD;
					lDirection = BACK;
					pwmL = 50;
					pwmR = 50;
					rWheel = 1;
					lWheel = 1;
					break;
				case MOVE_BACK:
					printf("move back");
					rDirection = BACK;
					lDirection = BACK;
					pwmL = 50;
					pwmR = 50;
					rWheel = 1;
					lWheel = 1;
					break;
				case MOVE_FORWARD:
					printf("move forward");
					rDirection = FORWARD;
					lDirection = FORWARD;
					pwmL = 50;
					pwmR = 50;
					rWheel = 1;
					lWheel = 1;
					break;
				case MOVE_FR:
					if(backmode) goto move_fl;
				move_fr:
					rDirection = FORWARD;
					lDirection = FORWARD;
					pwmL = 75;
					pwmR = 50;
					rWheel = 1;
					lWheel = 1;
					break;
				case MOVE_FL:
					if(backmode) goto move_fr;
				move_fl:
					rDirection = FORWARD;
					lDirection = FORWARD;
					pwmL = 50;
					pwmR = 75;
					rWheel = 1;
					lWheel = 1;
					break;
				case MOVE_BR:
					if(backmode) goto move_bl;
				move_br:
					rDirection = BACK;
					lDirection = BACK;
					pwmL = 75;
					pwmR = 50;
					rWheel = 1;
					lWheel = 1;
					break;
				case MOVE_BL:
					if(backmode) goto move_br;
				move_bl:
					rDirection = BACK;
					lDirection = BACK;
					pwmL = 50;
					pwmR = 75;
					rWheel = 1;
					lWheel = 1;
					break;
				case UNDER_GLOW:
					if (UNDER_PIN == 1)
						UNDER_PIN = 0;
					else
						UNDER_PIN = 1;
				default:
					printf("DEFAULT\n");
					rWheel = 0;
					lWheel = 0;
					crane =0;
					break;
			}
		}
		else if(0){
			command = NONE;
			rWheel = 0;
			lWheel = 0;
			crane =0;	
			P3 = 0xFF;
		}
	}
}
void main(void){
	
	int rAmp = 0;
	int lAmp = 0;
	int tempR, tempL;
	int command = NONE;
	//doPark();
	while(1){
		if(TEST) doManualDrive();
		rAmp = GetADC(SENSE_RIGHT);
		lAmp = GetADC(SENSE_LEFT);	
		//printf("idealAmp%d, sensitivity %d, ramp %d, lamp %d\n", amplitude, sensativity, rAmp, lAmp);
		if(rAmp == 0 && lAmp ==0){
			ET0 = 0;
			P3 = 0xFF;
			command = rData();
			printf("command %d\n",command);
			ET0 = 1;
			switch(command){
				case MOVE_FORWARD:
					idealAmp+= STEP;
					if(idealAmp> MAX_AMPLITUDE){
					 printf("too close\n");
					 idealAmp= MAX_AMPLITUDE;
					 }
					break;
				case MOVE_BACK:
					idealAmp-= STEP;
					if(idealAmp< MIN_AMPLITUDE){
					 printf("too far");
					 idealAmp= MIN_AMPLITUDE;
					 }
					break;
				case MANUAL_DRIVE:
					doManualDrive();
					break;
				case 255: //ERROR
					printf("error 255\n");
					rWheel = 0;
					lWheel = 0;
					crane = 0;
					break;
					//goto Exit;
						
			}
			command = NONE;
			
		}
		
		else if (lAmp > MIN_AMPLITUDE && rAmp > MIN_AMPLITUDE){
			//printf("R:%d L:%d\n", rAmp, lAmp);
			if(rAmp < idealAmp+ sensativity){
				rDirection = FORWARD;
				tempR = 1;	
			}
			else if(rAmp > idealAmp- sensativity){
				rDirection = BACK;
				tempR = 1;	
			}
			else tempR = 0;
			
			if(lAmp < idealAmp+ sensativity){
				lDirection = FORWARD;
				tempL = 1;	
			}
			else if(lAmp > idealAmp- sensativity){
				lDirection = BACK;
				tempL = 1;	
			}
			else tempL = 0;
			//tempPWM = abs(((lAmp+rAmp)/2)-idealAmp)*(100/idealAmp);
			//tempPWM = (tempPWM > 70 ? tempPWM : 70);
			pwmR = 50;
			pwmL = 50;
			rWheel = tempR;
			lWheel = tempL;
		}	
	}
	Exit:
	EA = 0;
}
