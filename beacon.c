 //The code for the beacon goes here.
#include <stdio.h>
#include <at89lp51rd2.h>

// ~C51~ 
 
#define CLK 22118400L
#define BAUD 115200L
#define BRG_VAL (0x100-(CLK/(32L*BAUD)))

//We want timer 0 to interrupt every 100 microseconds ((1/10000Hz)=100 us)
#define FREQ 10000L
#define TIMER0_RELOAD_VALUE (65536L-(CLK/(12L*FREQ)))

//PSX Controller Buttons
#define UP		00000001B
#define DOWN	00000010B
#define LEFT	00000011B
#define RIGHT	00000100B
#define UPRIGHT	00000101B
#define UPLEFT	00000110B
#define DOWNRIGHT 00000111B
#define DOWNLEFT  00001000B
#define R1	00001001B
#define R2	00001010B
#define X	00001011B
#define TRIANGLE 00001100B
#define SQUARE	 00001101B
#define CIRCLE	 00001110B
#define START	 00001111B
#define SELECT	 00010000B



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
	
    return 0;
}

//Utilities file for functions like wait
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

//Write
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

//Read
unsigned char rx_byte ( int min ) 
{ 
	unsigned char j, val; 
	int v;
	//Skip the start bit 
	val=0; 
	wait_one_and_half_bit_time(); 
	for(j=0; j<8; j++) 
	{
		v=GetADC(P1_7); 
		val|=(v>min)?(0x01<<j):0x00; 
		wait_bit_time(); 
	} //Wait for stop bits 
	wait_one_and_half_bit_time(); 
	return val;
}


void wait5ms (void)
{
	_asm	
		;For a 22.1184MHz crystal one machine cycle 
		;takes 12/22.1184MHz=0.5425347us
		mov R1, #25
	L2:	mov R0, #184
	L1:	djnz R0, L1 ; 2 machine cycles-> 2*0.5425347us*184=200us
	    djnz R1, L2 ; 200us*25=5ms

	    ret
    _endasm;
}

void SPIWrite( unsigned char value)
{
	SPSTA&=(~SPIF); // Clear the SPIF flag in SPSTA
	SPDAT=value;
	while((SPSTA & SPIF)!=SPIF); //Wait for transmission to end
}	

// Read 10 bits from the MCP3004 ADC converter
unsigned int GetADC(unsigned char channel)
{
	unsigned int adc;
	// initialize the SPI port to read the MCP3004 ADC attached to it.
	SPCON&=(~SPEN); // Disable SPI
	SPCON=MSTR|CPOL|CPHA|SPR1|SPR0|SSDIS;
	SPCON|=SPEN; // Enable SPI

	P1_4=0; // Activate the MCP3004 ADC.
	SPIWrite(channel|0x18); // Send start bit, single/diff* bit, D2, D1, and D0 bits.
	for(adc=0; adc<10; adc++); // Wait for S/H to setup
	SPIWrite(0x55); // Read bits 9 down to 4
	adc=((SPDAT&0x3f)*0x100);
	SPIWrite(0x55); // Read bits 3 down to 0
	P1_4=1; // Deactivate the MCP3004 ADC.
	adc+=(SPDAT&0xf0); // SPDR contains the low part of the result.
	adc>>=4;
	
	return adc;
}

void main(void)
{	
	unsigned char command;
	
	while(1) 
	{
		if(P1_5=1) 
		{
			command = rx_byte(2.5)); //find minimum noise value with oscilloscope		
		}
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
	}
}
