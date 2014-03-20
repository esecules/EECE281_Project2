#define CLK 22118400L
#define BAUD 115200L
#define BRG_VAL (0x100-(CLK/(32L*BAUD)))

#define FREQ 30600L
#define TIMER0_RELOAD_VALUE (65536L-(CLK/(12L*FREQ)))

unsigned char _c51_external_startup(void);
void isr0 (void);