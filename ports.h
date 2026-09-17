// ------------------------------------------------------------------------------
//  File Name: port.h
//
//  Description: This file contains all port definitions for use in the
//  ports.c file.
//
//
//  Mason Deal
//  Feb 2026
//  Built with Code Composer Studio version 12.8.1.00005
//
// ------------------------------------------------------------------------------

// ------------------------------------------------------------------------------
//  Port 1 pins
//
//  Description: This function contains all the definitions for port 1.
//
// ------------------------------------------------------------------------------

#define RED_LED             (0x01)					 // 0 RED_LED
#define A1_SEEED            (0x02)					 // 1 A_1SEEED
#define V_DETECT_L          (0x04)					 // 2 VEHICLE_DETECT_LEFT
#define V_DETECT_R          (0x08)					 // 3 VEHICLE_DETECT_RIGHT
#define A4_SEEED            (0x10)					 // 4 A_4SEEED
#define V_THUMB             (0x20)					 // 5 THUMBWHEEL_VOLTAGE
#define UCA0TXD             (0x40)					 // 6 UCA0RXD
#define UCA0RXD             (0x80)					 // 7 UCA0RXD

// ------------------------------------------------------------------------------
//  Port 2 pins
//
//  Description: This function contains all the definitions for port 2.
//
// ------------------------------------------------------------------------------

#define SLOW_CLK            (0x01)					 // 0 SLOW_CLK
#define CHECK_BAT           (0x02)					 // 1 Check ADC Voltages
#define IR_LED              (0x04)					 // 2 IR_LED
#define SW2                 (0x08)					 // 3 SW2
#define IOT_RUN_RED         (0x10)					 // 4 IOT_RUN_CPU
#define DAC_ENB             (0x20)					 // 5 DAC_ENB
#define LFXOUT              (0x40)					 // 6 XOUTR
#define LFXIN               (0x80)					 // 7 XINR

// ------------------------------------------------------------------------------
//  Port 3 pins
//
//  Description: This function contains all the definitions for port 3.
//
// ------------------------------------------------------------------------------

// Port 3 Pins
#define TEST_PROBE			 (0x01)					 // 0 TEST PROBE
#define OA20				 (0x02)					 // 1 OA2O
#define OA2N				 (0x04)					 // 2 OA2N
#define OA2P				 (0x08)					 // 3 OA2P
#define SMCLK_OUT			 (0x10)					 // 4 SMCLK
#define DAC_CTRL_3			 (0x20)					 // 5 DAC signal from Processor
#define IOT_LINK_CPU		 (0x40)					 // 6 IOT_LINK_GRN
#define IOT_RN_CPU			 (0x80)					 // 7 IOT_EN

// ------------------------------------------------------------------------------
//  Port 4 pins
//
//  Description: This function contains all the definitions for port 4.
//
// ------------------------------------------------------------------------------

#define RESET_LCD			 (0x01)					 // 0 RESET_LCD
#define SW1					 (0x02)					 // 1 SW1
#define UCA1RXD				 (0x04)					 // 2 Back Channel UCA1RXD
#define UCA1TXD				 (0x08)					 // 3 Back Channel UCA1TXD
#define UCB1_CS_LCD			 (0x10)					 // 4 Chip Select
#define UCB1CLK				 (0x20)					 // 5 SPI mode - clock output—UCB1CLK
#define UCB1SIMO			 (0x40)					 // 6 UCB1SIMO
#define UCB1SOMI			 (0x80)					 // 7 UCB1SOMI

// ------------------------------------------------------------------------------
//  Port 5 pins
//
//  Description: This function contains all the definitions for port 5.
//
//
// ------------------------------------------------------------------------------

#define V_BAT                (0x01)                  // 0 BATTERY VOLTAGE
#define V_5                  (0x02)                  // 1 5V
#define V_DAC                (0x04)                  // 2 DAC VOLTAGE
#define V3_3                 (0x08)                  // 3 3.3V
#define IOT_BOOT_CPU          (0x10)                 // 4 Boot IOT CPU

// ------------------------------------------------------------------------------
//  Port 6 pins
//
//  Description: This function contains all the definitions for port 6.
//
// ------------------------------------------------------------------------------

#define LCD_BACKLITE         (0x01)                  // 0 TOGGLE LCD BACKLIGHT
#define R_FORWARD            (0x02)                  // 1 RIGHT FORWARD
#define L_FORWARD            (0x04)                  // 2 LEFT FORWARD
#define R_REVERSE            (0x08)                  // 3 RIGHT REVERSE
#define L_REVERSE            (0x10)                  // 4 LEFT REVERSE
#define P6_5                 (0x20)                  // 5 P6_5
#define GRN_LED              (0x40)                  // 6 GREEN LED
