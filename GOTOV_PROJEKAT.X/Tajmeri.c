#include <p30fxxxx.h>
#include "Tajmeri.h"



#define TMR1_period  333  /*  Fosc = 3.33MHz,
					          1/Fosc = 0.3us !!!, 0.3us *333  = 0.1ms  */

#define TMR2_period 3333 /*  Fosc = 3.33MHz,
					          1/Fosc = 0.3us !!!, 0.3us *3333  = 1ms  */

#define TMR3_period 33 /*  Fosc = 3.33MHz,
					          1/Fosc = 0.3us !!!, 0.3us * 33 = 10us  */


void Init_T1(void)
{
	TMR1 = 0;
	PR1 = TMR1_period;
	
	T2CONbits.TCS = 0; // 0 = Internal clock (FOSC/4)
	//IPC1bits.T2IP = 3 // T2 interrupt pririty (0-7)
	//SRbits.IPL = 3; // CPU interrupt priority is 3(11)
	IFS0bits.T1IF = 0; // clear interrupt flag
	IEC0bits.T1IE = 1; // enable interrupt

	T1CONbits.TON = 1; // T1 on 
}


void Init_T2(void)
{
	TMR2 = 0;
	PR2 = TMR2_period;
	
	T2CONbits.TCS = 0; // 0 = Internal clock (FOSC/4)
	//IPC1bits.T2IP = 3 // T2 interrupt pririty (0-7)
	//SRbits.IPL = 3; // CPU interrupt priority is 3(11)
	IFS0bits.T2IF = 0; // clear interrupt flag
	IEC0bits.T2IE = 1; // enable interrupt

	//T2CONbits.TON = 1; // T2 on 
}

void Init_T3(void)
{
	TMR3 = 0;
	PR3 = TMR3_period;
	
	T3CONbits.TCS = 0; // 0 = Internal clock (FOSC/4)
	//IPC1bits.T2IP = 3 // T2 interrupt pririty (0-7)
	//SRbits.IPL = 3; // CPU interrupt priority is 3(11)
	IFS0bits.T3IF = 0; // clear interrupt flag
	IEC0bits.T3IE = 1; // enable interrupt

	//T3CONbits.TON = 1; // T2 on 
}

//---------------------------------------
