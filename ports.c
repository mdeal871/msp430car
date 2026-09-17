// ------------------------------------------------------------------------------
//  File Name :ports.c
//
//  Description: This file contains all port initialization functions.
//  Please see the port spreadsheet for more details
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

#define LOW (0x00)
#define OUTPUT (0x00)

void Init_Ports(void);
void Init_Port1(void);
void Init_Port2(void);
void Init_Port3(void);
void Init_Port4(void);
void Init_Port5(void);
void Init_Port6(void);

// ------------------------------------------------------------------------------
//  Function for Port1
//
//  Description: This function contains all the value definitions for Port1
//  P1.0/UCB0STE/SMCLK/COMP0.0/A0/VEREF+/DAC0REF
//  P1.1/UCB0CLK/ACLK/OA0O/COMP0.1/A1
//  P1.2/UCB0SIMO/UCB0SDA/TBTRG/OA0-/A2/VEREF
//  P1.3/UCB0SOMI/UCB0SCL/OA0+/A3
//  P1.4/UCA0STE/TCK/A4/DAC1REF
//  P1.5/UCA0CLK/TMS/OA10/A5
//  P1.6/UCA0RXD/UCA0SOMI/TB0.1/TDI/TCLK/OA1-/A6
//  P1.7/UCA0TXD/UCA0SIMO/TBD0.2/TDO/0A1+/A7/VREF+
//
// ------------------------------------------------------------------------------
void Init_Port1(void) {
P1OUT = LOW;                                     // P1 set low
P1DIR = OUTPUT;                                  // Set P1 direction to output

P1SEL0 &= ~RED_LED;								 // RED_LED GPI operation
P1SEL1 &= ~RED_LED;								 // RED_LED GPIO operation
P1OUT &= ~RED_LED;								 // Initial value = low / off
P1DIR |= RED_LED;								 // Direction = output

P1SEL0 &= ~A1_SEEED;                              // A1_SEEED GPI operation
P1SEL1 &= ~A1_SEEED;                              // A1_SEEED GPIO operation
P1OUT &= ~A1_SEEED;                               // Initial value = low / off
P1DIR &= ~A1_SEEED;                               // Direction = input

P1SEL0 |= V_DETECT_L;							 // V_DETECT_L operation
P1SEL1 |= V_DETECT_L;							 // V_DETECT_L operation

P1SEL0 |= V_DETECT_R;							 // V_DETECT_R operation
P1SEL1 |= V_DETECT_R;							 // V_DETECT_R operation

P1SEL0 &= ~A4_SEEED;                              // A4_SEEED GPI operation
P1SEL1 &= ~A4_SEEED;                              // A4_SEEED GPIO operation
P1OUT &= ~A4_SEEED;                               // Initial value = low / off
P1DIR &= ~A4_SEEED;                               // Direction = input

P1SEL0 |= V_THUMB;								 // V_THUMB operation
P1SEL1 |= V_THUMB;								 // V_THUMB operation

P1SEL0 |= UCA0TXD;								 // UCA0TXD operation
P1SEL1 &= ~UCA0TXD;								 // UCA0TXD operation

P1SEL0 |= UCA0RXD;								 // UCA0RXD operation
P1SEL1 &= ~UCA0RXD;								 // UCA0RXD operation
}

