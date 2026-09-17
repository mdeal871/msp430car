// ------------------------------------------------------------------------------
//  File Name : serial.c
//
//  Description: This file contains UART initialization for UCA0 (IOT module)
//  and UCA1 (PC back-channel), ring buffer storage, and all UART ISRs.
//
//  Cross-link architecture (see Project 09 block diagram):
//    PC  --> UCA1 RX --> PC_2_IOT buffer --> UCA0 TX --> IOT
//    IOT --> UCA0 RX --> IOT_2_PC buffer --> UCA1 TX --> PC
//
//  pc_enabled gate (Project 09 requirement):
//    The FRAM must NOT transmit to the PC until it receives the first character
//    FROM the PC.  pc_enabled starts at 0 (blocked) and is set to 1 inside the
//    UCA1 RX ISR the first time any character arrives.
//
//  Mason Deal
//  Mar 2026
//  Built with Code Composer Studio version 12.8.1.00005
//
// ------------------------------------------------------------------------------
#include "msp430.h"
#include "functions.h"
#include "macros.h"
#include "globals.h"
#include "serial.h"
#include <string.h>

// ------------------------------------------------------------------------------
//  Ring Buffer Definitions (declared extern in serial.h)
// ------------------------------------------------------------------------------
volatile unsigned char PC_2_IOT[PC_RING_SIZE];    // PC  RX -> IOT TX (16 bytes)
volatile unsigned char IOT_2_PC[IOT_RING_SIZE];   // IOT RX -> PC  TX (128 bytes)

volatile unsigned int usb_rx_wr  = BEGINNING; // UCA1 RX writes here
volatile unsigned int iot_rx_wr  = BEGINNING; // UCA0 RX writes here
unsigned int usb_rx_rd  = BEGINNING;           // main loop / display reads from here
unsigned int iot_rx_rd  = BEGINNING;           // main loop / display reads from here
unsigned int direct_iot = BEGINNING;           // UCA0 TX ISR reads from here
unsigned int direct_usb = BEGINNING;           // UCA1 TX ISR reads from here

volatile unsigned char suppress_iot_forward = 0;

// ------------------------------------------------------------------------------
//  General Serial State Definitions (declared extern in serial.h)
// ------------------------------------------------------------------------------
volatile unsigned char uca0_rx_flag    = 0;
volatile char          uca0_rx_char    = 0;
volatile char          uca1_rx_char    = 0;
volatile unsigned char uca1_rx_flag    = 0;
volatile unsigned char pc_enabled      = 0;   // BLOCKED until first PC character received
char baud_string[8]                    = "115,200";

// Temporary variable for ISR index arithmetic (avoids modifying index mid-expression)
volatile unsigned int temp = 0;

// ------------------------------------------------------------------------------
//  Init_Serial_UCA0
//  Configures eUSCI_A0 for UART - IOT module (J9)
//  Pins: P1.6 = UCA0TXD, P1.7 = UCA0RXD  (SEL bits set in ports.c)
//
//  Baud rate table (SMCLK = 8 MHz):
//  Baud    UCOS16  UCBRx  UCFx  UCSx
//  9600      1      52    1    0x49   -> MCTLW = 0x4911
//  19200     1      26    0    0xB6   -> MCTLW = 0xB601
//  115200    1       4    5    0x55   -> MCTLW = 0x5551
//  460800    0      17    0    0x4A   -> MCTLW = 0x4A00
// ------------------------------------------------------------------------------
void Init_Serial_UCA0(unsigned long baud_rate) {
    UCA0CTLW0  = 0;
    UCA0CTLW0 |= UCSWRST;          // Hold in reset during config
    UCA0CTLW0 |= UCSSEL__SMCLK;    // Clock = SMCLK (8 MHz)
    UCA0CTLW0 &= ~UCMSB;           // LSB first
    UCA0CTLW0 &= ~UCSPB;           // 1 stop bit
    UCA0CTLW0 &= ~UCPEN;           // No parity
    UCA0CTLW0 &= ~UCSYNC;          // Async (UART) mode
    UCA0CTLW0 &= ~UC7BIT;          // 8 data bits
    UCA0CTLW0 |=  UCMODE_0;        // UART mode

    switch (baud_rate) {
        case BAUD_9600:
            UCA0BRW   = 52;
            UCA0MCTLW = 0x4911;
            break;
        case BAUD_460800:
            UCA0BRW   = 17;
            UCA0MCTLW = 0x4A00;
            break;
        case BAUD_19200:
            UCA0BRW   = 26;
            UCA0MCTLW = 0xB601;
            break;
        default:                    // BAUD_115200
            UCA0BRW   = 4;
            UCA0MCTLW = 0x5551;
            break;
    }

    UCA0CTLW0 &= ~UCSWRST;         // Release from reset - UART active
    UCA0TXBUF   = 0x00;            // Prime TX pump
    UCA0IE     |= UCRXIE;          // Enable RX interrupt
}

