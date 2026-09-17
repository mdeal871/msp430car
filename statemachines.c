// ------------------------------------------------------------------------------
//  File Name: statemachines.c
//
//  Description: All state machines.
//    Run_Blackline()      - line following / navigation (Project 6)
//    Run_IOT_Wheels()  - executes motion commands received over TCP
//
//  IOT Motion State Machine
//  -------------------------
//  Triggered by IOT_Command_Dispatch() setting iot_motion_pending = 1.
//  iot_motion_dir      : 'F' forward, 'B' reverse, 'L' left, 'R' right
//  iot_motion_duration : milliseconds (converted to TB0 ticks: 1 tick = 10 ms)
//
//  Supported commands (examples):
//    ^1234F0200  -> forward  200 ms
//    ^1234B0100  -> reverse  100 ms  (1 sec = 1000 ms -> use 1000)
//    ^1234R0090  -> right turn ~90 deg
//    ^1234L0045  -> left  turn ~45 deg
//
//  Two commands can be chained on one +IPD line using ';' as separator
//  (handled in IOT_Command_Dispatch, both set iot_motion_pending in sequence).
//
//  Mason Deal
//  Apr 2026
//  Built with Code Composer Studio version 12.8.1.00005
// ------------------------------------------------------------------------------
#include  "msp430.h"
#include  <string.h>
#include  "functions.h"
#include  "LCD.h"
#include  "ports.h"
#include  "macros.h"
#include  "globals.h"

static long int last_derivative = 0;

static unsigned char  iot_state    = 0;
static unsigned int   iot_ticks    = 0;  // duration in TB0 ticks (10 ms each)
static unsigned char  iot_dir_copy = 'F';

void Reset_IOT_Wheels(void) {
    Wheels_Off();
    iot_state = 0;           // Reset the static state variable
    iot_motion_pending = 0;
    iot_motion_pending2 = 0;
}

// ==============================================================================
//  Run_IOT_Wheels
//  Call every main loop iteration. Executes one motion command at a time.
//
//  States:
//    0 - idle, waiting for iot_motion_pending
//    1 - apply motor direction
//    2 - wait for duration to elapse, then stop
// ==============================================================================
void Run_IOT_Wheels(void) {

    switch (iot_state) {

    case 0:  // Idle - wait for a pending command
        if (!iot_motion_pending) break;
        // Reject commands that arrive within the post-execution cooldown window.
        // This discards ESP32 echo packets / Java client retransmits.
        if (iot_elapsed < iot_motor_cooldown_until) {
            iot_motion_pending  = 0;
            iot_motion_pending2 = 0;
            break;
        }
        iot_motion_pending = 0;

        // Convert milliseconds to TB0 ticks (each tick = 10 ms)
        iot_ticks    = iot_motion_duration / 10;
        iot_dir_copy = iot_motion_dir;

        Time_Sequence = 0;
        iot_state = 1;
        break;

    case 1:  // Apply direction
        iot_motion_pending = 0;   // discard any late duplicate (TCP retransmit)
        Wheels_Off();
        switch (iot_dir_copy) {
            case 'F': Forward_On();  break;
            case 'B': Reverse_On();  break;
            case 'R': R_Forward(); L_Reverse();  break;  // spin right
            case 'L': L_Forward(); R_Reverse();  break;  // spin left
            default:  break;
        }
        Time_Sequence = 0;
        iot_state = 2;
        break;

    case 2:  // Wait for duration
        iot_motion_pending = 0;   // discard duplicates while executing
        if (Time_Sequence >= iot_ticks) {
            Wheels_Off();
            Time_Sequence = 0;
            // If a second chained command is queued, load it into slot 1 now
            if (iot_motion_pending2) {
                iot_motion_dir      = iot_motion_dir2;
                iot_motion_duration = iot_motion_duration2;
                iot_motion_pending  = 1;
                iot_motion_pending2 = 0;
            } else if (bl_pending) {
                Backlite_On();
                Reset_Driveup();            // stop positioning SM if still running
                Reset_IOT_Wheels();
                if (!demo_started) {
                    demo_started    = 1;
                    demo_start_time = iot_elapsed;
                }
                Demo_Set_Phase(2);
                Time_Sequence      = 0;
                BL_Sequence        = 0;
                Statemachine_State = 'a';     // skip drive-up, go straight to intercept
                Statemachine_Run   = 1;
                return;
            }
            // Set cooldown to block stale +IPD echoes / Java client retransmits.
            // IMPORTANT: check iot_motion_pending (not pending2) - if slot 2 was just
            // loaded into slot 1, pending=1 means a chained command is ready and must
            // NOT be blocked by the cooldown.
            if (!iot_motion_pending) {
                iot_motor_cooldown_until = iot_elapsed + 10; // 100ms cooldown
            }
            iot_state = 0;
        }
        break;

    default:
        iot_state = 0;
        break;
    }
}

