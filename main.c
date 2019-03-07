#include "SysTickInts.h"
#include "PLL.h"
#include "tm4c123gh6pm.h"

void PortF_Init(void);
void disable_interrupts(void);
void enable_interrupts(void);
void wait_for_interrupts(void);

volatile unsigned long count = 0;
volatile unsigned long In, Out;


/* main */
int main(void){
  PLL_Init();                 // bus clock at 80 MHz
  PortF_Init();
  count = 0;

  SysTick_Init(80000);        // initialize SysTick timer
  enable_interrupts();

  while(1){                   // interrupts every 1ms
      wait_for_interrupts();
  }
}


/* Initialize PortF GPIOs */
void PortF_Init(void) {
    SYSCTL_RCGC2_R |= 0x00000020;           // activate clock for PortF
    while ((SYSCTL_PRGPIO_R & 0x00000020) == 0)
    {};                          // wait until PortF is ready
    GPIO_PORTF_LOCK_R = 0x4C4F434B;         // unlock GPIO PortF
    GPIO_PORTF_CR_R = 0x1F;                 // allow changes to PF4-0
    GPIO_PORTF_AMSEL_R = 0x00;              // disable analog on PortF
    GPIO_PORTF_PCTL_R = 0x00000000;         // use PF4-0 as GPIO
    GPIO_PORTF_DIR_R = 0x0E;                // PF4,PF0 in, PF3-1 out
    GPIO_PORTF_AFSEL_R = 0x00;              // disable alt function on PF
    GPIO_PORTF_PUR_R = 0x11;                // enable pull-up on PF0,PF4
    GPIO_PORTF_DEN_R = 0x1F;                // enable digital I/O on PF4-0
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


/* Interrupt service routine for SysTick Interrupt */
// Executed every 12.5ns*(period)
void SysTick_Handler(void){
    
	
}