// ------------------------------------------------------------------------------
//  Init_Serial_UCA1
//  Configures eUSCI_A1 for UART - PC back-channel (J14)
//  Pins: P4.2 = UCA1RXD, P4.3 = UCA1TXD  (SEL bits set in ports.c)
//
//  pc_enabled is explicitly cleared here so power-cycling always resets the gate.
// ------------------------------------------------------------------------------
void Init_Serial_UCA1(unsigned long baud_rate) {
    pc_enabled = 0;                 // Block TX to PC until first character received from PC

    UCA1CTLW0  = 0;
    UCA1CTLW0 |= UCSWRST;
    UCA1CTLW0 |= UCSSEL__SMCLK;
    UCA1CTLW0 &= ~UCMSB;
    UCA1CTLW0 &= ~UCSPB;
    UCA1CTLW0 &= ~UCPEN;
    UCA1CTLW0 &= ~UCSYNC;
    UCA1CTLW0 &= ~UC7BIT;
    UCA1CTLW0 |=  UCMODE_0;

    switch (baud_rate) {
        case BAUD_9600:
            UCA1BRW   = 52;
            UCA1MCTLW = 0x4911;
            break;
        case BAUD_460800:
            UCA1BRW   = 17;
            UCA1MCTLW = 0x4A00;
            break;
        case BAUD_19200:
            UCA1BRW   = 26;
            UCA1MCTLW = 0xB601;
            break;
        default:                    // BAUD_115200
            UCA1BRW   = 4;
            UCA1MCTLW = 0x5551;
            break;
    }

    UCA1CTLW0 &= ~UCSWRST;
    UCA1TXBUF   = 0x00;
    UCA1IE     |= UCRXIE;
}

// ------------------------------------------------------------------------------
//  Set_Baud_Rate
//  Reinitializes both UCA0 and UCA1 at the requested baud rate.
//  NOTE: pc_enabled is NOT cleared here - only Init_Serial_UCA1 clears it.
// ------------------------------------------------------------------------------
void Set_Baud_Rate(unsigned long baud_rate) {
    unsigned char saved_pc_enabled = pc_enabled;  // preserve gate across reinit
    UCA0IE &= ~(UCRXIE | UCTXIE);
    UCA1IE &= ~(UCRXIE | UCTXIE);

    UCA0CTLW0 |= UCSWRST;
    UCA1CTLW0 |= UCSWRST;

    Init_Serial_UCA0(baud_rate);
    Init_Serial_UCA1(baud_rate);

    pc_enabled = saved_pc_enabled;  // restore - do not re-block an active session
    UCA0CTLW0 &= ~UCSWRST;
    UCA1CTLW0 &= ~UCSWRST;

    UCA0IE |= UCRXIE;
    UCA1IE |= UCRXIE;
}

// ------------------------------------------------------------------------------
//  USCI_A1_transmit - Blocking transmit over UCA1 (to PC).
//  Respects pc_enabled gate: does nothing if PC has not yet sent a character.
// ------------------------------------------------------------------------------
void USCI_A1_transmit(char *string) {
    if (!pc_enabled) return;        // Gate: do not transmit to PC before it is ready
    while (*string) {
        while (!(UCA1IFG & UCTXIFG));
        UCA1TXBUF = *string++;
    }
}

// ------------------------------------------------------------------------------
//  USCI_A0_transmit - Blocking transmit over UCA0 (to IOT).
// ------------------------------------------------------------------------------
void USCI_A0_transmit(char *string) {
    while (*string) {
        while (!(UCA0IFG & UCTXIFG));
        UCA0TXBUF = *string++;
    }
}

// ==============================================================================
//  INTERRUPT SERVICE ROUTINES
// ==============================================================================