// ==============================================================================
//  Run_Blackline  (intercept + follow + exit only - no drive-up logic)
//
//  Entry point: Statemachine_State = 5  (set by ^B command in IOT_Command_Dispatch)
//  The car is already positioned facing the track by Run_Driveup (^BR or ^BL).
//  ^B drives forward until the line is detected, then runs the full
//  intercept / turn / follow / circle / exit sequence.
// ==============================================================================
void Run_Blackline(void) {
    unsigned int final_time;

    if (!Statemachine_Run) return;

    switch (Statemachine_State) {

    case 1: // Calibrate White

        strcpy(display_line[0], "CAL WHITE ");
        strcpy(display_line[3], "PRESS SW1 ");
        update_display = 1;
        if (sw1_pressed) {
            sw1_pressed = 0;
            left_white  = left_detect_adc_value  >> 2;
            right_white = right_detect_adc_value >> 2;
            Statemachine_State = 2;
        }
        break;

    case 2: // Calibrate Black
        strcpy(display_line[0], "CAL BLACK ");
        strcpy(display_line[3], "PRESS SW2 ");
        update_display = 1;
        if (sw2_pressed) {
            sw2_pressed = 0;
            left_black  = left_detect_adc_value  >> 2;
            right_black = right_detect_adc_value >> 2;
            current_menu = MENU_DEMO;
            strcpy(display_line[0], "Waiting   ");
            strcpy(display_line[1], "for input ");
            strcpy(display_line[2], iot_ip_line3);
            strcpy(display_line[3], iot_ip_line4);
            update_display = 1;
            Statemachine_Run   = 0;
            Statemachine_State = 0;
        }
        break;

    case 'a': // Pause with "BL Start" display (15s per spec)
            strcpy(display_line[0], " BL Start ");

            P2OUT |= IR_LED;            // Ensure IR led on

            demo_seconds = 0;
            if (BL_Sequence >= BL_EVENT_PAUSE) {
                BL_Sequence = 0;
                Statemachine_State = 5;
            }
            break;

    case 5: // Slow creep forward until black line detected
        strcpy(display_line[0], " BL Start ");
        Forward_On();
//        RIGHT_FORWARD_SPEED = BL_APPROACH_SPEED;
//        LEFT_FORWARD_SPEED  = BL_APPROACH_SPEED;
        update_display = 1;
        if (left_line_detected || right_line_detected) {
            strcpy(display_line[0], " Intercept");
            Wheels_Off();
            BL_Sequence = 0;
            Statemachine_State = 6;
        }
        break;

    case 6: // Brief reverse brake pulse
        Reverse_On();
        if (BL_Sequence >= BRAKE_PULSE_TIME) {
            Wheels_Off();
            BL_Sequence = 0;
            Statemachine_State = 'b';
        }

        break;

    case 'b': // Pause with "Intercept" display (10-20s per spec)
        if (BL_Sequence >= BL_EVENT_PAUSE) {
            BL_Sequence = 0;
            Statemachine_State = 'c';
        }
        break;

    case 'c': // Pause with "BL Turn" display (15s per spec)
               strcpy(display_line[0], " BL Turn ");
               if (BL_Sequence >= BL_EVENT_PAUSE) {
                   BL_Sequence = 0;
                   Statemachine_State = 7;
               }
               break;

    case 7: // Stop and wait before spin
        Wheels_Off();
        strcpy(display_line[0], " BL Turn  ");
        update_display = 1;
        if (BL_Sequence >= STOP_TIME) {
            BL_Sequence = 0;
            Statemachine_State = 8;
        }
        break;

    case 8: // Spin until both sensors on the line
        if (driveup_side == 'R') {
        L_Forward(); R_Reverse();
        } else {
        R_Forward(); L_Reverse();
        }
        strcpy(display_line[3], " BL Turn  ");
        update_display = 1;
        {
            long int ls = left_detect_adc_value  >> 2;
            long int rs = right_detect_adc_value >> 2;
            long int ln = ((ls - left_white)  * 1000) / (left_black  - left_white);
            long int rn = ((rs - right_white) * 1000) / (right_black - right_white);
            if (ln > 600 && rn > 600) {
                Wheels_Off(); BL_Sequence = 0; Statemachine_State = 9;
            } else if (BL_Sequence >= TIMER_B0_THREESEC) {
                Wheels_Off(); Statemachine_Run = 0; Statemachine_State = 0;
            }
        }

        break;

    case 9: // Counter-spin to kill momentum
        if (driveup_side == 'R') {
            R_Forward(); L_Reverse();
        } else {
            R_Reverse(); L_Forward();
        }

        if (BL_Sequence >= SPIN_PULSE_TIME) {
            Wheels_Off();
            LEFT_REVERSE_SPEED = RIGHT_REVERSE_SPEED = WHEEL_OFF;
            motor_delay_counter = 0;
            while (motor_delay_counter < MOTOR_OFF_TIME);
            BL_Sequence = 0;
            Statemachine_State = 'd';
        }

        break;

    case 'd': // Pause with "BL Travel" display (15s per spec)
        Wheels_Off();
        strcpy(display_line[0], " BL Travel");
        update_display = 1;
        if (BL_Sequence >= BL_EVENT_PAUSE) {
            BL_Sequence = 0;
            Forward_On();
            Statemachine_State = 10;
        }
        break;

    case 10: // Follow the line
        Line_Follow();
        if (BL_Sequence >= FOLLOW_DURATION) {
            Wheels_Off(); BL_Sequence = 0; Statemachine_State = 'e';
        }
        break;

    case 'e': // Pause with "BL Circle"  shown (10-20s per spec)
        Wheels_Off();
        strcpy(display_line[0], " BL Circle");
        update_display = 1;
        if (BL_Sequence >= BL_EVENT_PAUSE) {
            BL_Sequence = 0;
            Statemachine_State = 'f';
        }
        break;

    case 'f': // Resume line follow until exit command
        Line_Follow();
        break;

    case 'g': // Pause with "BL Exit" display (15s per spec)
               strcpy(display_line[0], " BL Exit ");
               if (BL_Sequence >= BL_EVENT_PAUSE) {
                   BL_Sequence = 0;
                   Statemachine_State = 11;
               }
               break;

    case 11: // Exit: drive 2+ feet away from circle
        strcpy(display_line[0], " BL Exit  ");
        if (BL_Sequence == 0) {
            Forward_On();
        }
        if (BL_Sequence >= EXIT_DRIVE_TIME) {
            Wheels_Off();
            BL_Sequence = 0;
            Statemachine_State = 12;
            final_time = demo_seconds;
        }
        break;

    case 12: // Final stop - display BL Stop + custom message + time
        Wheels_Off();
        P2OUT &= ~IR_LED;                               // Turn off IR LED - no longer needed
        strcpy(display_line[0], " BL Stop  ");
        strcpy(display_line[1], " Demo Day ");
        strcpy(display_line[2], "done!! ;-)");
        // Display total seconds elapsed (uses demo_seconds updated in main loop)
        display_line[3][0] = 'T';
        display_line[3][1] = 'i';
        display_line[3][2] = 'm';
        display_line[3][3] = 'e';
        display_line[3][4] = ':';
        display_line[3][5] = ' ';
        display_line[3][6] = '0' + (final_time / 100) % 10;
        display_line[3][7] = '0' + (final_time / 10)  % 10;
        display_line[3][8] = '0' + (final_time)       % 10;
        display_line[3][9] = 's';
        display_line[3][10] = '\0';
        update_display = 1;

        if (BL_Sequence >= BL_EVENT_PAUSE) {
        Statemachine_Run = 0;
        Statemachine_State = 0;
        demo_seconds = 0;
        }
        break;

    default: break;
    }
}

