#include "SysTickInts.h"
#include "PLL.h"
#include "tm4c123gh6pm.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "inc/hw_memmap.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"

void PortF_Init(void);
void disable_interrupts(void);
void enable_interrupts(void);
void wait_for_interrupts(void);
void GPIO_Handler(void);

volatile unsigned long count = 0;
volatile unsigned long In, Out;
volatile unsigned long TOGGLE_COUNT = 1000;
volatile unsigned long int_count = 0;
volatile unsigned long PF_read;

/* main */
int main(void){
  PLL_Init();                 // bus clock at 80 MHz
  PortF_Init();
  count = 0;

  SysTick_Init(80000);        // initialize SysTick timer
  enable_interrupts();

  // initialize GPIO Interrupt
  GPIOIntDisable(GPIO_PORTF_BASE, GPIO_PIN_4);
  GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_4);
  GPIOIntRegister(GPIO_PORTF_BASE, GPIO_Handler);
  GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_LOW_LEVEL);
  GPIOIntEnable(GPIO_PORTF_BASE, GPIO_PIN_4);


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
// The input is 80000 => the interrupt service will get called every 1 msec
// Define TOGGLE_COUNT = 1000 for Blue LED to blink every 1 sec
void SysTick_Handler(void){
    count++;
    if (count == TOGGLE_COUNT - 1){
        count = 0;
        // Clear PF1
        GPIO_PORTF_DATA_R &= 0xFD;
        // toggle bit at PF2
        // With EOR logic: PF2 can be toggled in one line
        GPIO_PORTF_DATA_R ^= 0x04;

        // Alternative Method
        /*Out = GPIO_PORTF_DATA_R & 0x04;
        if (Out){
            GPIO_PORTF_DATA_R &= 0x00;
        }
        else{
            GPIO_PORTF_DATA_R |= 0x04;
        }*/

        // Read PORT F Register
        PF_read = GPIO_PORTF_DATA_R;
        }
    }

/* Interrupt service routine for GPIO (PF4 (SW1)) Interrupt */
void GPIO_Handler(void){
    //int_count++;
    GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_4);
    GPIO_PORTF_DATA_R = 0x02;   // LED Red

    // Read PORT F Register
    PF_read = GPIO_PORTF_DATA_R;
}
