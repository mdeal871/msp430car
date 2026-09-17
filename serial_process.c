// ------------------------------------------------------------------------------
//  File Name: serial_process.c
//
//  Description: Main-loop serial processing.
//
//  Buffer layers:
//    Ring buffers (ISR -> main):
//      PC_2_IOT[16]  bytes from PC
//      IOT_2_PC[128] bytes from IOT
//
//    Stage buffers (assemble lines):
//      uca1_stage[64]  accumulates PC chars until CR/LF
//      uca0_stage[64]  accumulates IOT chars until CR/LF
//
//    Display buffers (LCD-ready, written only on complete commit):
//      rx_message[11]      last complete line from PC
//      current_message[64] last complete line from IOT (full line for +IPD parsing)
//
//  Key design note for IOT parsing:
//    Serial_Process_UCA0_RX calls IOT_Parse_Response() immediately after
//    committing each line.  This is critical because multiple lines from the
//    ESP32 (e.g. "+CWJAP:...\r\n" followed immediately by "OK\r\n") arrive
//    in the same ring-drain pass.  Without inline parsing, the "OK" line would
//    overwrite "+CWJAP:..." before IOT_Parse_Response() ever sees it.
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
//  Display buffers (extern declared in serial.h)
// ------------------------------------------------------------------------------
char rx_message[MSG_BUF_SIZE];            // PC display: 10 chars + null
char current_message[IOT_MSG_BUF_SIZE];   // IOT full line: up to 63 chars + null

// ------------------------------------------------------------------------------
//  Stage buffers
// ------------------------------------------------------------------------------
#define STAGE_SIZE   (64)
#define CMD_BUF_SIZE (32)

static char uca1_stage[STAGE_SIZE];
static char uca0_stage[STAGE_SIZE];
static int  uca1_stage_idx = 0;
static int  uca0_stage_idx = 0;

static char          cmd_buf[CMD_BUF_SIZE];
static unsigned char cmd_active = 0;
static unsigned int  cmd_len    = 0;

// ------------------------------------------------------------------------------
//  Commit_Stage: copy first 10 chars of stage into 11-byte display buffer
// ------------------------------------------------------------------------------
static void Commit_Stage(char *stage, int stage_len, char *dest) {
    int i;
    for (i = 0; i < 10; i++) dest[i] = (i < stage_len) ? stage[i] : ' ';
    dest[10] = '\0';
}

// ------------------------------------------------------------------------------
//  Serial_Process  -  called every main loop iteration
// ------------------------------------------------------------------------------
void Serial_Process(void) {
    Serial_Process_UCA1_RX();
    Serial_Process_UCA0_RX();
    IOT_Parse_Response();
}

// ------------------------------------------------------------------------------
//  Serial_Process_UCA1_RX  -  drain PC ring buffer into stage / command buffer
// ------------------------------------------------------------------------------
void Serial_Process_UCA1_RX(void) {
    if (!uca1_rx_flag) return;
    uca1_rx_flag = 0;

    while (usb_rx_rd != usb_rx_wr) {
        char c = (char)PC_2_IOT[usb_rx_rd++];
        if (usb_rx_rd >= PC_RING_SIZE) usb_rx_rd = BEGINNING;

        if (c == '\r' || c == '\n') {
            if (cmd_active) {
                cmd_buf[cmd_len] = '\0';
                IOT_Command_Dispatch(cmd_buf);
                cmd_active = 0;
                cmd_len    = 0;
            } else if (uca1_stage_idx > 0) {
                Commit_Stage(uca1_stage, uca1_stage_idx, rx_message);
                display_changed = TRUE;
                update_display  = TRUE;
            }
            uca1_stage_idx = 0;
            uca1_stage[0]  = '\0';

        } else if (c == CMD_START_CHAR) {
            cmd_active     = 1;
            cmd_len        = 0;
            cmd_buf[0]     = '\0';
            uca1_stage_idx = 0;
            uca1_stage[0]  = '\0';

        } else {
            if (cmd_active) {
                if (cmd_len < CMD_BUF_SIZE - 1) cmd_buf[cmd_len++] = c;
            } else {
                if (uca1_stage_idx < STAGE_SIZE - 1) {
                    uca1_stage[uca1_stage_idx++] = c;
                    uca1_stage[uca1_stage_idx]   = '\0';
                }
            }
        }
    }
}

// ------------------------------------------------------------------------------
//  Serial_Process_UCA0_RX  -  drain IOT ring buffer into stage
//
//  current_message holds the FULL line (up to 63 chars) for +IPD parsing.
//
//  IMPORTANT: IOT_Parse_Response() is called immediately after each line is
//  committed.  The ESP32 sends multi-line responses (e.g. "+CWJAP:...\r\nOK\r\n")
//  that all arrive in the ring at once.  If we waited until the ring was fully
//  drained, "OK" would overwrite "+CWJAP:..." and the SSID would never be parsed.
// ------------------------------------------------------------------------------
void Serial_Process_UCA0_RX(void) {
    if (!uca0_rx_flag) return;
    uca0_rx_flag = 0;

    while (iot_rx_rd != iot_rx_wr) {
        char c = (char)IOT_2_PC[iot_rx_rd++];
        if (iot_rx_rd >= IOT_RING_SIZE) iot_rx_rd = BEGINNING;

        if (c == '\r' || c == '\n') {
            if (uca0_stage_idx > 0) {
                // Commit full line to current_message
                int copy_len = uca0_stage_idx;
                if (copy_len > IOT_MSG_BUF_SIZE - 1) copy_len = IOT_MSG_BUF_SIZE - 1;
                int i;
                for (i = 0; i < copy_len; i++) current_message[i] = uca0_stage[i];
                current_message[copy_len] = '\0';
                display_changed = TRUE;
                update_display  = TRUE;

                // Parse this line immediately before it can be overwritten
                // by the next line in the ring (e.g. "OK" after "+CWJAP:...")
                IOT_Parse_Response();
            }
            uca0_stage_idx = 0;
            uca0_stage[0]  = '\0';

        } else {
            if (uca0_stage_idx < STAGE_SIZE - 1) {
                uca0_stage[uca0_stage_idx++] = c;
                uca0_stage[uca0_stage_idx]   = '\0';
            }
        }
    }
}