// ==============================================================================
//  Blackline_Exit_Command
//  Called by IOT_Command_Dispatch when the TA-authorized exit command is received
//  (e.g. ^0630X). Jumps directly to the exit spin state from wherever the car
//  currently is in the circle, so the TA can trigger it at any point.
//  Per spec: car must exit in a direction away from pad 8 and stop 2+ feet away.
// ==============================================================================
void Blackline_Exit_Command(void) {
    Wheels_Off();
    Time_Sequence      = 0;
    BL_Sequence        = 0;
    Statemachine_State = 'g';  // jump directly to exit spin state
    Statemachine_Run   = 1;
}

// ==============================================================================
//  Blackline_Emergency_Exit
//  Immediately stops the black line state machine and all motors.
//  Triggered by ^E while in autonomous mode.
// ==============================================================================
void Blackline_Emergency_Exit(void) {
    Wheels_Off();
    P2OUT &= ~IR_LED;                               // Turn off IR LED
    Statemachine_Run   = 0;
    Statemachine_State = 0;
    strcpy(display_line[0], " BL STOP  ");
    strcpy(display_line[1], " E-STOP   ");
    strcpy(display_line[2], "          ");
    strcpy(display_line[3], "          ");
    update_display = 1;
}
// ==============================================================================
//  Run_Driveup  -  pad 8 positioning state machine
//
//  Triggered by ^BR (right board) or ^BL (left board) from IOT_Command_Dispatch.
//  Drives the car from a stopped position just before pad 8:
//    State 1: forward over pad 8
//    State 2: 90-degree turn toward chosen board (right: L_Fwd/R_Rev, left: R_Fwd/L_Rev)
//    State 3: forward to align with track entry point
//    State 4: 90-degree turn to face the track line (same direction as state 2)
//    State 5: stopped - display "Lined Up" awaiting ^B command
//
//  driveup_side: 'R' = right board, 'L' = left board (set by IOT_Command_Dispatch)
//  Driveup_Run:  1 = running, 0 = idle
// ==============================================================================
unsigned char Driveup_Run  = 0;
unsigned char Driveup_State = 0;
unsigned char driveup_side  = 'R';  // 'R' or 'L', set before Driveup_Run = 1

