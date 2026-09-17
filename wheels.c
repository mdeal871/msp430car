// ------------------------------------------------------------------------------
//  File Name: wheels.c
//
//  Description: This file contains all functions and processes related to the
//  wheels.
//  *IMPORTANT* ANY TIME A MODIFICATION IS MADE TO THIS FILE, AD3 TESTING MUST
//  BE DONE FIRST TO PREVENT ANY DAMAGE TO THE MOTORS
//
//
//  Mason Deal
//  Feb 2026
//  Built with Code Composer Studio version 12.8.1.00005
//
// ------------------------------------------------------------------------------

#include  "functions.h"
#include  "msp430.h"
#include  "macros.h"
#include  "ports.h"
#include  "globals.h"

void Wheels_Off(void);
void Forward_On(void);
void Forward_Off(void);
void R_Forward(void);
void L_Forward(void);
void R_Off(void);
void L_Off(void);
void L_Reverse(void);
void R_Reverse(void);
void Reverse_Off(void);
void Reverse_On(void);

static void Off_Delay(void) {
    motor_delay_counter = 0;
    while (motor_delay_counter < MOTOR_OFF_TIME);   // Wait for CCR0 ticks
}


void Wheels_Off(void) {
    LEFT_FORWARD_SPEED = WHEEL_OFF; // Set Port pin Low [Wheel Off]
    RIGHT_FORWARD_SPEED = WHEEL_OFF; // Set Port pin Low [Wheel Off]
    LEFT_REVERSE_SPEED = WHEEL_OFF; // Set Port pin Low [Wheel Off]
    RIGHT_REVERSE_SPEED = WHEEL_OFF; // Set Port pin Low [Wheel Off]
}

void Forward_On(void) {
    LEFT_REVERSE_SPEED = WHEEL_OFF; // Set Port pin High [Wheel On]
    RIGHT_REVERSE_SPEED = WHEEL_OFF; // Set Port pin High [Wheel On]

    Off_Delay();

    LEFT_FORWARD_SPEED = FAST; // Set Port pin High [Wheel On]
    RIGHT_FORWARD_SPEED = FAST; // Set Port pin High [Wheel On]
}

void Forward_Off(void) {
    LEFT_FORWARD_SPEED = WHEEL_OFF; // Set Port pin Low [Wheel Off]
    RIGHT_FORWARD_SPEED = WHEEL_OFF; // Set Port pin Low [Wheel Off]
}

void R_Forward(void) {
    RIGHT_REVERSE_SPEED = WHEEL_OFF;

    Off_Delay();

    RIGHT_FORWARD_SPEED = FAST; // Set Port pin High [Wheel On]
}

void L_Forward(void) {
    LEFT_REVERSE_SPEED = WHEEL_OFF;

    Off_Delay();

    LEFT_FORWARD_SPEED = FAST; // Set Port pin High [Wheel On]
}

void L_Off(void) {
    LEFT_FORWARD_SPEED = WHEEL_OFF; // Set Port pin Low [Wheel Off]
    LEFT_REVERSE_SPEED = WHEEL_OFF; // Set Port pin Low [Wheel Off]
}

void R_Off(void) {
    RIGHT_FORWARD_SPEED = WHEEL_OFF; // Set Port pin Low [Wheel Off]
    RIGHT_REVERSE_SPEED = WHEEL_OFF; // Set Port pin Low [Wheel Off]
}

void L_Reverse(void){
    LEFT_FORWARD_SPEED = WHEEL_OFF;

    Off_Delay();

   LEFT_REVERSE_SPEED = FAST;

}

void R_Reverse(void){
    RIGHT_FORWARD_SPEED = WHEEL_OFF;

    Off_Delay();

    RIGHT_REVERSE_SPEED = FAST;

}

void Reverse_Off(void){
    LEFT_REVERSE_SPEED = WHEEL_OFF;
    RIGHT_REVERSE_SPEED = WHEEL_OFF;

}
void Reverse_On(void){
    LEFT_FORWARD_SPEED = WHEEL_OFF;
    RIGHT_FORWARD_SPEED = WHEEL_OFF;


    Off_Delay();

   LEFT_REVERSE_SPEED = FAST;
    RIGHT_REVERSE_SPEED = FAST;

}
