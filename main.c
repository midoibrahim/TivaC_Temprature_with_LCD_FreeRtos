#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "ADC.h"
#include "LCD.h"

QueueHandle_t xUARTQueue;
QueueHandle_t xLCDQueue;
SemaphoreHandle_t xSemaphore;
const TickType_t xDelayinf = portMAX_DELAY;
char Txt1[4]; 
char Txt2[4];

void toString (int tim, char text []){
		int number=tim;
	int digits=0;
	while (number) {
        number /= 10;
        digits++;
    }
	for (int j =0; j<digits; j++){
		text[j]='0';
	}
	//put numbers in char array
        int i = digits;
	while (tim != 0){
   i--;		
		int rem=tim%10;
		text[i]=rem+'0';
		tim=(tim/10)|0;
	}
	text[digits]='\0';//add null terminator
}

void printchar(char c){
while((UART0_FR_R&(1<<5))!=0);
UART0_DR_R=c;
}

void print(char *string){
  while(*string){
  printchar(*(string++));
  }
}


void Init(){
SYSCTL_RCGCGPIO_R |= 0x00000020;      //Initialize clock to PORTF
while((SYSCTL_PRGPIO_R&0x00000020) == 0){}  //safety for clock initialization
GPIO_PORTF_LOCK_R = 0x4C4F434B;
GPIO_PORTF_CR_R = 0x1F;       //Enable change to PORTF
GPIO_PORTF_DIR_R = 0x0E;      //Make led ports as output
GPIO_PORTF_DEN_R = 0x1F;      // digital enable to pins
GPIO_PORTF_PUR_R = 0x11;
SYSCTL_RCGCUART_R|=0X0001;
SYSCTL_RCGCGPIO_R |= 0x00000001;
UART0_CTL_R &= ~0x0001;
UART0_CC_R=0X0;
UART0_IBRD_R=104;
UART0_FBRD_R=11;
UART0_LCRH_R=(0x3<<5);
GPIO_PORTA_AFSEL_R|=0X03;
GPIO_PORTA_PCTL_R=0X011;
GPIO_PORTA_DEN_R|=0X03;
UART0_CTL_R=0x0301;
//9600 1 stopbit no parity no fifo
adc_init2();
}

void ADC_Read_Task(){
	xSemaphoreTake( xSemaphore, 0 );
	unsigned int AdcValue;
	int Temp;
	 while(1){
		 xSemaphoreTake( xSemaphore, portMAX_DELAY );
		//Adc value range from 0 to 4095
		//ref used is 3.3V
		AdcValue = ADC0_SSFIFO3_R;									//Read ADC
		Temp = 147.5-(247.5 * AdcValue)/4096;	//Temp in C
		xQueueSendToBack(xUARTQueue,&Temp,1000 / portTICK_PERIOD_MS);
		xQueueSendToBack(xLCDQueue,&Temp,1000 / portTICK_PERIOD_MS); 
	 }
}


void UART_TASK(void){
	int Temp;
    while(1){	
				xQueueReceive(xUARTQueue,&Temp,xDelayinf);
				toString(Temp,Txt1);
			  print("\n\r\nTemperature is : ");
				print(Txt1);
				print(" Degree Celsius");			
				}	
}


void vApplicationIdleHook(void){}
	
void ADC0SS3_Handler(void){
	BaseType_t xHigherPriorityTaskWoken= pdFALSE;
	xSemaphoreGiveFromISR(xSemaphore, &xHigherPriorityTaskWoken );
	ADC0_ISC_R = 8;          /* clear coversion clear flag bit*/
	portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
	//xSemaphoreGive(xSemaphore);
	//if(xHigherPriorityTaskWoken){taskYIELD();}
}




void LCD_TASK(void){
	int Temp;
    while(1){	
				xQueueReceive(xLCDQueue,&Temp,xDelayinf);
				toString(Temp,Txt2);	
			//LCD_clear();
		  LCD_line(1); 
			LCD_display("TEMPRATURE: ");   
	  	LCD_display(Txt2);  		
		}	
}

int main(void)
{
	Init();
	LCD_start();
	LCD_clear();
	xUARTQueue = xQueueCreate( 10,sizeof(int) );
	xLCDQueue= xQueueCreate(10,sizeof(int) );
	
	vSemaphoreCreateBinary(xSemaphore);
	xTaskCreate(ADC_Read_Task,"ADC_Read_Task",100,NULL,1,0);
	xTaskCreate(UART_TASK,"UART_TASK",100,NULL,2,0);
	xTaskCreate(LCD_TASK,"LCD_TASK",100,NULL,2,0);
	vTaskStartScheduler();

}