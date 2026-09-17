// ------------------------------------------------------------------------------
//  File Name: menus.c
//
//  Description: LCD menu system.
//    MENU_MAIN         - select a page with thumb wheel + SW1
//    MENU_ADC_MONITOR  - live line detector values (raw / normalized)
//    MENU_SERIAL_MONITOR - last messages from PC and IOT
//    MENU_IOT          - IOT connection status (SSID, IP) and last command
//
//  Navigation:
//    SW1 - enter selected page / action within page
//    SW2 - return to main menu from any page
//
//  Mason Deal
//  Apr 2026
//  Built with Code Composer Studio version 12.8.1.00005
// ------------------------------------------------------------------------------
#include "msp430.h"
#include <string.h>
#include "functions.h"
#include "LCD.h"
#include "ports.h"
#include "macros.h"
#include "globals.h"
#include "serial.h"

// ------------------------------------------------------------------------------
//  Clear_Display  -  blank all four lines and force a refresh
// ------------------------------------------------------------------------------
void Clear_Display(void) {
    int i;
    for (i = 0; i < 4; i++) {
        strcpy(display_line[i], "          ");
    }
    display_changed = TRUE;
    update_display  = TRUE;
}

// ------------------------------------------------------------------------------
//  Tuned_Index_4  -  map 0-4095 ADC value to 4 menu slots
//  Band boundaries divide the range into four equal quarters.
// ------------------------------------------------------------------------------
static unsigned int Tuned_Index_4(unsigned int adc_val) {
    if (adc_val < 400) return 0;
    if (adc_val < 1000) return 1;
    if (adc_val < 2000) return 2;
    return 3;
}

// ==============================================================================
//  MENU_MAIN
//  Thumbwheel selects from 4 items. SW1 enters. SW2 has no action here.
//  Items: ADC Mon | Serial Mon | IOT | Demo Day
// ==============================================================================
static const char* main_menu_items[4] = {
    "ADC Mon   ", "Serial Mon", "IOT       ", "Demo Day  "
};

void Menu_Main(void) {
    unsigned int idx = Tuned_Index_4(thumb_adc_value);

    strcpy(display_line[0], "  SELECT  ");
    strncpy(display_line[1], (char*)main_menu_items[idx], 10);
    display_line[1][10] = '\0';
    strcpy(display_line[2], "          ");
    strcpy(display_line[3], "SW1=Enter ");
    display_changed = TRUE;

    if (sw1_pressed) {
        sw1_pressed = 0;
        switch (idx) {
            case 0: current_menu = MENU_ADC_MONITOR;    break;
            case 1: current_menu = MENU_SERIAL_MONITOR; break;
            case 2: current_menu = MENU_IOT;            break;
            case 3: current_menu = MENU_DEMO;           break;
            default: break;
        }
    }
}

// ==============================================================================
//  MENU_ADC_MONITOR
//  Lines 0/1: left detector label + value
//  Lines 2/3: right detector label + value
//  SW1 toggles raw / normalized view
// ==============================================================================
void Menu_ADC_Monitor(void) {
    static unsigned char mode = 0;


    unsigned int ls = left_detect_adc_value  >> 2;
    unsigned int rs = right_detect_adc_value >> 2;
    unsigned int ln = ((ls - left_white)  * 1000) / (left_black  - left_white);
    unsigned int rn = ((rs - right_white) * 1000) / (right_black - right_white);
    if (ln > 1000) ln = 1000;
    if (rn > 1000) rn = 1000;

    strcpy(display_line[0], "L_DETECT  ");
    strcpy(display_line[2], "R_DETECT  ");

    unsigned int lv = (mode == 0) ? ls : ln;
    unsigned int rv = (mode == 0) ? rs : rn;

    display_line[1][0] = '0' + (lv / 1000) % 10;
    display_line[1][1] = '0' + (lv / 100)  % 10;
    display_line[1][2] = '0' + (lv / 10)   % 10;
    display_line[1][3] = '0' + (lv)        % 10;
    display_line[1][4] = ' ';
    strcpy(display_line[1] + 5, (mode == 0) ? "RAW  " : "NORM ");
    display_line[1][10] = '\0';

    display_line[3][0] = '0' + (rv / 1000) % 10;
    display_line[3][1] = '0' + (rv / 100)  % 10;
    display_line[3][2] = '0' + (rv / 10)   % 10;
    display_line[3][3] = '0' + (rv)        % 10;
    display_line[3][4] = ' ';
    strcpy(display_line[3] + 5, (mode == 0) ? "RAW  " : "NORM ");
    display_line[3][10] = '\0';

    display_changed = TRUE;

    if (sw1_pressed) { sw1_pressed = 0; mode = !mode; }
    if (sw2_pressed) { sw2_pressed = 0; current_menu = MENU_MAIN; }
}