void Run_Driveup(void) {
    if (!Driveup_Run) return;

    switch (Driveup_State) {

    case 1: // Drive forward over pad 8
        strcpy(display_line[0], " Setup BL ");
        Forward_On();
        if (Time_Sequence >= DRIVEUP_FWD1_TIME) {
            Wheels_Off();
            Time_Sequence = 0;
            Driveup_State = 2;
        }
        break;

    case 2: // 90-degree turn toward the chosen board
        strcpy(display_line[0], " Turning  ");
        if (driveup_side == 'L') {
            // Turn right: spin right motor back, left motor forward
            R_Reverse(); L_Forward();
        } else {
            // Turn left: spin left motor back, right motor forward
            L_Reverse(); R_Forward();
        }
        if (Time_Sequence >= DRIVEUP_TURN_TIME) {
            Wheels_Off();
            Time_Sequence = 0;
            Driveup_State = 3;
        }
        break;

    case 3: // Drive forward to align with track board entry point
        strcpy(display_line[0], " Aligning ");
        Forward_On();
        if (Time_Sequence >= DRIVEUP_FWD2_TIME) {
            Wheels_Off();
            Time_Sequence = 0;
            Driveup_State = 4;
        }
        break;

    case 4: // Second 90-degree turn to face the track line
        strcpy(display_line[0], " Facing   ");
        if (driveup_side == 'L') {
            R_Reverse(); L_Forward();
        } else {
            L_Reverse(); R_Forward();
        }
        if (Time_Sequence >= DRIVEUP_TURN2_TIME) {
            Wheels_Off();
            Time_Sequence = 0;
            Driveup_State = 5;
        }
        break;

    case 5: // Stopped, facing the track - await ^B command
        // Only stop wheels on the first pass (Time_Sequence just reset to 0).
        // After that, yield to Run_IOT_Wheels so adjustment commands work.
        if (Time_Sequence == 0) {
            Wheels_Off();
            strcpy(display_line[3], "Send ^B   ");
            update_display = 1;
        }
        Backlite_On();\
        strcpy(display_line[0], "Arrived 08");
        update_display = 1;
        // Shut down once ^B fires (Statemachine_Run goes high)
        if (Statemachine_Run) {
            Driveup_Run   = 0;
            Driveup_State = 0;
        }
        break;

    default:
        Driveup_Run   = 0;
        Driveup_State = 0;
        break;
    }
}