// ------------------------------------------------------------------------------
//  Function for Port2
//
//  Description: This function contains all the value definitions for Port2
//  P2.0/TB1.1/COMP0.O
//  P2.1/TB1.2/COMP1.O
//  P2.2/TB1CLK/HOUT
//  P2.3/TB1TRG
//  P2.4/COMP1.1
//  P2.5/COMP1.0
//  P2.6/MCLK/XOUT
//  P2.7/TB0CLK/XIN
//
// ------------------------------------------------------------------------------
void Init_Port2(void){
P2OUT = LOW;                                     // P2 set Low
P2DIR = OUTPUT;                                     // Set P2 direction to output

P2SEL0 &= ~SLOW_CLK;                             // SLOW_CLK GPIO operation
P2SEL1 &= ~SLOW_CLK;                             // SLOW_CLK GPIO operation
P2OUT &= ~SLOW_CLK;                              // Initial Value = Low / Off
P2DIR |= SLOW_CLK;                               // Direction = output

P2SEL0 &= ~CHECK_BAT;                            // CHECK_BAT GPIO operation
P2SEL1 &= ~CHECK_BAT;                            // CHECK_BAT GPIO operation
P2OUT &= ~CHECK_BAT;                             // Initial Value = Low / Off
P2DIR |= CHECK_BAT;                              // Direction = output

P2SEL0 &= ~IR_LED;                               // P2_2 GPIO operation
P2SEL1 &= ~IR_LED;                               // P2_2 GPIO operation
P2OUT &= ~IR_LED;                                // Initial Value = Low / Off
P2DIR |= IR_LED;                                 // Direction = output

P2SEL0 &= ~SW2;                                  // SW2 Operation
P2SEL1 &= ~SW2;                                  // SW2 Operation
P2OUT |= SW2;                                    // Configure pullup resistor
P2DIR &= ~SW2;                                   // Direction = input
P2REN |= SW2;                                    // Enable pullup resistor

P2SEL0 &= ~IOT_RUN_RED;                          // IOT_RUN_CPU GPIO operation
P2SEL1 &= ~IOT_RUN_RED;                          // IOT_RUN_CPU GPIO operation
P2OUT &= ~IOT_RUN_RED;                           // Initial Value = Low / Off
P2DIR |= IOT_RUN_RED;                            // Direction = output

P2SEL0 &= ~DAC_ENB;                              // DAC_ENB GPIO operation
P2SEL1 &= ~DAC_ENB;                              // DAC_ENB GPIO operation
P2OUT |= DAC_ENB;                                // Initial Value = High
P2DIR |= DAC_ENB;                                // Direction = output

P2SEL0 &= ~LFXOUT;                               // LFXOUT Clock operation
P2SEL1 |= LFXOUT;                                // LFXOUT Clock operation

P2SEL0 &= ~LFXIN;                                // LFXIN Clock operation
P2SEL1 |= LFXIN;                                 // LFXIN Clock operation
//------------------------------------------------------------------------------
}

// ------------------------------------------------------------------------------
//  Function for Port3
//
//  Description: This function contains all the value definitions for Port3
//  P3.0/MCLK/DAC2REF
//  P3.1/OA2O
//  P3.2/OA2-
//  P3.3/OA2+
//  P3.4/SMCLK/DAC3REF
//  P3.5/OA30
//  P3.6/OA3-
//  P3.7/OA3+
//
// ------------------------------------------------------------------------------
void Init_Port3(void) {
P3OUT = LOW;                                     // P3 set low
P3DIR = OUTPUT;                                     // Set P3 direction to output

P3SEL0 &= ~TEST_PROBE;							 // TEST_PROBE GPIO operation
P3SEL1 &= ~TEST_PROBE;							 // TEST_PROBE GPIO operation
P3OUT &= ~TEST_PROBE;							 // Initial Value = Low / Off
P3DIR |=  TEST_PROBE;							 // Direction = output

P3SEL0 &= ~OA20;                                 // OA20 GPI operation
P3SEL1 &= ~OA20;                                 // OA20 GPIO operation
P3OUT &= ~OA20;                                  // Initial value = low / off
P3DIR &= ~OA20;                                  // Direction = input

P3SEL0 &= ~OA2N;                                 // OA2N GPI operation
P3SEL1 &= ~OA2N;                                 // OA2N GPIO operation
P3OUT &= ~OA2N;                                  // Initial value = low / off
P3DIR &= ~OA2N;                                  // Direction = input

P3SEL0 &= ~OA2P;                                 // OA2P GPI operation
P3SEL1 &= ~OA2P;                                 // OA2P GPIO operation
P3OUT &= ~OA2P;                                  // Initial value = low / off
P3DIR &= ~OA2P;                                  // Direction = input

P3SEL0 &= ~SMCLK_OUT;							 // SMCLK_OUT GPIO operation
P3SEL1 &= ~SMCLK_OUT;							 // SMCLK_OUT GPIO operation
P3OUT &= ~SMCLK_OUT;							 // Initial Value = Low / Off
P3DIR &= ~SMCLK_OUT;							 // Direction = input

P3SEL0 &= ~DAC_CTRL_3;							 // DAC_CTRL_3 GPIO operation
P3SEL1 &= ~DAC_CTRL_3;							 // DAC_CTRL_3 GPIO operation
P3OUT &= ~DAC_CTRL_3;							 // Initial Value = Low / Off
P3DIR &= ~DAC_CTRL_3;							 // Direction = input

P3SEL0 &= ~IOT_LINK_CPU;						 // IOT_LINK_CPU GPIO operation
P3SEL1 &= ~IOT_LINK_CPU;						 // IOT_LINK_CPU GPIO operation
P3OUT &= ~IOT_LINK_CPU;							 // Initial Value = Low / Off
P3DIR &= ~IOT_LINK_CPU;							 // Direction = input

P3SEL0 &= ~IOT_RN_CPU;							 // IOT_RN_CPU GPIO operation
P3SEL1 &= ~IOT_RN_CPU;							 // IOT_RN_CPU GPIO operation
P3OUT &= ~IOT_RN_CPU;							 // Initial Value = Low / Off
P3DIR |=  IOT_RN_CPU;							 // Direction = output - drives IOT reset
}