// ==============================================================================
//  MENU_SERIAL_MONITOR
//  Line 0: "PC RX:"   Line 1: last PC message
//  Line 2: "IOT RX:"  Line 3: last IOT message
//  SW1 clears both buffers
// ==============================================================================
void Menu_Serial_Monitor(void) {
    strcpy(display_line[0], "PC RX:    ");
    strncpy(display_line[1], rx_message, 10);
    display_line[1][10] = '\0';
    strcpy(display_line[2], "IOT RX:   ");
    strncpy(display_line[3], current_message, 10);
    display_line[3][10] = '\0';
    display_changed = TRUE;

    if (sw1_pressed) {
        sw1_pressed = 0;
        strcpy(rx_message,      "          ");
        strcpy(current_message, "          ");
        display_changed = TRUE;
    }
    if (sw2_pressed) { sw2_pressed = 0; current_menu = MENU_MAIN; }
}

// ==============================================================================
//  MENU_IOT
//  Displays IOT connection info populated by IOT_Parse_Response().
//
//  Line 1: SSID (centered, max 10 chars)
//  Line 2: "IP Address" (centered label)
//  Line 3: first two IP groups  e.g. "  192.168 "
//  Line 4: last  two IP groups  e.g. "   1.50   "
//
//  When iot_display_ready == 0 the page shows "Waiting..."
// ==============================================================================
void Menu_IOT(void) {
    static unsigned char cmd_display_active = 0;

    // If a motion command was just received, hold its display for 2 seconds.
    if (iot_elapsed < iot_cmd_display_until) {
        if (!cmd_display_active) {
            // First iteration of hold: clear stale content so the big command
            // display isn't fighting with leftover IP/SSID text on other lines.
            cmd_display_active = 1;
//            lcd_BIG_mid();
//            strcpy(display_line[0], "          ");
//            strcpy(display_line[1], "          ");
//            strcpy(display_line[2], "          ");
//            strcpy(display_line[3], "          ");
//            display_changed = TRUE;
        }
        if (sw2_pressed) { sw2_pressed = 0; current_menu = MENU_MAIN; }
        return;
    }
    cmd_display_active = 0;   // hold expired, resume normal display

    if (iot_display_ready) {
        lcd_4line();
        strncpy(display_line[0], iot_ssid,     10);  display_line[0][10] = '\0';
        strncpy(display_line[1], iot_ip_label,  10);  display_line[1][10] = '\0';
        strncpy(display_line[2], iot_ip_line3,  10);  display_line[2][10] = '\0';
        strncpy(display_line[3], iot_ip_line4,  10);  display_line[3][10] = '\0';
    } else {
        strcpy(display_line[0], " IOT Menu ");
        strcpy(display_line[1], " Waiting  ");
        strcpy(display_line[2], " for conn ");
        strcpy(display_line[3], "          ");
    }
    display_changed = TRUE;

    if (sw2_pressed) { sw2_pressed = 0; current_menu = MENU_MAIN; }
}

// ==============================================================================
//  MENU_DEMO  -  single demo day menu, phase-driven internally
//
//  Phase 0: Calibration
//    Line 1: "CAL WHITE" or "CAL BLACK" instruction
//    Line 2: live left ADC value
//    Line 3: live right ADC value
//    Line 4: button prompt
//    SW1 = capture white, SW2 = capture black -> advance to phase 1
//
//  Phase 1: WiFi course operation
//    Line 1: "Arrived 0X" (set by ^A command) or blank between pads
//    Line 2: IP line 3  e.g. "  10.155  "
//    Line 3: IP line 4  e.g. " .102.55  "
//    Line 4: last cmd (5 chars) + "  " + timer (3 digits + 's')
//    Timer starts on first motion command received (demo_started flag).
//    Automatically advances to phase 2 when ^B blackline command is received.
//
//  Phase 2: Autonomous black line
//    Line 1: BL event string written by Run_Blackline (do NOT overwrite)
//    Line 2: IP line 3
//    Line 3: IP line 4
//    Line 4: last cmd + timer  (same layout as phase 1)
//    Returns to phase 1 layout once Statemachine_Run goes low.
//
//  SW2 exits back to MENU_MAIN from phase 0 or 1 only (not during autonomous run).
// ==============================================================================

