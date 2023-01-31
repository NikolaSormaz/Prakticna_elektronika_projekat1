#include<p30fxxxx.h>
#include <stdlib.h>
#include <stdio.h>
#include "driverGLCD.h"
#include "adc.h"
#include "Tajmeri.h"

_FOSC(CSW_FSCM_OFF & HS3_PLL4);
_FWDT(WDT_OFF);
_FGS(CODE_PROT_OFF);

//const unsigned int ADC_THRESHOLD = 900; 
const unsigned int AD_Xmin = 384;
const unsigned int AD_Xmax = 3590;
const unsigned int AD_Ymin = 493;
const unsigned int AD_Ymax = 3213;
//const unsigned int AD_Tmin = 828;
//const unsigned int AD_Tmax = 3914;

#define AD_Tmin  828
#define AD_Tmax 3914

int ops;


#define DRIVE_A PORTCbits.RC13
#define DRIVE_B PORTCbits.RC14



void ConfigureTSPins(void)
{
	//ADPCFGbits.PCFG10=1;
	//ADPCFGbits.PCFG7=digital;

	//TRISBbits.TRISB10=0;
	TRISCbits.TRISC13=0;
    TRISCbits.TRISC14=0;
	
	//LATCbits.LATC14=0;
	//LATCbits.LATC13=0;
}

void initUART1(void)
{
    U1BRG=0x0015;//ovim odredjujemo baudrate - na terminalu 9600

    U1MODEbits.ALTIO=0;//biramo koje pinove koristimo za komunikaciju osnovne ili alternativne

    IEC0bits.U1RXIE=1;//omogucavamo rx1 interupt

    U1STA&=0xfffc;

    U1MODEbits.UARTEN=1;//ukljucujemo ovaj modul

    U1STAbits.UTXEN=1;//ukljucujemo predaju
}

unsigned char tempRX;
int provera=0;
unsigned int buf1 [7];
unsigned int buf2 [4];
int n=0;

void __attribute__((__interrupt__)) _U1RXInterrupt(void) 
{
    IFS0bits.U1RXIF = 0;
    tempRX=U1RXREG;
    
    if(tempRX != '/0' ){
        buf1[n]=tempRX;
        buf2[n]=tempRX;
        n++;       
        provera = 1;
    }
    
} 

void WriteUART1(unsigned int data)
{
	  while(!U1STAbits.TRMT);

    if(U1MODEbits.PDSEL == 3)
        U1TXREG = data;
    else
        U1TXREG = data & 0xFF;
}

void RS232_putst(register const char *str)
{

    while((*str)!=0){
    
        WriteUART1(*str);
        str++;
    } 
}


void WriteUART1dec2string(unsigned int data)
{
	unsigned char temp;

	temp=data/1000;
	WriteUART1(temp+'0');
	data=data-temp*1000;
	temp=data/100;
	WriteUART1(temp+'0');
	data=data-temp*100;
	temp=data/10;
	WriteUART1(temp+'0');
	data=data-temp*10;
	WriteUART1(data+'0');
}


unsigned int sirovi0,sirovi1,sirovi2,sirovi3,sirovi4;
unsigned int temp0,temp1,temp2; 
void __attribute__((__interrupt__)) _ADCInterrupt(void) 
{
							
	
    sirovi0=ADCBUF0;//0
	sirovi1=ADCBUF1;//1
    sirovi2=ADCBUF2; //lm35
	sirovi3=ADCBUF3; //pir
    sirovi4=ADCBUF4;//fotootpornik
   
	temp0=sirovi0;
	temp1=sirovi1;
    temp2=sirovi2;

    IFS0bits.ADIF = 0;
} 

void Delay(unsigned int N)
{
	unsigned int i;
	for(i=0;i<N;i++);
}

