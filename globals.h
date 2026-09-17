// ------------------------------------------------------------------------------
//  File Name : globals.h
//
//  Description: This file contains the global variables used and makes them external
//  so they are usable with the following files: main.c, display.c, init.c, and led.c
//
//  Mason Deal
//  Mar 2026
//  Built with Code Composer Studio version 12.8.1.00005
//
// ------------------------------------------------------------------------------

#ifndef GLOBALS_H_
#define GLOBALS_H_

// Menu state definitions
#define MENU_MAIN           (0)
#define MENU_ADC_MONITOR    (1)
#define MENU_SERIAL_MONITOR (2)
#define MENU_IOT            (3)

// Backlite
extern unsigned char bl_status;

// Global Variables
extern volatile char slow_input_down;
extern char display_line[4][11];
extern char *display[4];
extern unsigned char display_mode;
extern volatile unsigned char display_changed;
extern volatile unsigned char update_display;
extern volatile unsigned int update_display_count;
extern volatile char one_time;
extern unsigned int test_value;
extern char chosen_direction;
extern char change;

// Serial global flags
extern volatile unsigned char transmit_pending;
extern volatile unsigned int  transmit_countdown;

// Interrupt flags
extern volatile unsigned char IFG_B0;
extern volatile unsigned int Timer_B0_Target;

// State machine flag
extern volatile unsigned char Statemachine_Run;
extern volatile unsigned char Statemachine_State;
extern volatile unsigned int Runtime_FLG;

extern unsigned int wheel_move;
extern char forward;

// Menu system
extern unsigned char current_menu;
unsigned char menu_en;

// Calibration values
extern volatile unsigned int left_white;
extern volatile unsigned int right_white;
extern volatile unsigned int left_black;
extern volatile unsigned int right_black;

// ADC
extern volatile unsigned int thumb_adc_value;
extern volatile unsigned int left_detect_adc_value;
extern volatile unsigned int right_detect_adc_value;
extern volatile unsigned char adc_channel;

extern volatile unsigned char left_line_detected;
extern volatile unsigned char right_line_detected;

extern unsigned char thousands;
extern unsigned char hundreds;
extern unsigned char tens;
extern unsigned char ones;

// Timers
extern volatile unsigned int motor_delay_counter;
extern volatile unsigned int Time_Sequence;
extern volatile unsigned int BL_Sequence;
extern volatile unsigned int switch_deounce;
extern volatile unsigned int Runtime_Count;
extern volatile unsigned int backlight_timeout_count;  // 200ms ticks since last display update

// Switch debounce

extern volatile unsigned int sw1_debounce_count;
extern volatile unsigned int sw2_debounce_count;
extern volatile unsigned char sw1_pressed;
extern volatile unsigned char sw2_pressed;


// IOT display state (populated by IOT_Parse_Response)
extern char          iot_ssid[11];       // centered SSID string
extern char          iot_ip_label[11];   // "IP Address"
extern char          iot_ip_line3[11];   // first two IP groups
extern char          iot_ip_line4[11];   // last  two IP groups
extern unsigned char iot_display_ready;  // 1 once SSID+IP have been received

// IOT motion command (set by IOT_Command_Dispatch, consumed by Run_IOT_Statemachine)
extern char          iot_motion_dir;        // slot 1 direction 'F' 'B' 'L' 'R'
extern unsigned int  iot_motion_duration;   // slot 1 duration ms
extern unsigned char iot_motion_pending;    // 1 = slot 1 ready
extern char          iot_motion_dir2;       // slot 2 direction (chained command)
extern unsigned int  iot_motion_duration2;  // slot 2 duration ms
extern unsigned char iot_motion_pending2;   // 1 = slot 2 ready
extern volatile unsigned long  iot_cmd_display_until;
extern volatile unsigned long iot_motor_cooldown_until;
extern volatile unsigned long iot_elapsed;
extern unsigned char bl_pending;

// IOT periodic ping
extern volatile unsigned long  iot_next_ping_time;  // iot_elapsed value for next ping
extern volatile unsigned char  iot_ping_pending;    // 1 = ping result being awaited

// IOT auto-reconnect
extern volatile unsigned char  iot_connected;       // 1 = WiFi connected
extern volatile unsigned long  iot_reconnect_at;    // iot_elapsed value to retry connect

// Emergency stop flag (set by ^E command, clears all motion immediately)
extern volatile unsigned char  iot_estop;

// Demo day globals
extern volatile unsigned long  demo_start_time;
extern volatile unsigned char  demo_started;
extern volatile unsigned int   demo_seconds;
extern char                    demo_arrived[11];
extern char                    demo_last_cmd[11];
extern char                    demo_name_line3[11];
extern char                    demo_name_line4[11];

// Drive-up positioning state machine (defined in statemachines.c)
extern unsigned char Driveup_Run;    // 1 = running, 0 = idle
extern unsigned char Driveup_State;  // current state (1-5)
extern unsigned char driveup_side;   // 'R' = right board, 'L' = left board

#endif /* GLOBALS_H_ */