// Internal phase variable - persists across calls so the menu remembers where it is.
// Reset to 0 when MENU_DEMO is first entered (handled in Menu_Process transition block).
static unsigned char demo_phase = 0;

// Called by Menu_Process on MENU_DEMO entry to reset phase back to calibration.
void Demo_Reset_Phase(void) { demo_phase = 0; }
void Demo_Set_Phase(unsigned char phase) { demo_phase = phase; }

void Menu_Demo(void) {
    unsigned int s = demo_seconds;

    if (!bl_status && !Statemachine_Run) {
            Backlite_Off();
        }

    switch (demo_phase) {

    // ------------------------------------------------------------------
    // Phase 0: calibration entry screen - prompt user to begin
    // SW1 starts white calibration. SW2 exits back to main menu.
    // ------------------------------------------------------------------
    case 0: {
        P2OUT |= IR_LED;
        static unsigned char cal_step = 0;  // 0 = white, 1 = black

        unsigned int ls = left_detect_adc_value >> 2;
        unsigned int rs = right_detect_adc_value >> 2;

        if (cal_step == 0) {
            P2OUT |= IR_LED;
            strcpy(display_line[0], "CAL WHITE ");
            strcpy(display_line[3], "SW1=White ");
        } else {
            P2OUT |= IR_LED;
            strcpy(display_line[0], "CAL BLACK ");
            strcpy(display_line[3], "SW2=Black ");
        }

        // Lines 1-2: live sensor readings so user can position over surface
        display_line[1][0] = 'L'; display_line[1][1] = ':'; display_line[1][2] = ' ';
        display_line[1][3] = '0' + (ls / 1000) % 10;
        display_line[1][4] = '0' + (ls / 100)  % 10;
        display_line[1][5] = '0' + (ls / 10)   % 10;
        display_line[1][6] = '0' + (ls)        % 10;
        display_line[1][7] = ' '; display_line[1][8] = ' ';
        display_line[1][9] = ' '; display_line[1][10] = '\0';

        display_line[2][0] = 'R'; display_line[2][1] = ':'; display_line[2][2] = ' ';
        display_line[2][3] = '0' + (rs / 1000) % 10;
        display_line[2][4] = '0' + (rs / 100)  % 10;
        display_line[2][5] = '0' + (rs / 10)   % 10;
        display_line[2][6] = '0' + (rs)        % 10;
        display_line[2][7] = ' '; display_line[2][8] = ' ';
        display_line[2][9] = ' '; display_line[2][10] = '\0';

        display_changed = TRUE;

        // SW1: capture white calibration, advance to black step
        if (cal_step == 0 && sw1_pressed) {
            sw1_pressed = 0;
            left_white  = ls;
            right_white = rs;
            cal_step    = 1;

        // SW2 on black step: capture black, calibration complete -> phase 1
        } else if (cal_step == 1 && sw2_pressed) {
            sw2_pressed = 0;
            left_black  = left_detect_adc_value  >> 2;
            right_black = right_detect_adc_value >> 2;
            cal_step    = 0;   // reset for next demo run

            // Show post-cal waiting screen
            strcpy(display_line[0], "Waiting   ");
            strcpy(display_line[1], "for input ");
            strncpy(display_line[2], iot_ip_line3, 10); display_line[2][10] = '\0';
            strncpy(display_line[3], iot_ip_line4, 10); display_line[3][10] = '\0';
            display_changed = TRUE;
            update_display  = TRUE;

            // Initialize arrived display to pad 0 - updates to 01-08 via ^A command
            Demo_Set_Arrived(0);

            // Arm the blackline state machine: it will sit idle until
            // a ^B command is received (which sets Statemachine_Run = 1)
            Statemachine_Run   = 0;
            Statemachine_State = 0;

            demo_phase = 1;   // advance to WiFi course display

        // SW2 on white step: exit back to main menu
        } else if (cal_step == 0 && sw2_pressed) {
            P2OUT &= ~IR_LED;
            sw2_pressed = 0;
            cal_step    = 0;
            current_menu = MENU_MAIN;
        }
        break;
    }

    // ------------------------------------------------------------------
    // Phase 1: WiFi course operation
    // ------------------------------------------------------------------
    case 1: {
        // If the blackline SM just started (^B received), jump to phase 2
        // immediately so we stop overwriting display_line[0].
        if (Statemachine_Run) {
            demo_phase = 2;
            break;
        }

        // Line 1: arrived pad (e.g. "Arrived 00") - only written when SM is idle
        strncpy(display_line[0], demo_arrived, 10); display_line[0][10] = '\0';

        // Lines 2-3: IP address
        strncpy(display_line[1], iot_ip_line3, 10); display_line[1][10] = '\0';
        strncpy(display_line[2], iot_ip_line4, 10); display_line[2][10] = '\0';

        // Line 4: last cmd (6 chars: "D TTTT") + space + timer (3 digits)
        strncpy(display_line[3], demo_last_cmd, 6);
        display_line[3][6] = ' ';
        display_line[3][7] = '0' + (s / 100) % 10;
        display_line[3][8] = '0' + (s / 10)  % 10;
        display_line[3][9] = '0' + (s)       % 10;
        display_line[3][10] = '\0';

        display_changed = TRUE;

        if (sw2_pressed) { sw2_pressed = 0; current_menu = MENU_MAIN; }
        break;
    }

    // ------------------------------------------------------------------
    // Phase 2: Autonomous black line
    // Line 1 is owned by Run_Blackline - do NOT overwrite it here.
    // Lines 2-3 show IP. Line 4 shows live timer.
    // ------------------------------------------------------------------
    case 2: {
        // Lines 2-3: IP address (same as phase 1)
        strncpy(display_line[1], iot_ip_line3, 10); display_line[1][10] = '\0';
        strncpy(display_line[2], iot_ip_line4, 10); display_line[2][10] = '\0';

        // Line 4: timer only (SM owns line 0, last cmd is less relevant now)
        display_line[3][0] = 'T';
        display_line[3][1] = 'i';
        display_line[3][2] = 'm';
        display_line[3][3] = 'e';
        display_line[3][4] = ':';
        display_line[3][5] = ' ';
        display_line[3][6] = '0' + (s / 100) % 10;
        display_line[3][7] = '0' + (s / 10)  % 10;
        display_line[3][8] = '0' + (s)       % 10;
        display_line[3][9] = 's';
        display_line[3][10] = '\0';

        display_changed = TRUE;

        // Return to phase 1 layout once autonomous run ends
        if (!Statemachine_Run && Statemachine_State == 0) { demo_phase = 1;}
        break;
    }

    default:
        demo_phase = 0;
        break;
    }
}