unsigned int X, Y,x_vrednost, y_vrednost;
void Touch_Panel (void)
{
// vode horizontalni tranzistori
	DRIVE_A = 1;  
	DRIVE_B = 0;
    
     LATCbits.LATC13=1;
     LATCbits.LATC14=0;

	Delay(500); //cekamo jedno vreme da se odradi AD konverzija
				
	// ocitavamo x	
	x_vrednost = temp0;//temp0 je vrednost koji nam daje AD konvertor na BOTTOM pinu		

	// vode vertikalni tranzistori
     LATCbits.LATC13=0;
     LATCbits.LATC14=1;
	DRIVE_A = 0;  
	DRIVE_B = 1;

	Delay(500); //cekamo jedno vreme da se odradi AD konverzija
	
	// ocitavamo y	
	y_vrednost = temp1;// temp1 je vrednost koji nam daje AD konvertor na LEFT pinu	
    
    X = ((float)(x_vrednost-AD_Xmin)/(AD_Xmax-AD_Xmin))*128;	
//vrednosti AD_Xmin i AD_Xmax su minimalne i maksimalne vrednosti koje daje AD konvertor za touch panel.


	Y = abs(64-((float)(y_vrednost-AD_Ymin)/(AD_Ymax-AD_Ymin))*64);
    
}


unsigned int stoperica1 = 50001;
unsigned int stoperica2, stoperica3;
void __attribute__((__interrupt__)) _T1Interrupt(void)
{

   	TMR1 =0;
    stoperica1++;
    IFS0bits.T1IF = 0;
}


void __attribute__((__interrupt__)) _T2Interrupt(void)
{

   	TMR2 =0;
    stoperica2++;
    IFS0bits.T2IF = 0;
}

void __attribute__((__interrupt__)) _T3Interrupt(void)
{

   	TMR3 =0;
    stoperica3++;
    IFS0bits.T3IF = 0;
}


void Delay1_ms (int vreme)//funkcija za kasnjenje u 0.1 ms
	{
		stoperica1 = 0;
		while(stoperica1 < vreme);
	}


void Delay_ms (int vreme)//funkcija za kasnjenje u milisekundama
	{
		stoperica2 = 0;
		while(stoperica2 < vreme);
	}

void Delay_10us (int vreme)//funkcija za kasnjenje u 10us
	{
		stoperica3 = 0;
		while(stoperica3 < vreme);
	}




//fotootpornik
int noc = 0;
void fotootpornik()
{
    
    if(sirovi4 >= 0 && sirovi4 < 2000)
    {
        if(noc == 1){
            RS232_putst("Pala je noc, svetlo u stakleniku se upalilo");
            LATBbits.LATB7=1;
            WriteUART1(13);
            noc = 0;
         
        }
    }  

    else if(sirovi4 > 2000)
    {
        if(noc == 0){
            RS232_putst("Svanuo je dan, svetlo u stakleniku se ugasilo");
            LATBbits.LATB7=0;
            WriteUART1(13);
            noc = 1;
         
        }
        
    } 
}

//lm35
int temper=0;
void lm35()
{  
   
    if(sirovi2 >= 0 && sirovi2 < 3000)
    {
        temper=1;
    }  

    else if(sirovi2 >= 3000)
    {
        if(temper == 1){
            RS232_putst("! ! ! Temperatura u stakleniku je veca od kriticne ! ! !");
            WriteUART1(13);
            temper=0;
        }
    } 
}



int k;
void zujanje(){

     for(k=0; k < 3000; k++){
        LATAbits.LATA11 = 1;
        Delay_10us(10);
        LATAbits.LATA11 = 0;
        Delay_10us(23);
        LATAbits.LATA11 = 1;
    }

}

int stanje=0;
void pir()
{
   
    if(sirovi3 > 500 && stanje == 0){
        
        RS232_putst("! ! ! Detektovano je nesto u stakleniku ! ! !");
        WriteUART1(13);
        T3CONbits.TON = 1;
        zujanje();
        T3CONbits.TON = 0;
    }        
   
}



int k, pwm;
void PWM(unsigned int pwm){
    
    for(k=0; k < 40; k++){
        
        LATBbits.LATB6 = 1;
        Delay_ms(pwm);
        LATBbits.LATB6 = 0;
        Delay_ms(20 - pwm);
        LATBbits.LATB6 = 1;
    }
}



void servo1(){

     pwm = 2;
     PWM(pwm);

}

void servo2(){

     pwm = 1;
     PWM(pwm);

}

int j;
void reset(){

    for (j= 0; j < 6; j++) buf1[j]='0';
    for (j= 0; j < 4; j++) buf2[j]='0';
    n=0;
    provera = 0;

}

