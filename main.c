/*
 * File:   main.c
 * Author: Felix
 *
 * Created on March 14, 2017, 3:23 PM
 * 
 * Program dims RGB Led strip with 16bit PWM resolution, currently without color
 * In the future RGB Leds will be calibrated to XY color space used to determine RGB values
 */

#pragma config FOSC = INTOSC    //  (INTOSC oscillator; I/O function on CLKIN pin)
#pragma config WDTE = OFF        // Watchdog Timer Enable (WDT enabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable (PWRT disabled)
#pragma config MCLRE = ON       // MCLR Pin Function Select (MCLR/VPP pin function is MCLR)
#pragma config CP = OFF         // Flash Program Memory Code Protection (Program memory code protection is disabled)
#pragma config BOREN = ON       // Brown-out Reset Enable (Brown-out Reset enabled)
#pragma config CLKOUTEN = OFF   // Clock Out Enable (CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin)

// CONFIG2
#pragma config WRT = OFF        // Flash Memory Self-Write Protection (Write protection off)
#pragma config PLLEN = ON       // PLL Enable (4x PLL enabled)
#pragma config STVREN = ON      // Stack Overflow/Underflow Reset Enable (Stack Overflow or Underflow will cause a Reset)
#pragma config BORV = LO        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (Vbor), low trip point selected.)
#pragma config LPBOREN = OFF    // Low Power Brown-out Reset enable bit (LPBOR is disabled)
#pragma config LVP = OFF         // Low-Voltage Programming Enable (Low-voltage programming enabled)

#include <xc.h>
                   
float Yr, Yg, Yb;
float dimmerVal;

unsigned int  adcResult, adcResultCurrent;

void main(void) {
    init();
    //Program in infinite loop
    while(1){
    }   
return;
}

static int init(){
    
    GIE    	= 1;             //Global Interrupt Enable
    PEIE   	= 1;             //Periphieal Interrupt Enable
    TMR0IE 	= 0;             //TMR0 Interrupt Enable 
    
    OSCCON 	= 0B11110010;    //32MHz
    LATA   	= 0x00;
    
    //ADC
    TRISA  	= 0b00000001;    //Out/In on Pin (1 = in)
    ANSELA 	= 0b00000001;    //1 = Analog In on pin
    ADCON0 	= 0b00000001;    //ADC on for RA0
    ADCON1 	= 0b11100000;    //Clock speed and Voltage ref select
    ADGO 	= 1;             //Start ADC conversion
    
   
    while(ADGO == 1){}      						//Wait for conversion to finish
    adcResult 			= (ADRESH << 8) | ADRESL;   //ADC result stored in two byte sized registers, shifted into 16bit int value
    
    ADIE   				= 1;             //ADC Interrupt Enable
    ADGO 				= 1;
    
    //Timer 1 - PWM
    APFCON 				= 0b00000011;   //PWM1 on RA5 - Blue, PWM2 on RA4 - Red, PWM3 on RA2 - Green (by default)
    
    //PWM1 - Blue
    PWM1PH          	= 0;            // clear phase registers.
    PWM1DC          	= 0;            // clear duty cycle registers
    PWM1PR         		= 65535;        // set period register
     
    PWM1CLKCON      	= 0b00010000;   //PWM CLK Prescaler
    PWM1CON         	= 0b01000000;   //PMW Control
    PWM1LDCONbits.LDT 	= 0;          	//PWM Register Buffer Loading
    PWM1LDCONbits.LDA 	= 1; 
    
    //PWM2 - Red
    PWM2PH          	= 0;            // clear phase registers.
    PWM2DC         		= 0;            // clear duty cycle registers
    PWM2PR         		= 65535;        // set period register
    
    PWM2CLKCON      	= 0b00010000;   //PWM CLK Prescaler
    PWM2CON         	= 0b01000000;   //PMW Control
    PWM2LDCONbits.LDT	= 0;          	//PWM Register Buffer Loading
    PWM2LDCONbits.LDA 	= 1;
    
    //PWM3 - Green
    PWM3PH          	= 0;            // clear phase registers.
    PWM3DC          	= 0;            // clear duty cycle registers
    PWM3PR          	= 65535;        // set period register
    
    PWM3CLKCON      	= 0b00010000;   //PWM CLK Prescaler
    PWM3CON         	= 0b01000000;   //PMW Control
    PWM3LDCONbits.LDT 	= 0;      	    //PWM Register Buffer Loading
    PWM3LDCONbits.LDA 	= 1;
    
    PWMEN          	 	= 0x07;         // Enable All PWM's at Once
}
 
//General Interrupt service routine function
void interrupt isr(void)
{
    //ADC complete and ADC enabled interrupt flags
    if(ADIE == 1 && ADIF == 1){
        ADIF = 0;       //Clear interrupt flag
        
        //Since ADC is running at set frequency only update the PWM if value changes
        adcResultCurrent = (ADRESH << 8) | ADRESL;
        if(adcResult != adcResultCurrent){
            adcResult = adcResultCurrent;
            dimmerVal = ((float)adcResult)/((float)0x03FF);
            
            PWM1DC          = 0xFFFF * dimmerVal; //Blue
            PWM2DC          = 0xFFFF * dimmerVal; //Red
            PWM3DC          = 0xFFFF * dimmerVal; //Green

            PWM1LDCONbits.LDA = 1;      //Update PWM on next period
            PWM2LDCONbits.LDA = 1;
            PWM3LDCONbits.LDA = 1;

            
        }
         ADGO = 1;
    }
}
