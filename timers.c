// ------------------------------------------------------------------------------
//  File Name : timers.c
//
//  Description: This file contains the initializations and code for TimerB3 to
//  be used for motor PWM.
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


void Init_Timer_B3(void) {
//------------------------------------------------------------------------------
// SMCLK source, up count mode, PWM Right Side
// TB3.1 P6.1 L_FORWARD
// TB3.2 P6.2 R_FORWARD
// TB3.3 P6.3 L_REVERSE
// TB3.4 P6.4 R_REVERSE
// TB3.5 P6.5 LCD_BACKLITE
//------------------------------------------------------------------------------
TB3CTL = TBSSEL__SMCLK; // SMCLK
TB3CTL |= MC__UP; // Up Mode
TB3CTL |= TBCLR; // Clear TAR

TB3CCTL0 |= CCIE;           // enable CCR0 interrupt

PWM_PERIOD = WHEEL_PERIOD; // PWM Period [Set this to 50005]

TB3CCTL1 = OUTMOD_7;
LCD_BACKLITE_DIMING  = PERCENT_80;   // P6.0 LCD backlight

TB3CCTL2 = OUTMOD_7;
RIGHT_FORWARD_SPEED  = WHEEL_OFF;    // P6.1 R_FORWARD

TB3CCTL3 = OUTMOD_7;
LEFT_FORWARD_SPEED   = WHEEL_OFF;    // P6.2 L_FORWARD

TB3CCTL4 = OUTMOD_7;
RIGHT_REVERSE_SPEED  = WHEEL_OFF;    // P6.3 R_REVERSE

TB3CCTL5 = OUTMOD_7;
LEFT_REVERSE_SPEED   = WHEEL_OFF;    // P6.4 L_REVERSE
//------------------------------------------------------------------------------
}

void Init_Timer_B0(void){
//------------------------------------------------------------------------------
// Timer B0 initialization sets up both B0_0 and B0_1 to B0_2 and OverFlow
// 8,000,000 / 8 / 8 / [1/time]
// 1,000,000 / 8 / [1/time]
// 125,000 / [1/time]
// 1/time => 1 / 8msec => 1 / 0.005 = 200
// 125,000 / 200 = 625 – This is Capture Compare Register Interval
// 8usec per clock tick
//------------------------------------------------------------------------------
TB0CTL = TBSSEL__SMCLK; // SMCLK
TB0CTL |= MC__CONTINUOUS; // continuous mode
TB0CTL |= TBCLR; // Clear Count

TB0CTL |= ID__8; // Divide clock by 8
TB0EX0 = TBIDEX__8; // Second Divider - Divide clock by 8

TB0CCR0 = TB0CCR0_INTERVAL; // CCR0
TB0CCTL0 &= ~CCIFG; // Clear CCR0 interrupt flag
TB0CCTL0 |= CCIE; // CCR0 enable interrupt

TB0CCR1 = TB0CCR1_INTERVAL; // CCR1
TB0CCTL1 &= ~CCIFG; // Clear CCR1 interrupt flag
TB0CCTL1 &= ~CCIE; // CCR1 disable interrupt

TB0CCR2 = TB0CCR2_INTERVAL; // CCR2
TB0CCTL2 &= ~CCIFG; // Clear CCR2 interrupt flag
TB0CCTL2 &= ~CCIE; // CCR2 disable interrupt

TB0CTL &= ~TBIE; // Timer A0 overflow interrupt disable
TB0CTL &= ~TBIFG; // Clear Overflow Interrupt flag
}
