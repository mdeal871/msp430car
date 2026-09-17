// ------------------------------------------------------------------------------
//  File Name : led.c
//
//  Description: This file contains all functions corresponding to the LED indicators
//  Currently, this file only contains the Carlson_StateMachine that switches the
//  Green and red LEDs back and forth.
//
//  Mason Deal
//  Feb 2026
//  Built with Code Composer Studio version 12.8.1.00005
//
// ------------------------------------------------------------------------------
#include  "functions.h"
#include  "msp430.h"
#include  "macros.h"
#include "globals.h"

void Init_LEDs(void);
void Carlson_StateMachine(void);

void Init_LEDs(void){
//------------------------------------------------------------------------------
// LED Configurations
//------------------------------------------------------------------------------
// Turns on both LEDs
  P1OUT &= ~RED_LED;
  P6OUT &= ~GRN_LED;
//------------------------------------------------------------------------------
}

//void Carlson_StateMachine(void){
//    switch(Time_Sequence){
//      case 250:                        //
//        if(one_time){
//          Init_LEDs();
//          lcd_BIG_mid();
//          display_changed = 1;
//          one_time = 0;
//        }
//        Time_Sequence = 0;             //
//        break;
//      case 200:                        //
//        if(one_time){
////          P1OUT &= ~RED_LED;            // Change State of LED 4
//          P6OUT |= GRN_LED;            // Change State of LED 5
//          one_time = 0;
//        }
//        break;
//      case 150:                         //
//        if(one_time){
//          P1OUT |= RED_LED;            // Change State of LED 4
//          P6OUT &= ~GRN_LED;            // Change State of LED 5
//          one_time = 0;
//        }
//        break;
//      case 100:                         //
//        if(one_time){
////          lcd_4line();
//          lcd_BIG_bot();
//          P6OUT |= GRN_LED;            // Change State of LED 5
//          display_changed = 1;
//          one_time = 0;
//        }
//        break;
//      case  50:                        //
//        if(one_time){
//          one_time = 0;
//        }
//        break;                         //
//      default: break;
//    }
//}
