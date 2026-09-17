// ------------------------------------------------------------------------------
//  File Name : init.c
//
//  Description: This file contains the initialization functions and uses the following
//  global variables: display_line[][], update display, and display[].
//
//  Mason Deal
//  Feb 2026
//  Built with Code Composer Studio version 12.8.1.00005
//
// ------------------------------------------------------------------------------

#include  "functions.h"
#include  "msp430.h"
#include  "macros.h"
#include  "globals.h"
#include "serial.h"

void Init_Conditions(void);
void Init_Ports(void);
void Init_Timers(void);


void Init_Ports(void){
    Init_Port1();
    Init_Port2();
    Init_Port3();
    Init_Port4();
    Init_Port5();
    Init_Port6();
}

void Init_Timers(void) {
    Init_Timer_B0();
    Init_Timer_B3();
}

void Init_Conditions(void){
//------------------------------------------------------------------------------

  int i;
  for(i=0;i<11;i++){
    display_line[0][i] = RESET_STATE;
    display_line[1][i] = RESET_STATE;
    display_line[2][i] = RESET_STATE;
    display_line[3][i] = RESET_STATE;
  }
  display_line[0][10] = 0;
  display_line[1][10] = 0;
  display_line[2][10] = 0;
  display_line[3][10] = 0;

  display[0] = &display_line[0][0];
  display[1] = &display_line[1][0];
  display[2] = &display_line[2][0];
  display[3] = &display_line[3][0];
  update_display = 0;

  // Initialize flat message display buffers to spaces
  strcpy(rx_message,      "          ");
  strcpy(current_message, "          ");


// Interrupts are disabled by default, enable them.
  enable_interrupts();
//------------------------------------------------------------------------------
}