// ==============================================================================
//  Menu_Process  -  called every main loop iteration
// ==============================================================================
void Menu_Process(void) {
    static unsigned int  prev_menu     = 99;
    static unsigned char iot_was_ready = 0;

    // When IOT connects for the first time, auto-switch to IOT status menu
    if (iot_display_ready && !iot_was_ready) {
        iot_was_ready = 1;
        if (menu_en && current_menu == MENU_MAIN) {
            current_menu = MENU_IOT;
        }
    }

    if (!menu_en) return;

    if (current_menu != prev_menu) {
        lcd_4line();
        Clear_Display();
        iot_cmd_display_until = 0;
        // Reset internal demo phase and timer whenever re-entering the demo menu
        if (current_menu == MENU_DEMO) {
            Demo_Reset_Phase();
            demo_started = 0;
            demo_seconds = 0;
            Demo_Clear_Arrived();
        }
        prev_menu = current_menu;
    }

    switch (current_menu) {
        case MENU_MAIN:           Menu_Main();           break;
        case MENU_ADC_MONITOR:    Menu_ADC_Monitor();    break;
        case MENU_SERIAL_MONITOR: Menu_Serial_Monitor(); break;
        case MENU_IOT:            Menu_IOT();            break;
        case MENU_DEMO:           Menu_Demo();           break;
        default: current_menu = MENU_MAIN;               break;
    }

    // Keep demo seconds counter ticking every loop iteration
    Demo_Update_Seconds();
}
