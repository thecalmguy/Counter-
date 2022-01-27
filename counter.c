/* 
 * File:   counter.c
 * Author: Manik Mittal
 *
 * Created on 8 December, 2021, 8:51 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <xc.h>

#pragma config FOSC = INTRCIO   // Oscillator Selection bits (INTOSC oscillator: I/O function on RA4/OSC2/CLKOUT pin, I/O function on RA5/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = ON       // Power-up Timer Enable bit (PWRT enabled)
#pragma config MCLRE = OFF       // RA3/MCLR pin function select (RA3/MCLR pin function is MCLR)
#pragma config BOREN = ON       // Brown-out Detect Enable bit (BOD enabled)
#pragma config CP = OFF         // Code Protection bit (Program Memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#define _XTAL_FREQ 8000000
#define CLOCK_PIN RA4
#define INPUT_PIN RA3
#define DATA_PIN RC2
#define DIGIT3 RC5
#define DIGIT2 RC4
#define DIGIT1 RC3
#define BUZZER RA1

int count=0;

unsigned int seg(unsigned int num) {
  switch (num) {
    case 0 : return 0xA0;
    case 1 : return 0xBE;
    case 2 : return 0x62;
    case 3 : return 0x2A;
    case 4 : return 0x3C;
    case 5 : return 0x29;
    case 6 : return 0x21;
    case 7 : return 0xBA;
    case 8 : return 0x20;
    case 9 : return 0x28;
    }
  return 0;
}


int displaySingleNum(int num, int place) {
    unsigned int code = seg(num);
    unsigned int j;
    for (j = 0x80; j > 0; j = j >> 1) {
        if (code & j)
            DATA_PIN = 1;
        else
            DATA_PIN = 0;
        CLOCK_PIN = 1;
        CLOCK_PIN = 0;
    }
    
    switch (place) {
    case 1 : 
        DIGIT1 =1;
        break;
    case 2 : 
        DIGIT2 =1;
        break;
    case 3 : 
        DIGIT3 =1;
        break;
    }
    
    __delay_ms(2);
    
    
    DIGIT1 =0;
    DIGIT2 =0;
    DIGIT3 =0;
    
    return 0;
}

void writeEEPROM(unsigned char address, unsigned char datas)
{
  unsigned char INTCON_SAVE;//To save INTCON register value
  EEADR = address; //Address to write
  EEDATA = datas; //Data to write
  WREN = 1; //Enable writing of EEPROM
  INTCON_SAVE=INTCON;//Backup INCON interupt register
  INTCON=0; //Diables the interrupt
  EECON2=0x55; //Required sequence for write to internal EEPROM
  EECON2=0xAA; //Required sequence for write to internal EEPROM
  WR = 1; //Initialise write cycle
  INTCON = INTCON_SAVE;//Enables Interrupt
  WREN = 0; //To disable write
  while(EEIF == 0)//Checking for complition of write operation
  {
    asm ("nop"); //do nothing
  }
  EEIF = 0; //Clearing EEIF bit
}

unsigned char readEEPROM(unsigned char address)
{
  EEADR = address; //Address to be read
  RD = 1; //Initialise read cycle
  return EEDATA; //Returning data
}


int displayAllNum(int num) {
    int dig100=0;
    int dig10=0;
    int dig1=0;
    
    dig100 = num/100;
    num -= (dig100*100);
    dig10  = num/10;
    num -= (dig10*10);
    dig1   = num;
    if(dig100){
        displaySingleNum(dig100,1);
        displaySingleNum(dig10,2);
        displaySingleNum(dig1,3);
    }else if(dig10){
        __delay_ms(2);
        displaySingleNum(dig10,2);
        displaySingleNum(dig1,3);
    }else {
        __delay_ms(4);
        displaySingleNum(dig1,3);
    }
    
    
    return 0;
}

int main(int argc, char** argv) {
    
  unsigned int j;
  extern int count;
  //ANSEL  = 0X00;
  TRISA  = 0X08;    
  TRISC=0X00;  
  INTEDG = 0;
  GIE    = 0;
  
  count = 0;
  
  TMR0   = 0;         // reset Timer0
  DIGIT1 =0;
  DIGIT2 =0;
  DIGIT3 =0;
  CLOCK_PIN=0;
  
  if (readEEPROM(0X11) == 0XFF) {
        count = 0;
    } else {
        count = readEEPROM(0X11);
    }
  
  while(1)
  {
    if (INPUT_PIN == 0) {
        count++;
        for(j = 0; j < 350; j++){
            BUZZER=1;
            //custom_delay(count);
            __delay_us(150);
            BUZZER=0;
            //custom_delay(count);
            __delay_us(150);
        }
        writeEEPROM(0X11,count);
        for (j = 50; j > 0; j--) {
            displayAllNum(count);
            }
        j=0;
        while(INPUT_PIN == 0 && j<150){
            displayAllNum(count);
            j++;
        }
        if(j>140){
            count=0;
            for (j = 0; j < 750; j++) {
                BUZZER = 1;
                //custom_delay(count);
                __delay_us(150);
                BUZZER = 0;
                //custom_delay(count);
                __delay_us(150);
            }
            for (j = 150; j > 0; j--) {
                displayAllNum(count);
            }
        }
    }
    displayAllNum(count);
  }

}