unsigned int broj1,broj2; 
void servo(){

    if(buf1[0]=='O' && buf1[1]=='T' && buf1[2]=='V' && buf1[3]=='O' && buf1[4]=='R' && buf1[5]=='I' && stanje == 0)

            {
                RS232_putst("Unesite sifru");
                WriteUART1(13);
                reset();
                stanje = 1;
                provera=0;
 
            }
 
    
     
    else if(buf2[0]=='4' && buf2[1]=='3' && buf2[2]=='2' && buf2[3]=='1' && stanje == 1)

            {
                RS232_putst("Otvoreno");
                WriteUART1(13);
                T2CONbits.TON = 1;
                servo1();
                T2CONbits.TON = 0;
                reset();
                stanje = 2;
                provera = 0;
                
            }
 
    else if(buf1[0]=='Z' && buf1[1]=='A' && buf1[2]=='T' && buf1[3]=='V' && buf1[4]=='O' && buf1[5]=='R' && buf1[6]=='I' && stanje == 2)

            {
                RS232_putst("Zatvoreno");
                WriteUART1(13);
                T2CONbits.TON = 1;
                servo2();
                T2CONbits.TON = 0;
                reset();
                stanje = 0;
                provera = 0;
                
            }
    else if(provera == 1) {
 
        RS232_putst("Pogresna komanda");
        WriteUART1(13);
        reset();
        provera = 0;
 
    }
 
 
  for(broj1=0;broj1<100;broj1++)
            for(broj2=0;broj2<100;broj2++);

}



void Write_GLCD(unsigned int data)
{
unsigned char temp;

temp=data/1000;
Glcd_PutChar(temp+'0');
data=data-temp*1000;
temp=data/100;
Glcd_PutChar(temp+'0');
data=data-temp*100;
temp=data/10;
Glcd_PutChar(temp+'0');
data=data-temp*10;
Glcd_PutChar(data+'0');
}

int c;
int temperature[13] = {AD_Tmin,AD_Tmin,AD_Tmin,AD_Tmin,AD_Tmin,AD_Tmin,AD_Tmin,AD_Tmin,AD_Tmin,AD_Tmin,AD_Tmin,AD_Tmin,AD_Tmin};
float p = (float)((AD_Tmax - AD_Tmin) / 20);
    

int max_niza(){
    
    int max = AD_Tmin;
    int res;
    
    for (c = 0; c < 13; c++)
        if (max < temperature[c]) 
            max = temperature[c];
    
   
      res=(int)(((float)((max-AD_Tmin)/p)) + 15);
    
    return res;
  }


int min_niza(){
    
    int min = AD_Tmax;
    int res;
    
    for (c = 0; c < 13; c++)
        if (min > temperature[c]) 
            min = temperature[c];
    
    res=(int)(((float)((min-AD_Tmin)/p)) + 15);
    return res; 
  }

int avr_niza(){
    
    int suma = 0;
    int r,res;
    
    for (c = 0; c < 13; c++)
        suma += temperature[c];
    
    r= suma/13;
    res=(int)(((float)((float)(r-AD_Tmin)/p)) + 15);
    
    return res;
  }


int a,b;
int mod = 2;

void merenje_temp(){
            
        for(a=0 ;a < 12; a++){
            temperature[a] = temperature[a+1];
        }
        temperature[12] = temp2;

}


int pom = 1;
int ipsilon;
float del;
void grafik(){
    
    Touch_Panel();
    
    if( X > 8 && Y > 52 && X < 70 && Y < 61){	
       mod = 1;
       pom = 1;
       
    }
   
    if(stoperica1 > 50000){
    
        GLCD_ClrScr();
    
        GoToXY(5,0);		
        GLCD_Printf ("T");
    
        GoToXY(120,6);		
        GLCD_Printf ("t");
        
         GoToXY(1,1);
         GLCD_Printf ("35");
          
         GoToXY(1,3);
         GLCD_Printf ("25");
            
         GoToXY(1,5);
         GLCD_Printf ("15");
        
        GoToXY(12,7);
        GLCD_Printf ("PODACI ->");
        
         merenje_temp();
         
        
        for(b=0 ; b < 13 ; b++){

            del = (float) (ops / 32);
            ipsilon=(int)(43-((float)((temperature[b]-AD_Tmin)/del)));
            GLCD_Rectangle(8*(b+2)+2,(int)ipsilon, 8*(b+3)+1, 44);
        }
         
         stoperica1 = 0;
       
    }
    
}

