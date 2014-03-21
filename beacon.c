#include <stdio.h>
#include <at89lp51rd2.h>

#define CLK 22118400L
#define BAUD 115200L
#define TIMER_2_RELOAD (0x10000L-(CLK/(32L*BAUD)))

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
	unsigned char val, bit0, bit1, bit2, bit3;
	
	val = 0;
	
	bit0 = P1_0;
	bit1 = P1_1;
	bit2 = P1_2;
	bit3 = P1_3;
	
	if(bit0 == 1) 
	{
		val|=(0x01<<0);
	}
	else if (bit0 == 0) 
	{
		val|=0x00;
	}
	if(bit1 == 1) 
	{
		val|=(0x01<<1);
	}
	else if (bit1 == 0) 
	{
		val|=0x00;
	}
	if(bit2 == 1) 
	{
		val|=(0x01<<2);
	}
	else if (bit2 == 0) 
	{
		val|=0x00;
	}
	if(bit3 == 1) 
	{
		val|=(0x01<<3);
	}
	else if (bit3 == 0) 
	{
		val|=0x00;
	}
	return val;
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
