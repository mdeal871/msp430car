// ------------------------------------------------------------------------------
//  File Name : interrupt_ports.c
//
//  Description: This file contains all interrupts dealing with the MSP430 ports
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
#include "serial.h"

void Init_Switch_Interrupts(void) {
    // SW1 on P4.1
    P4IES |=  SW1;       // High-to-low edge trigger (button press pulls low)
    P4IFG &= ~SW1;       // Clear any pending flag
    P4IE  |=  SW1;       // Enable interrupt

    // SW2 on P2.3
    P2IES |=  SW2;
    P2IFG &= ~SW2;
    P2IE  |=  SW2;
}

//------------------------------------------------------------------------------
// Port 4 interrupt for switch 1, it is disabled for the duration
// of the debounce time. Debounce time is set for 1 second
#pragma vector=PORT4_VECTOR
__interrupt void switch1_interrupt(void) {
// Switch 1
    if (P4IFG & SW1) {
        P4IE  &= ~SW1;                                  // Clear Switch 1 flag
        P4IFG &= ~SW1;                                  // Clear TB0CCR1 interrupt flag
        TB0CCTL1 &= ~CCIFG;                             // CCR1 one-shot: fires ~1000ms from now
        TB0CCR1 = TB0R + TB0CCR1_INTERVAL;              // Enable TB0CCR1 interrupt
        TB0CCTL1 |= CCIE;


//-------------------------------------------------------------------------
//  ACTION:
//-------------------------------------------------------------------------

//-------------------------------------------------------------------------

        // HW6 TEMPORARY turn off backlite during debounce
//        P6OUT &= ~LCD_BACKLITE;
        }
//------------------------------------------------------------------------------
}
//------------------------------------------------------------------------------
// Port 2 interrupt for switch 2, it is disabled for the duration
// of the debounce time. Debounce time is set for 1 second
#pragma vector=PORT2_VECTOR
__interrupt void switch2_interrupt(void) {
// Switch 2
if (P2IFG & SW2) {
    P2IE  &= ~SW2;                                  // Clear Switch 2 flag
    P2IFG &= ~SW2;                                  // Clear TB0CCR2 interrupt flag
    TB0CCTL2 &= ~CCIFG;                             // CCR2 one-shot: fires ~1000ms from now
    TB0CCR2 = TB0R + TB0CCR2_INTERVAL;              // Enable TB0CCR2 interrupt
    TB0CCTL2 |= CCIE;

    //-------------------------------------------------------------------------
    //  ACTION:
    //-------------------------------------------------------------------------


    //-------------------------------------------------------------------------
}
}
//------------------------------------------------------------------------------