void ekran(){
    
    if(pom == 1){
        
    
        GLCD_ClrScr();
        GLCD_Rectangle(8,4,60,20);
        GLCD_Rectangle(8,28,60,44);
        GLCD_Rectangle(68,4,120,20);
        GLCD_Rectangle(68,28,120,44);
        
        GoToXY(11,1);
        GLCD_Printf ("MAKSIMUM");
        
        GoToXY(13,4);
        GLCD_Printf ("MINIMUM");
        
        GoToXY(75,1);
        GLCD_Printf ("PROSEK");
        
        GoToXY(72,4);
        GLCD_Printf ("NAZAD ->");
        
        pom = 0;
        
    }
        Touch_Panel();
        GoToXY(4,7);
    
    if( X > 8 && Y > 4 && X < 56 && Y < 20){
        GoToXY(4,7);
        GLCD_Printf ("Maksimalna T:");
        GoToXY(100,7);
        Write_GLCD(max_niza());
        GoToXY(100,7);
        GLCD_Printf("  ");
    }
        
    if( X > 8 && Y > 28 && X < 56 && Y < 44){
        GoToXY(4,7);		
        GLCD_Printf ("Minimalna T: ");
        GoToXY(100,7);
        Write_GLCD(min_niza());
        GoToXY(100,7);
        GLCD_Printf("  ");
    }
    if( X > 72 && Y > 4 && X < 120 && Y < 20){	
        GoToXY(4,7);
        GLCD_Printf ("Prosecna T:  ");
        GoToXY(100,7);
        Write_GLCD(avr_niza());
        GoToXY(100,7);
        GLCD_Printf("  ");
    }
    
    if( X > 72 && Y > 28 && X < 120 && Y < 44){	
        mod = 2;
       GoToXY(72,4);
       GLCD_Printf ("->GRAFIK");
    }
       
    }
    

void kalibracija(){

    //WriteUART1('x');
    //WriteUART1dec2string(x_vrednost);
    //WriteUART1('y');
    //WriteUART1dec2string(y_vrednost);
    WriteUART1('t');
    WriteUART1dec2string(temperature[12]);
    
    for(a=0; a < 13 ; a++){
         WriteUART1dec2string(temperature[a]);
          RS232_putst("  ");
    }
    
    WriteUART1(13);
    
    Delay1_ms(10000);

}



void main(void)
{

    ops = AD_Tmax - AD_Tmin;
    
    int uu;
    int d = 1;
    //inicijalizacija
    ADCinit();
    ADCON1bits.ADON=1;
    initUART1();
    Init_T1();
    Init_T2();
    Init_T3();
    
    
	ConfigureLCDPins();
	ConfigureTSPins();

	GLCD_LcdInit();

	GLCD_ClrScr();
	
	ConfigureADCPins();
    
    //fotootpornik
    TRISBbits.TRISB12=1;//B12 je ulazni
    ADPCFGbits.PCFG12=0;//B12 je analogni za foto(dovodi se signal s naponskog razdelnika)
    TRISBbits.TRISB7=0;//B7 je izlazni
    ADPCFGbits.PCFG7=1;//digitani
    
    //lm35
    TRISBbits.TRISB10=1;//konfigurisemo kao ulaz za lm35
    ADPCFGbits.PCFG10=0;//analoognmi
    
    //pir  i buzzer
    TRISBbits.TRISB11=1;//B11 je ulazni od pira
    ADPCFGbits.PCFG11=0;//B11 je analogni
    TRISAbits.TRISA11=0; 
  
    
    //servo
    TRISBbits.TRISB6=0;
    ADPCFGbits.PCFG6=1;
    
    //ISPIS TEMPERATURE
    TRISFbits.TRISF6=1;//ulazni
    
	while(1)
	{
        if(mod == 1)
            ekran();
        else if(mod == 2)
            grafik();
        
        fotootpornik();
        lm35();
        servo();
        pir();
        
        if(PORTFbits.RF6 && d){
            
            uu = (int)(((float)((temperature[12]-AD_Tmin)/p)) + 15);
            RS232_putst("Trenutna temperatura je: ");
            WriteUART1dec2string(uu);
            WriteUART1(13);
            d = 0;
            stoperica3 = 0;
            T3CONbits.TON = 1;
        
        }
    
        if(d == 0 && stoperica3 > 30000){
            d = 1;
            stoperica3 = 0;
            T3CONbits.TON = 0;
            
        }
            
      //  Touch_Panel();
      // kalibracija();
      // merenje_temp();
        
     
	}//while

}//main

			
		
												