// ------------------------------------------------------------------------------
//  Function for Port4
//
//  Description: This function contains all the value definitions for Port4
//  P4.0/UCA1STE/ISOTXD/ISORXD
//  P4.1/UCA1CLK
//  P4.2/UCA1RXD/UCA1SOMI/UCA1RXD
//  P4.3/UCA1TXD/UCA1SIMO/UCA1TXD
//  P4.4/UCB1STE
//  P4.5/UCB1CLK
//  P4.6/UCB1SIMO/UCB1SDA
//  P4.7/UCB1SOMI/UCB1SCL
//
// ------------------------------------------------------------------------------
void Init_Port4(void){
P4OUT = LOW;									 // P4 set Low
P4DIR = OUTPUT; 							     // Set P4 direction to output

P4SEL0 &= ~RESET_LCD; 							 // RESET_LCD GPIO operation
P4SEL1 &= ~RESET_LCD; 							 // RESET_LCD GPIO operation
P4OUT &= ~RESET_LCD;							 // Initial Value = Low / Off
P4DIR |= RESET_LCD;								 // Direction = output

P4SEL0 &= ~SW1;									 // SW1 GPIO operation
P4SEL1 &= ~SW1;									 // SW1 GPIO operation
P4OUT |= SW1;									 // Configure pullup resistor
P4DIR &= ~SW1;									 // Direction = input
P4REN |= SW1;									 // Enable pullup resistor

P4SEL0 |= UCA1TXD;								 // USCI_A1 UART operation
P4SEL1 &= ~UCA1TXD;								 // USCI_A1 UART operation

P4SEL0 |= UCA1RXD;								 // USCI_A1 UART operation
P4SEL1 &= ~UCA1RXD;								 // USCI_A1 UART operation

P4SEL0 &= ~UCB1_CS_LCD;							 // UCB1_CS_LCD GPIO operation
P4SEL1 &= ~UCB1_CS_LCD;							 // UCB1_CS_LCD GPIO operation
P4OUT |= UCB1_CS_LCD;							 // Set SPI_CS_LCD Off [High]
P4DIR |= UCB1_CS_LCD;							 // Set SPI_CS_LCD direction to output

P4SEL0 |= UCB1CLK;								 // UCB1CLK SPI BUS operation
P4SEL1 &= ~UCB1CLK;								 // UCB1CLK SPI BUS operation

P4SEL0 |= UCB1SIMO;								 // UCB1SIMO SPI BUS operation
P4SEL1 &= ~UCB1SIMO;							 // UCB1SIMO SPI BUS operation

P4SEL0 |= UCB1SOMI;								 // UCB1SOMI SPI BUS operation
P4SEL1 &= ~UCB1SOMI;							 // UCB1SOMI SPI BUS operation
//------------------------------------------------------------------------------
}

