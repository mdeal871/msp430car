// ------------------------------------------------------------------------------
//  File Name : globals.c
//
//  Description: This file contains the global variables used for the following files:
//  main.c, display.c, init.c, and led.c
//
//  Mason Deal
//  Mar  2026
//  Built with Code Composer Studio version 12.8.1.00005
//
// ------------------------------------------------------------------------------

#include "globals.h"
#include "macros.h"

// Global Variables
volatile char slow_input_down;
extern char display_line[4][11];
extern char *display[4];
unsigned char display_mode;
extern volatile unsigned char display_changed;
extern volatile unsigned char update_display;
extern volatile unsigned int update_display_count;
extern volatile char one_time;
unsigned int test_value;
char chosen_direction;
char change;

// Backlite
unsigned char bl_status;

// Serial transmit global flags
volatile unsigned char transmit_pending;
volatile unsigned int  transmit_countdown;

// Calibration values
volatile unsigned int left_white = 225;  // Default safe values
volatile unsigned int right_white = 225;
volatile unsigned int left_black = 735;
volatile unsigned int right_black = 735;

// Timers
volatile unsigned int motor_delay_counter;
volatile unsigned int Time_Sequence;
volatile unsigned int BL_Sequence;
volatile unsigned int Runtime_Count;
volatile unsigned int Runtime_FLG;
volatile unsigned int backlight_timeout_count;  // counts 200ms display ticks since last update

// Switch debounce
volatile unsigned int sw1_debounce_count;
volatile unsigned int sw2_debounce_count;
volatile unsigned char sw1_pressed;
volatile unsigned char sw2_pressed;

// ADC
volatile unsigned int thumb_adc_value;
volatile unsigned int left_detect_adc_value;
volatile unsigned int right_detect_adc_value;
volatile unsigned char adc_channel;                 // 0=thumb, 1=left, 2=right

volatile unsigned char left_line_detected;
volatile unsigned char right_line_detected;

unsigned char thousands;
unsigned char hundreds;
unsigned char tens;
unsigned char ones;

// Project 5
volatile unsigned char Statemachine_Run = 0;
volatile unsigned char Statemachine_State = 0;

// Interrupt flags
volatile unsigned char IFG_B0;
volatile unsigned int Timer_B0_Target;

unsigned int wheel_move;
char forward;

// Menu System
unsigned char current_menu = MENU_MAIN;
unsigned char menu_en;

// IOT display state
char          iot_ssid[11]      = "          ";
char          iot_ip_label[11]  = "IP Address";
char          iot_ip_line3[11]  = "          ";
char          iot_ip_line4[11]  = "          ";
unsigned char iot_display_ready = 0;

// IOT motion command
char          iot_motion_dir      = 'F';
unsigned int  iot_motion_duration = 0;
unsigned char iot_motion_pending  = 0;
char          iot_motion_dir2     = 'F';
unsigned int  iot_motion_duration2 = 0;
unsigned char iot_motion_pending2  = 0;
unsigned char bl_pending           = FALSE;

// IOT Timers
volatile unsigned long  iot_motor_cooldown_until;
volatile unsigned long  iot_cmd_display_until = 0;
volatile unsigned long  iot_elapsed           = 0;

// IOT periodic ping
volatile unsigned long  iot_next_ping_time    = 0;  // iot_elapsed value for next ping
volatile unsigned char  iot_ping_pending      = 0;  // 1 = ping result being awaited

// IOT auto-reconnect
volatile unsigned char  iot_connected         = 0;  // 1 = WiFi connected
volatile unsigned long  iot_reconnect_at      = 0;  // iot_elapsed value to retry connect

// Emergency stop flag (set by ^E command, clears all motion)
volatile unsigned char  iot_estop             = 0;

// Demo day globals
volatile unsigned long  demo_start_time       = 0;
volatile unsigned char  demo_started          = 0;
volatile unsigned int   demo_seconds          = 0;
char                    demo_arrived[11]      = "          ";
char                    demo_last_cmd[11]     = "          ";
char                    demo_name_line3[11]   = " 10.155   ";
char                    demo_name_line4[11]   = " .102.55  ";

// Drive-up positioning state machine globals (used by Run_Driveup / IOT_Command_Dispatch)
// Declared here as extern targets; definitions are in statemachines.c
extern unsigned char Driveup_Run;
extern unsigned char Driveup_State;
extern unsigned char driveup_side;
