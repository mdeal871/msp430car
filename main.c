//------------------------------------------------------------------------------
//
//  Description: This file contains the Main Routine - "While" Operating System
//
//  Mason Deal
//  Apr 2026
//  Built with Code Composer Version: CCS12.4.0.00007_win64
//------------------------------------------------------------------------------

#include  "msp430.h"
#include  <string.h>
#include  "functions.h"
#include  "LCD.h"
#include  "ports.h"
#include  "macros.h"
#include  "globals.h"
#include "serial.h"

void main(void) {
//------------------------------------------------------------------------------
// Main Program
// This is the main routine for the program. Execution of code starts here.
// The operating system is Back Ground Fore Ground.
//
//------------------------------------------------------------------------------

    PM5CTL0 &= ~LOCKLPM5;
  // Disable the GPIO power-on default high-impedance mode to activate
  // previously configured port settings

//------------------------------------------------------------------------------
//  Start up sequence
//------------------------------------------------------------------------------

  Init_Ports();                        // Initialize Ports
  Init_Clocks();                       // Initialize Clock System
  Init_Conditions();                   // Initialize Variables and Initial Conditions
  Init_Timers();                       // Initialize Timers
  Init_Serial_UCA0(BAUD_115200);       // Initialize UCA0 serial (IOT)
  Init_Serial_UCA1(BAUD_115200);       // Initialize UCA1 serial (PC)
  Init_ADC();                          // Initialize ADC and interrupts
  Init_LCD();                          // Initialize LCD
  IOT_Init_Sequence();                 // Auto-connect: query SSID, get IP, start server
  Init_Switch_Interrupts();            // Initialize switch debounce interrupts

  // Display splash screen with name and P10 info
    lcd_BIG_mid();
    strcpy(display_line[0], "Mason Deal");
    strcpy(display_line[1], "Project 10");
    strcpy(display_line[2], " Demo Day ");
    display_changed = TRUE;
    update_display  = TRUE;
    Display_Process();

//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Beginning of the "While" Operating System
//------------------------------------------------------------------------------
  while(ALWAYS) {
     Display_Process();                // Update Display
     Menu_Process();                   // Process menu selections
     Serial_Process();                 // Handle all serial communication
     ADC_Process();                    // Update ADC values and line detection flags
     Run_Blackline();                  // Autonomous black line state machine (^B)
     Backlite_Flash();                 // Backlite flashing function. Ran when bl_status is set true
     IOT_Check_Reconnect();            // Auto-reconnect if WiFi was lost

     if (!Statemachine_Run) {
         Run_IOT_Wheels();
         Run_Driveup();
     }

     // Periodic network ping (every IOT_PING_INTERVAL ticks)
     if (iot_elapsed >= iot_next_ping_time) {
         IOT_Ping();
     }


     // SW1 opens the main menu after the splash screen
     if (sw1_pressed) {
         sw1_pressed = FALSE;
         menu_en      = TRUE;
         current_menu = MENU_MAIN;
         Reset_IOT_Wheels();
     }
  }
}
