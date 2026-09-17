// ------------------------------------------------------------------------------
//  File Name : display.c
//
//  Description: This file contains the functions corresponding to the display.
//  This file uses the following globals: update_display, display_changed, display_update
//  (found in globals.c)
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

unsigned int flash_target;

void Display_Process(void){
  if(update_display){
    update_display = 0;
    if(display_changed){
      display_changed = 0;
      Display_Update(0,0,0,0);
//      // Turn on backlight and reset timeout whenever content actually changes
//      P6OUT |= LCD_BACKLITE;
      backlight_timeout_count = 0;
    }
  }
}

void Backlite_On(void) {
    LCD_BACKLITE_DIMING  = PERCENT_80;
}

void Backlite_Off(void) {
    LCD_BACKLITE_DIMING  = WHEEL_OFF;
}

void Backlite_Flash(void) {
    if (bl_status) {
        // If the light is OFF but status is ON, this is the START of the flash
        if (LCD_BACKLITE_DIMING == WHEEL_OFF) {
            LCD_BACKLITE_DIMING = PERCENT_80;
            flash_target = Time_Sequence + 1000; // Set target ONCE
        }

        // Check if time has expired
        if (Time_Sequence >= flash_target) {
            LCD_BACKLITE_DIMING = WHEEL_OFF;
            bl_status = FALSE; // Reset the flag so it can be triggered again
        }
    }
}

