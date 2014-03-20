#include <stdio.h>
#include <at89lp51rd2.h>

#define CLK 22118400L
#define BAUD 115200L
#define TIMER_2_RELOAD (0x10000L-(CLK/(32L*BAUD)))


unsigned char _c51_external_startup(void)
{
	// Configure ports as a bidirectional with internal pull-ups.
	P0M0=0;	P0M1=0;
	P1M0=0;	P1M1=0;
	P2M0=0;	P2M1=0;
	P3M0=0;	P3M1=0;
	AUXR=0B_0001_0001; // 1152 bytes of internal XDATA, P4.4 is a general purpose I/O
	P4M0=0;	P4M1=0;
    setbaud_timer2(TIMER_2_RELOAD); // Initialize serial port using timer 2
    
    return 0;
}

//Utilities file for functions like wait
void wait_bit_time (void)
{
	_asm
	;For a 22.1184MHz crystal one machine cycle 
	;takes 12/22.1184MHz=0.5425347us
	N2:
	 mov R1, #250
	N1:
	 mov R0, #184
	 
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
	M3:
	 mov R2, #3
	M2:
	 mov R1, #125
	M1:
	 mov R0, #184
	 
	djnz R0, M1 ; 2 machine cycles-> 2*0.5425347us*184=200us
	djnz R1, M2 ; 200us*125=0.025s
	DJNZ R2, M3 ; 3*0.025s= 0.075s

	ret
	_endasm;
}

//Write
/*

void tx_byte ( unsigned char val )
{
	unsigned char j;
	//Send the start bit
	txon=0;
	wait_bit_time();
	for (j=0; j<8; j++)
	{
		txon=val&(0x01<<j)?1:0;
		wait_bit_time();
	}
	txon=1;
	//Send the stop bits
	wait_bit_time();
	wait_bit_time();
}

*/



//Read
unsigned char rx_byte () 
{ 
	unsigned char j, val; 

	//Skip the start bit 
	val=0; 
	wait_one_and_half_bit_time(); 
	
	for(j=0; j<8; j++) 
	{
		if(P1_7==1) {
		val&(0x01<<j);
		 }
		else if (P1_7==0) {
		val&(0x00<<j);
		}
		
	wait_bit_time(); 
	
	}
	
	wait_one_and_half_bit_time(); 
	return val;
}




void main(void)
{	const unsigned char  UP	= 1;  
	const unsigned char  DOWN =2; 
	const unsigned char  LEFT =3; 
	const unsigned char  RIGHT =4; 
	const unsigned char  UPRIGHT =5; 
	const unsigned char  UPLEFT =6; 
	const unsigned char  DOWNRIGHT =7; 
	const unsigned char  DOWNLEFT  =8; 
	const unsigned char  R1	=9; 
	const unsigned char  R2	=10; 
	const unsigned char  X	=11; 
	const unsigned char  TRIANGLE =12; 
	const unsigned char  SQUARE =13; 
	const unsigned char  CIRCLE =14; 
	const unsigned char  START =15; 
	const unsigned char  SELECT =16;
	unsigned char command=0;
	printf("hello");
	
	while(1) 
	{
			printf("%c", command);
			command = rx_byte(); //find minimum noise value with oscilloscope	
		
			
		if(command == UP)
		{
			printf("UP");
		}
		else if(command == DOWN)
		{
			printf("DOWN");
		}
		else if(command == LEFT)
		{
			printf("LEFT");
		}
		else if(command == RIGHT)
		{
			printf("RIGHT");
		}
		else if(command == UPLEFT)
		{
			printf("UP LEFT");
		}
		else if(command == UPRIGHT)
		{
			printf("UP RIGHT");
		}
		else if(command == DOWNLEFT)
		{
			printf("DOWN LEFT");
		}
		else if(command == SELECT)
		{
			printf("Select Pressed - Change Modes");
		}
		else if(command == START)
		{
			printf("START");
		}
		else if(command == X)
		{
			printf("X");
		}
		else if(command == TRIANGLE)
		{
			printf("TRIANGLE");
		}
		else if(command == SQUARE)
		{
			printf("SQUARE");
		}
		else if(command == CIRCLE)
		{
			printf("CIRCLE");
		}
		else if(command == R1)
		{
			printf("R1");
		}
		else if(command == R2)
		{
			printf("R2");
		}
		else 
			printf("none");
			
	}
}