// ==============================================================================
//  Reset_Driveup  -  stop the driveup state machine and clear all state
// ==============================================================================
void Reset_Driveup(void) {
    Wheels_Off();
    Driveup_Run   = 0;
    Driveup_State = 0;
}

// Line Follow
void Line_Follow(void) {
    long int error;
    long int derivative;
    long int correction;
    long int left_speed;
    long int right_speed;
    long int base_speed = 9000;

    static long int last_valid_error = 0;
    static long int previous_error = 0;

    long int left_scaled  = left_detect_adc_value  >> 2;
    long int right_scaled = right_detect_adc_value >> 2;

    // Formula: ((Current - White) * 1000) / (Black - White)
    long int left_norm = ((left_scaled - left_white) * 1000) / (left_black - left_white);
    long int right_norm = ((right_scaled - right_white) * 1000) / (right_black - right_white);

    // Cap values to prevent weird math if ambient light shifts slightly
    if (left_norm < 0) left_norm = 0;
    if (left_norm > 1000) left_norm = 1000;
    if (right_norm < 0) right_norm = 0;
    if (right_norm > 1000) right_norm = 1000;

    // Calculate error using the normalized values
    error = left_norm - right_norm;

    // Guardrail recovery logic using normalized values (threshold is now ~500)
    if ((left_norm < 300) && (right_norm < 300)) {
        error = (last_valid_error > 0) ? 700 : -700;
    } else {
        last_valid_error = error;
    }

    long int current_derivative = error - previous_error;

    // Average the current rate of change with the previous rate of change
    derivative = (current_derivative + last_derivative) / 2;

    // Update memories for the next cycle
    last_derivative = current_derivative;
    previous_error = error;

    correction = (error * STEERING_GAIN) + (derivative * DERIVATIVE_GAIN);

    // Speed application
    if (correction > 0) {
        left_speed = base_speed - correction;
        right_speed = base_speed;
    } else {
        left_speed = base_speed;
        right_speed = base_speed + correction;
    }

    // Safety clamps
    if (left_speed < 0) left_speed = 0;
    if (right_speed < 0) right_speed = 0;

    // Prevent speeds from exceeding motor limits (30,000-50,000)
    if (left_speed > 30000)  left_speed = 30000;
    if (right_speed > 30000) right_speed = 30000;
    // ---------------------------

    LEFT_FORWARD_SPEED = (unsigned int)left_speed;
    RIGHT_FORWARD_SPEED = (unsigned int)right_speed;
}
