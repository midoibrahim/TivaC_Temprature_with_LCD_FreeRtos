#include "ADC.h"

void adc_init (){
	// GIVING CLOCK
	SYSCTL_RCGCGPIO_R|=(1<<4);  // give clock to port E 
	SYSCTL_RCGCADC_R |=  1; /* AD0 clock enable*/ 
	
	GPIO_PORTE_AFSEL_R |= (1UL<<3);   /* enable alternate function */
  GPIO_PORTE_DEN_R &= ~(1UL<<3);    /* disable digital function */
  GPIO_PORTE_AMSEL_R |= (1UL<<3);   /* enable analog function */
	/* initialize ADC0 */
    ADC0_ACTSS_R &= ~8;          /* disable SS3 during configuration */
    ADC0_EMUX_R &= ~0xF000;
    ADC0_SSMUX3_R = 0;           /* get input from channel 0 */
    ADC0_SSCTL3_R |= (1<<1)|(1<<2);       /* take chip temperature one sample at a time, set flag at 1st sample */
    //enable ADC0 interrupt
		ADC0_IM_R |= (1<<3);  //unmask ADC0 seq 3 interrupt
		NVIC_EN0_R|= 0x00010001;
		ADC0_ACTSS_R |= 8;           /* enable ADC0 sequencer 3 */
}

void adc_init2()
{
	  /* enable clocks */
    SYSCTL_RCGCADC_R |= 1;       /* enable clock to ADC0 */
    SYSCTL_RCGCWTIMER_R |= 1;    /* enable clock to WTimer Block 0 */
 
    /* initialize ADC0 */
    ADC0_ACTSS_R &= ~8;          /* disable SS3 during configuration */
    ADC0_EMUX_R &= ~0xF000;
    ADC0_EMUX_R |= 0x5000;       /* timer trigger conversion seq 0 */
    ADC0_SSMUX3_R = 0;           /* get input from channel 0 */
    ADC0_SSCTL3_R |= 0x0E;       /* take chip temperature, set flag at 1st sample */
		ADC0_IM_R |= (1<<3);  //unmask ADC0 seq 3 interrupt
		NVIC_EN0_R|= (1<<17);
		NVIC_PRI4_R |=(111<<13);
    ADC0_ACTSS_R |= 8;           /* enable ADC0 sequencer 3 */
 
    /* initialize wtimer 0 to trigger ADC at 1 sample/sec */
    WTIMER0_CTL_R = 0;           /* disable WTimer before initialization */
    WTIMER0_CFG_R = 0x04;        /* 32-bit option */
    WTIMER0_TAMR_R = 0x02;       /* periodic mode and down-counter */
    WTIMER0_TAILR_R = 16000000;  /* WTimer A interval load value reg (1 s) */
    WTIMER0_CTL_R |= 0x20;       /* timer triggers ADC */
    WTIMER0_CTL_R |= 0x01;       /* enable WTimer A after initialization */
}