// ------------------------------------------------------------------------------
//  Function for Port5
//
//  Description: This function contains all the value definitions for Port5
//  P5.0/TB2.1/MFM.RX/A8
//  P5.1/TB2.2/MFM.TX/A9
//  P5.2/TB2CLK/A10
//  P5.3/TB2TRGA/A11
//  P5.4
//
// ------------------------------------------------------------------------------
void Init_Port5(void){
P5OUT = LOW;                                     // set P5 low
P5DIR = OUTPUT;                                  // Set P5 direction to output

P5SEL0 &= ~V_BAT;							     // V_BAT GPIO operation
P5SEL1 &= ~V_BAT;								 // V_BAT GPIO operation
P5OUT &= ~V_BAT;								 // set initial value to low / off
P5DIR &= ~V_BAT;								 // set direction to input

P5SEL0 &= ~V_5;								     // V_5 GPIO operation
P5SEL1 &= ~V_5;									 // V_5 GPIO operation
P5OUT &= ~V_5;									 // set initial value to low / off
P5DIR &= ~V_5;									 // set direction to input

P5SEL0 &= ~V_DAC;							     // V_DAC GPIO operation
P5SEL1 &= ~V_DAC;								 // V_DAC GPIO operation
P5OUT &= ~V_DAC;								 // set initial value to low / off
P5DIR &= ~V_DAC;								 // set direction to input

P5SEL0 &= ~V3_3;							     // V3_3 GPIO operation
P5SEL1 &= ~V3_3;								 // V3_3 GPIO operation
P5OUT &= ~V3_3;									 // set initial value to low / off
P5DIR &= ~V3_3;									 // set direction to input

P5SEL0 &= ~IOT_BOOT_CPU;					     // IOT_BOOT_CPU GPIO operation
P5SEL1 &= ~IOT_BOOT_CPU;						 // IOT_BOOT_CPU GPIO operation
P5OUT &= ~IOT_BOOT_CPU;							 // set initial value to low / off
P5DIR &= ~IOT_BOOT_CPU;							 // set direction to input
}

// ------------------------------------------------------------------------------
//  Function for Port6
//
//  Description: This function contains all the value definitions for Port6
//  P6.0/TB3.1
//  P6.1/TB3.2
//  P6.2/TB3.3
//  P6.3/TB3.4
//  P6.4/TB3.5
//  P6.5/TB3.6
//  P6.6/TB3CLK
//
// ------------------------------------------------------------------------------
void Init_Port6(void) {
    P6OUT = LOW;                                     // set p6 low
    P6DIR = OUTPUT;                                  // Set p6 direction to output

    P6SEL0 |= LCD_BACKLITE;                          // LCD_BACKLITE T3.1 operation
    P6SEL1 &= ~LCD_BACKLITE;                         // LCD_BACKLITE TB3.1 operation
    //P6OUT |= LCD_BACKLITE;                             // set initial value to on / high
    P6DIR |= LCD_BACKLITE;                           // set direction to output

    P6SEL0 |= R_FORWARD;                             // R_FORWARD TB3.2 operation
    P6SEL1 &= ~R_FORWARD;                            // R_FORWARD TB3.2 operation
    //P6OUT &= ~R_FORWARD;                           // set initial value to low / off
    P6DIR |= R_FORWARD;                              // set direction to output

    P6SEL0 |= L_FORWARD;                             // L_FORWARD TB3.3 operation
    P6SEL1 &= ~L_FORWARD;                            // L_FORWARD TB3.3 operation
    //P6OUT &= ~L_FORWARD;                           // set initial value to low / off
    P6DIR |= L_FORWARD;                              // set direction to output

    P6SEL0 |= R_REVERSE;                             // R_REVERSE TB3.4 operation
    P6SEL1 &= ~R_REVERSE;                            // R_REVERSE TB3.4 operation
    //P6OUT &= ~R_REVERSE;                           // set initial value to low / off
    P6DIR |= R_REVERSE;                               // set direction to output

    P6SEL0 |= L_REVERSE;                             // L_REVERSE TB3.5 operation
    P6SEL1 &= ~L_REVERSE;                            // L_REVERSE TB3.5 operation
    //P6OUT &= ~L_REVERSE;                           // set initial value to low / off
    P6DIR|= L_REVERSE;                               // set direction to output

    P6SEL0 &= ~P6_5;                                 // V3_3 GPIO operation
    P6SEL1 &= ~P6_5;                                 // V3_3 GPIO operation
    P6OUT &= ~P6_5;                                  // set initial value to low / off
    P6DIR &= ~P6_5;                                  // set direction to input

    P6SEL0 &= ~GRN_LED;                              // GRN_LED GPIO operation
    P6SEL1 &= ~GRN_LED;                              // GRN_LED GPIO operation
    P6OUT &= ~GRN_LED;                               // set initial value to low / off
    P6DIR |= GRN_LED;                                // set direction to output							 // set direction to output
}
