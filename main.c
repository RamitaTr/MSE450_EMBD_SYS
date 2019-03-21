#include "SysTickInts.h"
#include "PLL.h"
#include "tm4c123gh6pm.h"

void DAC_Init(void);
void PortA_Init(void);
void PortE_Init(void);
void PortF_Init(void);
void disable_interrupts(void);
void enable_interrupts(void);
void wait_for_interrupts(void);
void DAC_Out(unsigned long);

const unsigned char SineWave[32] = {8,9,10,11,12,13,14,15,15,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,1,1,2,3,4,5,6,7};
volatile unsigned char Index = 0;           // Index varies from 0 to 31

volatile unsigned long count = 0;
volatile unsigned long In, Out;
const unsigned long note[8] = {9582,8503,7576,7163,6377,5682,5060,4780}; //C,D,E,F,G,A,B,C
volatile unsigned long period;


/* main */
int main(void){
    PLL_Init();
    PortE_Init();
    PortA_Init();
    PortF_Init();
    DAC_Init();
    period = note[0];
    SysTick_Init(period);        // initialize SysTick timer
    enable_interrupts();
    while(1){                   // interrupts every 1ms
        wait_for_interrupts();
    }
}


/* Initialize DAC on PB2-0 */
// Initialize 3-bit DAC
void DAC_Init(void) {
    SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOB;   // Activate clock for PortB
    while ((SYSCTL_PRGPIO_R & SYSCTL_RCGC2_GPIOB) == 0)
    {};                                     // Wait until PortB is ready
    GPIO_PORTB_AMSEL_R &= ~0x0F;            // Disable analog on PB3-0
    GPIO_PORTB_PCTL_R &= ~0x0000FFFF;       // Use PB3-0 as GPIO
    GPIO_PORTB_DIR_R |= 0x0F;               // Configure PB3-0 out
    GPIO_PORTB_AFSEL_R &= ~0x0F;            // Disable alt function on PB3-0
    GPIO_PORTB_PUR_R &= ~0x0F;              // Disable pull-up resistor on PB3-0
    GPIO_PORTB_DEN_R |= 0x0F;               // Enable digital I/O on PB3-0
}

/* Initialize PortE GPIOs */
void PortA_Init(void) {
    SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOA;           // activate clock for PortA
    while ((SYSCTL_PRGPIO_R & SYSCTL_RCGC2_GPIOA) == 0)
    {};                          // wait until PortA is ready
    GPIO_PORTA_AMSEL_R &= ~0x3C;            // Disable analog on PA5-2
    GPIO_PORTA_PCTL_R &= ~0x00FFFF00;       // Use PA5-2 as GPIO
    GPIO_PORTA_DIR_R &= ~0x3C;               // Configure PA5-2 in
    GPIO_PORTA_AFSEL_R &= ~0x3C;            // Disable alt function on PA5-2
    GPIO_PORTA_PDR_R |= 0x3C;              // Enable pull-down resistor on PA5-2
    GPIO_PORTA_DEN_R |= 0x3C;               // Enable digital I/O on PA5-2
}

/* Initialize PortA GPIOs */
void PortE_Init(void) {
    SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOE;           // activate clock for PortE
    while ((SYSCTL_PRGPIO_R & SYSCTL_RCGC2_GPIOE) == 0)
    {};                          // wait until PortE is ready
    GPIO_PORTE_AMSEL_R &= ~0x0F;            // Disable analog on PE3-0
    GPIO_PORTE_PCTL_R &= ~0x0000FFFF;       // Use PE3-0 as GPIO
    GPIO_PORTE_DIR_R &= ~0x0F;               // Configure PE3-0 in
    GPIO_PORTE_AFSEL_R &= ~0x0F;            // Disable alt function on PE3-0
    GPIO_PORTE_PDR_R |= 0x0F;              // Enable pull-down resistor on PE3-0
    GPIO_PORTE_DEN_R |= 0x0F;               // Enable digital I/O on PE3-0
}

/* Initialize PortF GPIOs */
void PortF_Init(void) {
    SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOF;           // activate clock for PortF
    while ((SYSCTL_PRGPIO_R & SYSCTL_RCGC2_GPIOF) == 0)
    {};                          // wait until PortF is ready
    GPIO_PORTF_LOCK_R = 0x4C4F434B;         // unlock GPIO PortF
    GPIO_PORTF_CR_R |= 0x0F;                 // allow changes to PF3-0
    GPIO_PORTF_AMSEL_R &= 0x0F;              // disable analog on PF3-0
    GPIO_PORTF_PCTL_R &= ~0x0000FFFF;         // use PF4-0 as GPIO
    GPIO_PORTF_DIR_R |= 0x0F;                // configure PF3-0 out
    GPIO_PORTF_AFSEL_R &= ~0x0F;              // disable alt function on PF3-0
    GPIO_PORTF_PUR_R &= ~0x0F;                // disable pull-up on PF3-0
    GPIO_PORTF_DEN_R |= 0x0F;                // enable digital I/O on PF3-0
}

/* Disable interrupts by setting the I bit in the PRIMASK system register */
void disable_interrupts(void) {
    __asm("    CPSID  I\n"
          "    BX     LR");
}


/* Enable interrupts by clearing the I bit in the PRIMASK system register */
void enable_interrupts(void) {
    __asm("    CPSIE  I\n"
          "    BX     LR");
}


/* Enter low-power mode while waiting for interrupts */
void wait_for_interrupts(void) {
    __asm("    WFI\n"
          "    BX     LR");
}

/* DAC_Out */
void DAC_Out(unsigned long data){
    GPIO_PORTB_DATA_R = data;
    Out = GPIO_PORTB_DATA_R;
}

/* Interrupt service routine for SysTick Interrupt */
// Executed every 12.5ns*(period)
void SysTick_Handler(void){
//    In = GPIO_PORTE_DATA_R;
//    count++;
    if (GPIO_PORTE_DATA_R&0x0F || GPIO_PORTA_DATA_R&0x3C){
        if (GPIO_PORTE_DATA_R&0x01) {
            period = note[0];
            GPIO_PORTF_DATA_R = 0x02;
        }
        else if (GPIO_PORTE_DATA_R&0x02) {
            period = note[1];
            GPIO_PORTF_DATA_R = 0x04;
        }
        else if (GPIO_PORTE_DATA_R&0x04) {
            period = note[2];
            GPIO_PORTF_DATA_R = 0x08;
        }
        else if (GPIO_PORTE_DATA_R&0x08) {
            period = note[3];
            GPIO_PORTF_DATA_R = 0x0A;
        }
        else if (GPIO_PORTA_DATA_R&0x04) {
            period = note[4];
            GPIO_PORTF_DATA_R = 0x0C;
        }
        else if (GPIO_PORTA_DATA_R&0x08) {
            period = note[5];
            GPIO_PORTF_DATA_R = 0x0E;
        }
        else if (GPIO_PORTA_DATA_R&0x10) {
            period = note[6];
            GPIO_PORTF_DATA_R = 0x06;
        }
        else if (GPIO_PORTA_DATA_R&0x20) {
            period = note[7];
            GPIO_PORTF_DATA_R = 0x02;
        }
        DAC_Out(SineWave[Index]);
        Index = (Index+1)&0x0F;
        SysTick_Init(period);
    }
    else {
        DAC_Out(0);
        GPIO_PORTF_DATA_R = 0x00;
    }
}