// ------------------------------------------------------------------------------
//  eUSCI_A1_ISR  (UCA1 - PC back-channel)
//
//  RX (Vector 2): character arrives from PC
//    1. Open the pc_enabled gate on first received character.
//    2. Store in PC_2_IOT ring buffer so the main loop can read it for display.
//    3. Also forward directly to UCA0 TX to pass through to IOT.
//       - Characters starting with CMD_START_CHAR are FRAM-only commands;
//         the command parser in serial_process.c will suppress forwarding them.
//         However, we still buffer them here so the main loop can parse them.
//    4. Enable UCA0 TX interrupt to drain the buffer to the IOT.
//
//  TX (Vector 4): UCA1 TX buffer empty
//    - Send next byte from IOT_2_PC (IOT data going to PC).
//    - Only transmit if pc_enabled; disable TX interrupt when buffer drained.
// ------------------------------------------------------------------------------
#pragma vector = EUSCI_A1_VECTOR
__interrupt void eUSCI_A1_ISR(void) {
    switch (__even_in_range(UCA1IV, 0x08)) {
        case 0: break;  // No interrupt

        case 2: {       // RXIFG - character from PC
            unsigned char rxChar = UCA1RXBUF;   // Read buffer exactly once

            // Open the gate: FRAM may now transmit back to the PC
            pc_enabled = 1;

            // Store in ring buffer - guard against overflow
            temp = (usb_rx_wr + 1 >= SMALL_RING_SIZE) ? BEGINNING : usb_rx_wr + 1;
            if (temp != usb_rx_rd) {
                PC_2_IOT[usb_rx_wr] = rxChar;
                usb_rx_wr = temp;
            }

            // Capture for serial_process main loop
            uca1_rx_char = rxChar;
            uca1_rx_flag = 1;

            // Detect command prefix in the ISR to close the race window.
            // 0x5E = '^'  0x0D = CR  0x0A = LF
            // Using hex literals avoids any escape-sequence encoding issues.
            if (rxChar == 0x5E) {
                // '^' starts a FRAM-only command - block forwarding, don't send to IOT
                suppress_iot_forward = 1;
            } else if (rxChar == 0x0D || rxChar == 0x0A) {
                // CR/LF ends a command. Clear the flag AFTER deciding not to forward
                // this CR/LF (it belongs to the command, not the IOT passthrough).
                // If suppress was already 0, this is a normal CR/LF - forward it.
                if (!suppress_iot_forward) {
                    UCA0IE |= UCTXIE;   // forward the CR/LF to IOT (normal passthrough)
                }
                suppress_iot_forward = 0;   // clear after decision - next chars forward normally
            } else {
                // Normal printable character - forward only if not in command mode
                if (!suppress_iot_forward) {
                    UCA0IE |= UCTXIE;
                }
            }
        } break;

        case 4: {       // TXIFG - ready to send next byte to PC
            // Hard break if PC not ready: without this, direct_usb never
            // advances, the drain check never fires, and the ISR loops forever.
            if (!pc_enabled) {
                UCA1IE &= ~UCTXIE;
                break;
            }
            UCA1TXBUF = IOT_2_PC[direct_usb++];
            if (direct_usb >= IOT_RING_SIZE) {
                direct_usb = BEGINNING;
            }
            if (direct_usb == iot_rx_wr) {
                UCA1IE &= ~UCTXIE;  // Buffer drained
            }
        } break;

        default: break;
    }
}

// ------------------------------------------------------------------------------
//  eUSCI_A0_ISR  (UCA0 - IOT module)
//
//  RX (Vector 2): character arrives from IOT
//    - Store in IOT_2_PC ring buffer.
//    - Enable UCA1 TX interrupt to forward to PC (if pc_enabled gate is open).
//
//  TX (Vector 4): UCA0 TX buffer empty
//    - Send next byte from PC_2_IOT (PC data forwarded to IOT).
//    - Disable TX interrupt when buffer is drained.
// ------------------------------------------------------------------------------
#pragma vector = EUSCI_A0_VECTOR
__interrupt void eUSCI_A0_ISR(void) {
    switch (__even_in_range(UCA0IV, 0x08)) {
        case 0: break;  // No interrupt

        case 2: {       // RXIFG - character from IOT
            unsigned char rxChar = UCA0RXBUF;   // Read buffer exactly once

            // Guard against overflow: drop byte rather than overwrite unread data
            temp = (iot_rx_wr + 1 >= IOT_RING_SIZE) ? BEGINNING : iot_rx_wr + 1;
            if (temp != iot_rx_rd) {
                IOT_2_PC[iot_rx_wr] = rxChar;
                iot_rx_wr = temp;
            }
            // If full, rxChar is silently dropped - a dropped 'OK' is safer than
            // a clobbered '+IPD' line causing a phantom command dispatch.

            uca0_rx_char = rxChar;
            uca0_rx_flag = 1;


            pc_enabled = 1;  // ← ADD THIS LINE ← CRITICAL FIX

            // Enable UCA1 TX ISR to forward this byte to the PC
            // (UCA1 TX ISR checks pc_enabled before placing byte in TX buffer)
            UCA1IE |= UCTXIE;
        } break;

        case 4: {       // TXIFG - ready to send next byte to IOT
            if (suppress_iot_forward) {
                // '^' command in flight - drain byte without sending to IOT
                direct_iot++;
            } else {
                UCA0TXBUF = PC_2_IOT[direct_iot++];
            }
            if (direct_iot >= SMALL_RING_SIZE) direct_iot = BEGINNING;
            if (direct_iot == usb_rx_wr) {
                UCA0IE &= ~UCTXIE;  // Buffer drained
            }
        } break;

        default: break;
    }
}
