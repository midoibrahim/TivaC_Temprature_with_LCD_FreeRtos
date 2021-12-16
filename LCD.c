#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "semphr.h"
#include <stdint.h>
#include <stdbool.h>
#include "TM4C123GH6PM.h"
#include "LCD.h"

void delayUs(unsigned int i){
    int volatile s=0;
    while(s<100){s++;}
}

void delayMs(unsigned int i){
    int volatile s=0;
    while(s<10000){s++;}
}

void LCD_command(unsigned char command)
{
    LCD_CTRL_DATA = 0;          /* RS = 0, R/W = 0 */
    LCD_DATA_DATA = command;
    LCD_CTRL_DATA = EN;         /* pulse E */
    delayUs(1);		/* Enable pulse Width */
    LCD_CTRL_DATA = 0;
    if (command < 4)
        delayMs(2);         	/* command 1 and 2 needs up to 1.64ms */
    else
        delayUs(40);        /* all others 40 us */
}

void LCD_start(void){
   /* GPIO initialization */
    SYSCTL_RCGCGPIO_R |= 0x00000003;
    while((SYSCTL_PRGPIO_R&0x00000003) == 0);
    GPIO_PORTB_DIR_R = 0xFF;
    GPIO_PORTA_DIR_R = 0xFF;
    GPIO_PORTA_DEN_R = 0xFF;
    GPIO_PORTB_DEN_R = 0xFF;
   /* LCD initialization sequence */
    delayMs(20);           // Wait >15 ms after power is applied
    LCD_command(Function_set_8bit);		// command 0x30 = Wake up
    delayMs(5);				// must wait 5ms, busy flag not available
    LCD_command(Function_set_8bit);		// command 0x30 = Wake up #2
    delayUs(100);			// must wait 100us, busy flag not available
    LCD_command(Function_set_8bit);		// command 0x30 = Wake up #3
    delayUs(100);			// must wait 100us, busy flag not available
  // Prepare LCD Operation and Function  
    LCD_command(Function_set_8bit);      // set 8-bit data, 2-line, 5x7 font 
    LCD_command(moveCursorRight);      // move cursor right 
    LCD_command(clear_display);      // clear screen, move cursor to home 
    LCD_command(cursorBlink);      // turn on display, cursor blinking 
}

void LCD_data(unsigned char data){       
    LCD_CTRL_DATA = RS;    /* RS = 1, R/W = 0 */
    LCD_DATA_DATA = data;
    LCD_CTRL_DATA = EN | RS;   /* pulse E */
    delayUs(1);
    LCD_CTRL_DATA = 0;
    delayUs(40);
}

void LCD_clear (void){
    LCD_command(clear_display);
}

void LCD_line(uint8_t line){
    if (line == 1)
        LCD_command(FirstRow);
    else if (line == 2)
        LCD_command(SecondRow);
}


void LCD_display(char* name){
    for(int i=0; i<strlen(name);i++){
        LCD_data(name[i]);
    }
}

