#include "SysTickInts.h"
#include "PLL.h"
#include "tm4c123gh6pm.h"

void PortF_Init(void);
void disable_interrupts(void);
void enable_interrupts(void);
void wait_for_interrupts(void);

void PWM_Init(void);

volatile unsigned long count = 0;
volatile unsigned long In, Out;
unsigned int TOGGLE_COUNT = 1000;
volatile int PortF = 0;
volatile int temp = 0;
volatile int dis_blue = 0;


/* main */
int main(void){
  PortF_Init();
  PWM_Init();
  count = 0;

  SysTick_Init(16000);        // initialize SysTick timer
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
    // Interrupt Initialization
    GPIO_PORTF_IM_R &= ~0x10;   // Masked PF4
    GPIO_PORTF_IS_R &= ~0x10;   // Edge Trigger
    GPIO_PORTF_IBE_R &= ~0x10;  // Single Edge
    GPIO_PORTF_IEV_R &= ~0x10;  // Falling Edge
    GPIO_PORTF_ICR_R = 0x10;    // Clear Flag for PF4
    GPIO_PORTF_IM_R |= 0x10;    // Unmasked PF4
    NVIC_EN0_R = 0x40000000;    // enable interrupt 30 in NVIC
    enable_interrupts();
}

void PWM_Init(void){
    SYSCTL_RCGC0_R |= 0x00100000;       // 1) Activate PWM clock
    SYSCTL_RCGC2_R |= 0x00000002;       // 2) Activate clock for PortB
    while((SYSCTL_PRGPIO_R&0x02) == 0)  // Wait until PortB is ready for access
    {};
    GPIO_PORTB_AFSEL_R |= 0x40;         // 3) Enable alt funct on PB6 (0100.0000)
    GPIO_PORTB_DEN_R |= 0x40;           // enable digital I/O on PB6
    GPIO_PORTB_PCTL_R &= ~0x0F000000;   // Clear PMC6 (Port Mux Control 6)
    GPIO_PORTB_PCTL_R |= 0x04000000;    // 4) Configure PB6 as PWM
    SYSCTL_RCC_R |= 0x00100000;         // 5) Use PWM divider
    SYSCTL_RCC_R &= ~0x000E0000;        // Clear PWMDIV
    SYSCTL_RCC_R |= 0x00060000;         // Set divider /16: SysClock = 16MHz, PWM clock = 1MHz
    PWM0_0_CTL_R = 0;                   // 6) Configure PWM generator for count-down mode
    PWM0_0_GENA_R = 0x8C;               // Drive pwmA High on Load, low on CMPA down
    //PWM0_0_GENB_R = 0x80C;            // Drive pwmA High on Load, low on CMPB down
    PWM0_0_LOAD_R = 10000 - 1;          // 7) Set period (-1 for counting down to 0)
                                        // (PWM clock source 8MHz / Desire PWM frequency 100MHz)
                                        // 80000 ticks per period
    PWM0_0_CMPA_R = 9999 - 1;        // 8) Set the pulse width (zero duty cycle)
    PWM0_0_CTL_R |= 0x00000001;         // 9) Start PWM0
    PWM0_ENABLE_R |= 0x00000001;        // 10) Enable PWM outputs
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
// TOGGLE_COUNT = 1000 call SysTick Interrupt every 1 sec
void SysTick_Handler(void){

}

// Interrupt service routine for PF4
void GPIO_Handler(void){
    GPIO_PORTF_ICR_R = 0x10;    // Acknowledge Flag for PF4
    PWM0_0_CMPA_R-=1000;
    if (PWM0_0_CMPA_R <= 1000){
        PWM0_0_CMPA_R = 9999 - 1;
    }
    temp = PWM0_0_CMPA_R;
}
