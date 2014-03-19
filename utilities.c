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
	 
	djnz R0, N1 ; 2 machine cycles-> 2*0.5425347us*184=200us
	djnz R1, N2 ; 200us*250=0.05s

	ret
	_endasm;
}
void wait_one_and_half_bit_time (void)
{
	_asm
	;For a 22.1184MHz crystal one machine cycle 
	;takes 12/22.1184MHz=0.5425347us
	N3:
	 mov R2, #3
	N2:
	 mov R1, #125
	N1:
	 mov R0, #184
	 
	djnz R0, N1 ; 2 machine cycles-> 2*0.5425347us*184=200us
	djnz R1, N2 ; 200us*125=0.025s
	DJNZ R2, N3 ; 3*0.025s= 0.075s

	ret
	_endasm;
}