// ------------------------------------------------------------------------------
//  File Name : interrupts_timers.c
//
//  Description: This file contains all interrupts used by all timers/
//
//  Mason Deal
//  Mar 2026
//  Built with Code Composer Studio version 12.8.1.00005
//
//------------------------------------------------------------------------------
#include  "msp430.h"
#include  <string.h>
#include  "functions.h"
#include  "LCD.h"
#include  "ports.h"
#include  "macros.h"
#include  "globals.h"

static unsigned int display_count;

// Timer B3 Interrupt (PWM 6.25ms tick delay)
#pragma vector = TIMER3_B0_VECTOR
__interrupt void Timer3_B0_ISR(void){
    motor_delay_counter++;
}

//------------------------------------------------------------------------------
//  Timer B0 Interrupt Service Routines
//------------------------------------------------------------------------------

#pragma vector = TIMER0_B0_VECTOR
__interrupt void Timer0_B0_ISR(void){
// 10ms periodic interrupt
Time_Sequence++;
BL_Sequence++;
iot_elapsed++;   // free-running, never reset - used for display hold timing

// Set flag when target duration has elapsed
if (Time_Sequence >= Timer_B0_Target) {
    IFG_B0 = 1;
}

// Increment runtime counter
if (Runtime_FLG) {
Runtime_Count++;
}

// Update display

display_count++;
if(display_count >= 20){          // 20 * 10ms = 200ms
    display_count = 0;
    update_display = TRUE;
}

// Schedule ADC interrupt every 10ms instead of every 2.8us
ADCCTL0 |= ADCENC;
ADCCTL0 |= ADCSC;

TB0CCR0 += TB0CCR0_INTERVAL;      // schedule next interrupt
}

#pragma vector=TIMER0_B1_VECTOR
__interrupt void TIMER0_B1_ISR(void){
switch(__even_in_range(TB0IV,14)){

case 0: break; // No interrupt

// CCR1 � SW1 debounce
case 2:
    TB0CCTL1 &= ~CCIE;          // disable CCR1 interrupt
    TB0CCTL1 &= ~CCIFG;         // clear flag

    // TEMPORARY: Restore backlight only if SW2 debounce is not still active
    if (!(TB0CCTL2 & CCIE)) {
        display_count = 0;
    }

    sw1_pressed = PRESSED;

    P4IFG &= ~SW1;              // clear SW1 flag (change port/bit to yours)
    P4IE  |= SW1;               // re-enable SW1 interrupt


    //TB0CCR1 += TB0CCR1_INTERVAL;
    break;

// CCR2 � SW2 debounce
case 4:
    TB0CCTL2 &= ~CCIE;          // disable CCR2 interrupt
    TB0CCTL2 &= ~CCIFG;         // clear flag

    // TEMPORARY: Restore backlight only if SW1 debounce is not still active
    if (!(TB0CCTL1 & CCIE)) {
        display_count = 0;
    }

    sw2_pressed = PRESSED;

    P2IFG &= ~SW2;              // clear SW2 flag
    P2IE  |= SW2;               // re-enable SW2 interrupt
    //TB0CCR2 += TB0CCR2_INTERVAL;
    break;

case 14:
    // could increment seconds counter here
    break;

default: break;
}
}


