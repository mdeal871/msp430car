// ------------------------------------------------------------------------------
//  File Name : macros.h
//
//  Description: This file contains the common #defines used throughout the project
//
//  Mason Deal
//  Feb 2026
//  Built with Code Composer Studio version 12.8.1.00005
//
// ------------------------------------------------------------------------------

#ifndef MACROS_H_
#define MACROS_H_

#define ALWAYS                  (1)
#define RESET_STATE             (0)
#define RED_LED              (0x01) // RED LED 0
#define GRN_LED              (0x40) // GREEN LED 1
#define TEST_PROBE           (0x01) // 0 TEST PROBE
#define TRUE                 (0x01)
#define FALSE                (0x00)

// ADC Detection
#define BRAKE_PULSE_TIME    (5)     // 20ms reverse brake pulse (TB0 ticks)
#define SPIN_PULSE_TIME     (6)
#define STOP_TIME           (400)   // 4 seconds stop (TB0 ticks)
#define STARTUP_DELAY       (100)   // 1 second startup delay (TB0 ticks)

#define DETECT_THRESHOLD    (480)   // White ~225, Black ~735
#define LINE_DETECTED       (1)
#define LINE_NOT_DETECTED   (0)

// Black line following - PD controller
#define STEERING_GAIN       (15)    // "P" term: how hard it steers
#define DERIVATIVE_GAIN     (200)   // "D" term: how much it fights overshoot
#define FOLLOW_DURATION     (400)  // Line following duration (TB0 ticks)

// Black line intercept approach speed (used in state 5 before line detected)
// Range 0-50000. FAST=50000, PERCENT_80=45000, SLOW=35000.
// Slower approach gives sensors more reaction time and reduces overshoot.
// Tune this value: start around 20000 and increase until reliable.
#define BL_APPROACH_SPEED   (20000) // Slow creep toward the line - tune on floor

// Final center sequence
#define CENTER_SPIN_TIME    (20)    // Spin to face center (TB0 ticks) - tune on floor
#define CENTER_DRIVE_TIME   (150)   // Drive to center (TB0 ticks) - tune on floor

// Drive-up sequence: pad 8 -> black line board
//   Step 1: drive forward over pad 8
//   Step 2: 90-degree turn toward chosen board (right or left)
//   Step 3: drive forward to align with track entry point
//   Step 4: 90-degree turn to face the track line
//   All times in TB0 ticks (1 tick = 10 ms) - tune on floor
#define DRIVEUP_FWD1_TIME   (220)    // Forward burst over pad 8
#define DRIVEUP_TURN_TIME   (28)    // 90-degree spin (same duration for L and R)
#define DRIVEUP_FWD2_TIME   (300)    // Forward burst to align with board entry
#define DRIVEUP_TURN2_TIME  (28)    // Second 90-degree spin to face the line

// Black line exit: spin 180 away from pad 8, drive 2+ feet, stop
#define EXIT_SPIN_TIME      (56)    // 180-degree spin away from circle (TB0 ticks)
#define EXIT_DRIVE_TIME     (500)   // Drive 2+ feet clear of circle (TB0 ticks)

// Black line event pause between states (spec: 10-20 seconds, TB0 ticks)
#define BL_EVENT_PAUSE      (1000)   // (Spec 10 secs (1000))

// IOT periodic network ping interval (TB0 ticks, 1 tick = 10 ms)
#define IOT_PING_INTERVAL   (1000)  // Ping www.google.com every 10 seconds

// IOT auto-reconnect: delay before retrying WiFi after disconnect (TB0 ticks)
#define IOT_RECONNECT_DELAY (500)   // 5 seconds before reconnect attempt

// Emergency stop command letter (sent as ^E via TCP or serial)
#define IOT_ESTOP_CMD       ('E')

// Demo day menu state definition (extends globals.h MENU_* defines)
#define MENU_DEMO           (4)     // Single demo day menu; phase driven internally

// PWM
#define PWM_PERIOD (TB3CCR0)

#define RIGHT_FORWARD_SPEED  (TB3CCR2)   // P6.1 = TB3.2 = R_FORWARD
#define LEFT_FORWARD_SPEED   (TB3CCR3)   // P6.2 = TB3.3 = L_FORWARD
#define RIGHT_REVERSE_SPEED  (TB3CCR4)   // P6.3 = TB3.4 = R_REVERSE
#define LEFT_REVERSE_SPEED   (TB3CCR5)   // P6.4 = TB3.5 = L_REVERSE
#define LCD_BACKLITE_DIMING  (TB3CCR1)   // P6.0 = TB3.1 = LCD_BACKLITE

#define WHEEL_PERIOD (50005)
#define WHEEL_OFF (0)
#define SLOW (35000)
#define FAST (50000)
#define PERCENT_100 (50000)
#define PERCENT_80 (45000)

// Motor off-time: TB3 CCR0 ticks between direction changes
#define MOTOR_OFF_TIME    (2)   // 12.5 ms delay
#define TIMER_B3_ONESEC   (160) // 1 s
#define TIMER_B3_TWOSEC   (320) // 2 s
#define TIMER_B3_THREESEC (480) // 3 s

#define TIMER_B0_ONESEC   (100) // 1 s
#define TIMER_B0_TWOSEC   (200) // 2 s
#define TIMER_B0_THREESEC (300) // 3 s
#define TIMER_B0_FIVESEC  (500) // 5 s

// Switch debounce
#define PRESSED (1)             // switch has been pressed

// Backlight timeout: ticks of the 200ms display counter before backlight turns off
// 50 x 200ms = 10 seconds
#define BACKLIGHT_TIMEOUT (50)

// Timer B0 intervals
#define TB0CCR0_INTERVAL 1250   // 10 ms system tick
#define TB0CCR1_INTERVAL 6400   // 1000 ms debounce
#define TB0CCR2_INTERVAL 6400   // 1000 ms debounce

#define TIMER_B0_CCR0_VECTOR      TIMER0_B0_VECTOR
#define TIMER_B0_CCR1_2_OV_VECTOR TIMER0_B1_VECTOR
#define TIMER_B1_CCR0_VECTOR      TIMER1_B0_VECTOR
#define TIMER_B1_CCR1_2_OV_VECTOR TIMER1_B1_VECTOR
#define TIMER_B2_CCR0_VECTOR      TIMER2_B0_VECTOR
#define TIMER_B2_CCR1_2_OV_VECTOR TIMER2_B1_VECTOR
#define TIMER_B3_CCR0_VECTOR      TIMER3_B0_VECTOR
#define TIMER_B3_CCR1_2_OV_VECTOR TIMER3_B1_VECTOR

#endif /* MACROS_H_ */
