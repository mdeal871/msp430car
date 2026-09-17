// ------------------------------------------------------------------------------
//  File Name : serial.h
//
//  Description: This file contains all serial definitions, ring buffer externs,
//  and function prototypes for UCA0 (IOT) and UCA1 (PC back-channel).
//
//  Cross-link architecture:
//    PC  --> UCA1 RX --> PC_2_IOT buffer --> UCA0 TX --> IOT
//    IOT --> UCA0 RX --> IOT_2_PC buffer --> UCA1 TX --> PC
//
//  Mason Deal
//  Mar 2026
//  Built with Code Composer Studio version 12.8.1.00005
//
// ------------------------------------------------------------------------------

#ifndef SERIAL_H_
#define SERIAL_H_

// ------------------------------------------------------------------------------
//  Ring Buffer / Index Constants
// ------------------------------------------------------------------------------
#define BEGINNING        (0)    // Reset index for all ring buffers
#define PC_RING_SIZE     (16)   // PC->IOT ring: human typing is slow, 16 is enough
#define IOT_RING_SIZE    (128)  // IOT->PC ring: AT responses burst 100+ bytes at once
#define SMALL_RING_SIZE  (PC_RING_SIZE)  // Legacy alias used by main loop indices
#define MSG_BUF_SIZE     (11)   // PC display buffer: 10 display chars + null
#define IOT_MSG_BUF_SIZE (64)   // IOT message buffer: large enough for full +IPD lines

// Baud rate selectors
#define BAUD_115200  (0)
#define BAUD_460800  (1)
#define BAUD_19200   (2)
#define BAUD_9600    (3)

// Command parsing sentinels
#define CMD_START_CHAR   ('^')   // Marks start of a FRAM-only command (not forwarded to IOT)
#define CMD_END_CHAR     (0x0D)  // Carriage Return = end of command

// ------------------------------------------------------------------------------
//  Flags
// ------------------------------------------------------------------------------
extern volatile unsigned char uca0_rx_flag;     // Set when UCA0 RX ISR captures a byte
extern volatile char          uca0_rx_char;     // Last byte captured by UCA0 RX ISR
extern volatile unsigned char pc_enabled;       // Gate: 0 = block TX to PC, 1 = allow TX to PC
                                                //   Cleared in Init_Serial_UCA1, set on first
                                                //   character received from PC (per proj spec)

// ------------------------------------------------------------------------------
//  Ring Buffers
//  PC_2_IOT - characters from PC (UCA1 RX) queued for IOT (UCA0 TX)
//  IOT_2_PC - characters from IOT (UCA0 RX) queued for PC (UCA1 TX)
// ------------------------------------------------------------------------------
extern volatile unsigned char PC_2_IOT[PC_RING_SIZE];   // PC->IOT: 16 bytes
extern volatile unsigned char IOT_2_PC[IOT_RING_SIZE];  // IOT->PC: 128 bytes

// Ring buffer indices
extern volatile unsigned int usb_rx_wr;  // PC_2_IOT write index  (written by UCA1 RX ISR)
extern volatile unsigned int iot_rx_wr;  // IOT_2_PC write index  (written by UCA0 RX ISR)
extern unsigned int usb_rx_rd;           // PC_2_IOT read index   (read by main loop / display)
extern unsigned int iot_rx_rd;           // IOT_2_PC read index   (read by main loop / display)
extern unsigned int direct_iot;          // PC_2_IOT read index   (consumed by UCA0 TX ISR)
extern unsigned int direct_usb;          // IOT_2_PC read index   (consumed by UCA1 TX ISR)

// Message display buffers - flat, 10 chars + null, always LCD-ready
extern char rx_message[MSG_BUF_SIZE];          // Last complete line from PC  (UCA1)
extern char current_message[IOT_MSG_BUF_SIZE]; // Last complete line from IOT (UCA0)

// ------------------------------------------------------------------------------
//  General Serial State
// ------------------------------------------------------------------------------
extern volatile char          uca1_rx_char;    // Last byte captured by UCA1 RX ISR
extern volatile unsigned char uca1_rx_flag;    // Set when UCA1 RX ISR captures a byte
extern char baud_string[8];                    // Current baud label e.g. "115,200\0"

// ------------------------------------------------------------------------------
//  Function Prototypes - Core Serial
// ------------------------------------------------------------------------------
void Init_Serial_UCA0(unsigned long baud_rate);
void Init_Serial_UCA1(unsigned long baud_rate);
void Set_Baud_Rate(unsigned long baud_rate);
void USCI_A1_transmit(char *string);
void USCI_A0_transmit(char *string);

// ------------------------------------------------------------------------------
//  Function Prototypes - IOT AT Commands
// ------------------------------------------------------------------------------
void Send_IOT_Command(char *command);   // Appends \r\n and sends over UCA0
void IOT_Init_Sequence(void);           // Reset IOT, send AT+CIPMUX / AT+CIPSERVER
void IOT_Connect_WiFi(char *ssid, char *password); // AT+CWJAP
void IOT_Start_Server(unsigned int port);           // AT+CIPSERVER=1,port
void IOT_Parse_Response(void);          // Called from Serial_Process - parses IOT_2_PC msgs
void IOT_Command_Dispatch(char *cmd);   // Routes a parsed '^' command to the correct action

extern volatile unsigned char suppress_iot_forward; // 1 while a ^ command is being parsed

#endif /* SERIAL_H_ */
