// ------------------------------------------------------------------------------
//  File Name : IOT.c
//
//  Description: AT command helpers for the ESP32-WROOM IOT module (UCA0).
//
//  FRAM commands (typed in Termite with '^' prefix):
//    ^^              -> reply "I'm here\r\n" to PC
//    ^F              -> set UCA0 to 115,200, reply "115,200\r\n"
//    ^S              -> set UCA0 to 9,600, reply "9,600\r\n"
//    ^I              -> run IOT_Init_Sequence (CIPMUX + CIPSERVER)
//    ^W<ssid>,<pw>   -> connect to WiFi
//    ^G              -> send AT+CIFSR, display IP on LCD
//
//  TCP motion commands (from Java client via +IPD):
//    ^<PIN><DIR><TIME>  e.g. ^1234F0200
//    PIN  = 4 chars   DIR = F/B/L/R   TIME = ms (up to 4 digits)
//
//  Two motion commands can be chained on one line separated by ';':
//    ^1234F0200;^1234R0090
//
//  IOT Menu (Menu_IOT) is populated by IOT_Parse_Response:
//    - On +CWJAP response:  SSID -> lcd line 1
//    - On +CIFSR response:  IP   -> lcd lines 2-4 (split at '.')
//    - On +IPD command:     motion command displayed on enlarged middle line
//
//  Mason Deal
//  Apr 2026
//  Built with Code Composer Studio version 12.8.1.00005
// ------------------------------------------------------------------------------
#include "msp430.h"
#include "functions.h"
#include "ports.h"
#include "macros.h"
#include "globals.h"
#include "serial.h"
#include <string.h>

const char SECRET_PIN[5] = "0630";

// ------------------------------------------------------------------------------
//  Send_IOT_Command  -  append \r\n and transmit over UCA0
// ------------------------------------------------------------------------------
void Send_IOT_Command(char *command) {
    USCI_A0_transmit(command);
    USCI_A0_transmit("\r\n");
}

// ------------------------------------------------------------------------------
//  IOT_Init_Sequence  -  full auto-connect boot sequence
//
//  Called once after power-on reset delay in main.c.
//  Sequence:
//    1. AT            - verify module is alive
//    2. AT+CWJAP?     - query current WiFi connection
//                       ESP32 remembers last network in flash.
//                       Response "+CWJAP:..." is parsed by IOT_Parse_Response
//                       to populate iot_ssid and set iot_display_ready.
//    3. AT+CIFSR      - get IP address.
//                       Response "+CIFSR:STAIP,..." parsed by IOT_Parse_Response
//                       to populate iot_ip_line3/4.
//    4. AT+CIPMUX=1   - allow multiple TCP connections (required before server)
//    5. AT+CIPSERVER=1,8080 - open TCP server on port 8080
//
//  Each command is separated by a short delay so the ESP32 has time to respond
//  before the next command arrives. Responses are processed asynchronously by
//  IOT_Parse_Response in the main loop - this function just sends the commands.
//
//  If the module is not connected to WiFi, steps 3-5 will still execute but
//  AT+CIFSR will return an empty IP and AT+CIPSERVER may return ERROR (which
//  is harmless - user can connect manually with ^W then ^I).
// ------------------------------------------------------------------------------
// IOT_Wait: wait 'ticks' TB0 ticks (10ms each) while draining the IOT ring buffer.
// At 115200 baud the 128-byte IOT_2_PC ring fills in <11ms. Any blocking delay
// without draining causes overflow and lost responses (SSID/IP never parsed).
// Calling Serial_Process_UCA0_RX + IOT_Parse_Response here processes each line
// as it arrives rather than after the whole sequence completes.
static void IOT_Wait(unsigned int ticks) {
    unsigned int start = Time_Sequence;
    while ((unsigned int)(Time_Sequence - start) < ticks) {
        Serial_Process_UCA0_RX();
        IOT_Parse_Response();
        Display_Process();
    }
}

void IOT_Init_Sequence(void) {
    strcpy(display_line[0], " IOT Boot ");
    strcpy(display_line[1], "Connecting");
    strcpy(display_line[2], "  WiFi... ");
    strcpy(display_line[3], "          ");
    display_changed = TRUE;
    update_display  = TRUE;
    Display_Process();

    // Set IOT_EN (P3.7) to LOW for 100ms to reset
      P3OUT &= ~IOT_RN_CPU;
      __delay_cycles(100UL * 8000UL);  // 100ms at 8MHz

      // Release reset (set IOT_EN to HIGH)
      P3OUT |= IOT_RN_CPU;
      // ESP32 needs ~500ms to boot after reset before it responds to AT commands
      __delay_cycles(1000UL * 8000UL);  // 1000ms at 8MHz

    IOT_Wait(200);

    // Step 1: verify AT communication (500ms)
    Send_IOT_Command("AT");
    IOT_Wait(50);

    // Step 2: query current WiFi SSID (2000ms - WiFi status can be slow)
    // IOT_Wait drains the ring so the +CWJAP: response is parsed immediately.
    Send_IOT_Command("AT+CWJAP?");
    IOT_Wait(200);

    // Step 3: get IP address (500ms)
    Send_IOT_Command("AT+CIFSR");
    IOT_Wait(50);

    // Step 4: enable multiple connections (500ms)
    Send_IOT_Command("AT+CIPMUX=1");
    IOT_Wait(50);

    // Step 5: open TCP server on port 8080 (500ms)
    Send_IOT_Command("AT+CIPSERVER=1,8080");
    IOT_Wait(50);

    // Step 6: resend query for Wifi SSID
    Send_IOT_Command("AT+CWJAP?");
    IOT_Wait(200);

    strcpy(display_line[0], "  Server  ");
    strcpy(display_line[1], " Port 8080");
    strcpy(display_line[2], "  Ready   ");
    strcpy(display_line[3], "          ");
    display_changed = TRUE;
    update_display  = TRUE;
    Display_Process();

    IOT_Wait(300);
}

// ------------------------------------------------------------------------------
//  IOT_Connect_WiFi  -  AT+CWJAP="ssid","password"
// ------------------------------------------------------------------------------
void IOT_Connect_WiFi(char *ssid, char *password) {
    char cmd[80];
    strcpy(cmd, "AT+CWJAP=\"");
    strcat(cmd, ssid);
    strcat(cmd, "\",\"");
    strcat(cmd, password);
    strcat(cmd, "\"");
    Send_IOT_Command(cmd);
    strcpy(display_line[0], "Connecting");
    strncpy(display_line[1], ssid, 10);
    display_line[1][10] = '\0';
    strcpy(display_line[2], "  WiFi..  ");
    strcpy(display_line[3], "          ");
    display_changed = TRUE;
}

// ------------------------------------------------------------------------------
//  IOT_Start_Server  -  standalone CIPMUX + CIPSERVER helper
// ------------------------------------------------------------------------------
void IOT_Start_Server(unsigned int port) {
    char cmd[32];
    char port_str[6];
    int  digits = 0, divisor = 10000;
    while (divisor > 1 && (port / divisor) == 0) divisor /= 10;
    while (divisor >= 1) { port_str[digits++] = '0' + (port / divisor) % 10; divisor /= 10; }
    port_str[digits] = '\0';
    Send_IOT_Command("AT+CIPMUX=1");
    strcpy(cmd, "AT+CIPSERVER=1,");
    strcat(cmd, port_str);
    Send_IOT_Command(cmd);
}

// ------------------------------------------------------------------------------
//  IOT_Ping
//  Sends an ICMP ping to 8.8.8.8 (Google DNS) using AT+PING.
//
//  AT+PING does NOT open or touch any TCP connections, so the iPad client
//  connection (or any other CIPMUX slot) is completely unaffected.
//  The ESP32 responds with "+PING:X" (ms) on success or "-1" on failure.
//
//  If ping fails, iot_connected is cleared so IOT_Check_Reconnect will
//  attempt a WiFi reconnect on the next check interval.
//
//  Called periodically from the main loop when iot_elapsed >= iot_next_ping_time.
// ------------------------------------------------------------------------------
void IOT_Ping(void) {
    // Use IP address instead of hostname to avoid DNS lookup delays
    Send_IOT_Command("AT+PING=\"8.8.8.8\"");
    IOT_Wait(30);   // 300ms - enough for ICMP round-trip on local network

    // Schedule next ping regardless of result
    iot_next_ping_time = iot_elapsed + IOT_PING_INTERVAL;

    // Check parsed response: IOT_Parse_Response sets iot_connected=0 on
    // "+PING:-1" (unreachable), which triggers IOT_Check_Reconnect to re-init.
}

// ------------------------------------------------------------------------------
//  IOT_Check_Reconnect
//  Monitors WiFi connectivity via the +CWJAP? query response.
//  If iot_connected has been cleared (e.g., by "+CWJAP:" not being parsed after
//  a disconnect), and the reconnect delay has elapsed, re-runs IOT_Init_Sequence
//  to restore the WiFi connection and TCP server automatically.
//  Called every main loop iteration from Serial_Process or main.
// ------------------------------------------------------------------------------
void IOT_Check_Reconnect(void) {
    // Only attempt reconnect if disconnected and delay has elapsed
    if (iot_connected) return;
    if (iot_elapsed < iot_reconnect_at) return;

    // Update display to show reconnect attempt
    strcpy(display_line[0], "WiFi Lost ");
    strcpy(display_line[1], "Reconnect.");
    strcpy(display_line[2], "          ");
    strcpy(display_line[3], "          ");
    display_changed = TRUE;
    update_display  = TRUE;
    Display_Process();

    // Re-run full init sequence (AT, CWJAP?, CIFSR, CIPMUX, CIPSERVER)
    IOT_Init_Sequence();

    // If init succeeded, iot_display_ready will be set by IOT_Parse_Response.
    // Schedule the next reconnect check in case it fails again.
    iot_reconnect_at = iot_elapsed + IOT_RECONNECT_DELAY;
}

// ------------------------------------------------------------------------------
//  IOT_Emergency_Stop
//  Immediately halts all motor output and clears every pending motion command.
//  Triggered by the ^E command from TCP or serial. Safe to call from anywhere.
// ------------------------------------------------------------------------------
void IOT_Emergency_Stop(void) {
    Wheels_Off();
    iot_estop           = 1;
    iot_motion_pending  = 0;
    iot_motion_pending2 = 0;
    // Block any further motion for 1 second to absorb queued TCP packets
    iot_motor_cooldown_until = iot_elapsed + TIMER_B0_ONESEC;
    Reset_IOT_Wheels();

    strcpy(display_line[0], "          ");
    strcpy(display_line[1], " E  STOP  ");
    strcpy(display_line[2], "          ");
    strcpy(display_line[3], "          ");
    display_changed = TRUE;
    update_display  = TRUE;
    Display_Process();
}

// ------------------------------------------------------------------------------
//  Demo_Update_Seconds
//  Recalculates demo_seconds from iot_elapsed since demo_start_time.
//  Call every main loop iteration once demo_started is set.
//  TB0 ticks at 10 ms each, so 100 ticks = 1 second.
// ------------------------------------------------------------------------------
void Demo_Update_Seconds(void) {
    if (!demo_started) return;
    demo_seconds = (unsigned int)((iot_elapsed - demo_start_time) / TIMER_B0_ONESEC);
}

// ------------------------------------------------------------------------------
//  Demo_Set_Arrived
//  Formats "Arrived 0X" into demo_arrived for the demo IOT display.
// ------------------------------------------------------------------------------
void Demo_Set_Arrived(unsigned char pad_num) {
    strcpy(demo_arrived, "Arrived 0 ");
    demo_arrived[9] = '0' + (pad_num % 10);
    demo_arrived[10] = '\0';
}

// ------------------------------------------------------------------------------
//  Demo_Clear_Arrived
//  Blanks the arrived line when the car is not on a numbered pad.
// ------------------------------------------------------------------------------
void Demo_Clear_Arrived(void) {
    strcpy(demo_arrived, "          ");
}

// ------------------------------------------------------------------------------
//  Demo_Set_Last_Cmd
//  Builds the last-command display string: "D TTTT" where D=direction, TTTT=ms.
//  Stored in demo_last_cmd for the demo IOT display (line 4).
// ------------------------------------------------------------------------------
void Demo_Set_Last_Cmd(char dir, unsigned int ms) {
    demo_last_cmd[0] = dir;
    demo_last_cmd[1] = ' ';
    demo_last_cmd[2] = '0' + (ms / 1000) % 10;
    demo_last_cmd[3] = '0' + (ms / 100)  % 10;
    demo_last_cmd[4] = '0' + (ms / 10)   % 10;
    demo_last_cmd[5] = '0' + (ms)        % 10;
    demo_last_cmd[6] = ' ';
    // Append seconds counter (up to 3 digits) then 's'
    demo_last_cmd[7]  = '0' + (demo_seconds / 100) % 10;
    demo_last_cmd[8]  = '0' + (demo_seconds / 10)  % 10;
    demo_last_cmd[9]  = '0' + (demo_seconds)       % 10;
    demo_last_cmd[10] = '\0';
}

// ------------------------------------------------------------------------------
//  Center_String
//  Write a null-terminated string centered into a 10-char display_line row.
//  Pads with spaces on both sides.
// ------------------------------------------------------------------------------
static void Center_String(char *src, char *dest) {
    int len = (int)strlen(src);
    int i, pad;
    if (len > 10) len = 10;
    pad = (10 - len) / 2;
    for (i = 0; i < 10; i++) dest[i] = ' ';
    for (i = 0; i < len; i++) dest[pad + i] = src[i];
    dest[10] = '\0';
}

// ------------------------------------------------------------------------------
//  IOT_Parse_Response
//  Called every main loop iteration from Serial_Process().
//  Parses current_message for known IOT response patterns and updates the
//  LCD / IOT menu display and triggers motion state machine as needed.
//
//  Patterns handled:
//    +CWJAP:"ssid",...        -> center SSID on line 1 of IOT menu
//    +CIFSR:STAIP,"x.x.x.x"  -> parse IP, split across lines 2-4
//    +IPD,N,NN:<payload>      -> display payload, dispatch motion command
// ------------------------------------------------------------------------------
void IOT_Parse_Response(void) {
    if (!display_changed) return;

    char *line = current_message;
    char *p;
    char  extract[32];
    int   i;

    // --- SSID (from AT+CWJAP? response) ---
    // Format: +CWJAP:"ssid","bssid",channel,rssi
    p = strstr(line, "+CWJAP:\"");
    if (p) {
        p += 8;
        i = 0;
        while (*p && *p != '"' && i < 10) extract[i++] = *p++;
        extract[i] = '\0';
        Center_String(extract, iot_ssid);
        iot_connected = 1;  // SSID confirmed - WiFi is up
        // Clear current_message so we don't re-parse this line on the next iteration.
        // iot_display_ready stays 0 until IP also arrives.
        current_message[0] = '\0';
        display_changed = TRUE;
        return;
    }

    // --- Not connected response ---
    if (strstr(line, "No AP")) {
        Center_String("No WiFi", iot_ssid);
        Center_String("Not Conn.", iot_ip_line3);
        Center_String("Use ^W", iot_ip_line4);
        iot_connected     = 0;  // WiFi is down - trigger reconnect logic
        iot_reconnect_at  = iot_elapsed + IOT_RECONNECT_DELAY;
        iot_display_ready = 1;
        current_message[0] = '\0';
        display_changed = TRUE;
        return;
    }

    // --- IP address ---
    // Format: +CIFSR:STAIP,"a.b.c.d"
    // SSID may have already been parsed from a previous line - check both fields.
    p = strstr(line, "+CIFSR:STAIP,\"");
    if (p) {
        char ip[16];
        int  g, gi;
        char groups[4][8];

        p += 14;
        i = 0;
        while (*p && *p != '"' && i < 15) ip[i++] = *p++;
        ip[i] = '\0';

        // Parse four dot-separated groups
        gi = 0;
        for (g = 0; g < 4; g++) {
            int len = 0;
            while (ip[gi] && ip[gi] != '.') groups[g][len++] = ip[gi++];
            groups[g][len] = '\0';
            if (ip[gi] == '.') gi++;
        }

        // Line 2: "a.b" centered
        strcpy(extract, groups[0]); strcat(extract, "."); strcat(extract, groups[1]);
        Center_String(extract, iot_ip_line3);

        // Line 3: "c.d" centered
        strcpy(extract, groups[2]); strcat(extract, "."); strcat(extract, groups[3]);
        Center_String(extract, iot_ip_line4);

        strcpy(iot_ip_label, "IP Address");

        // If SSID wasn't received yet (not connected at boot), fill a placeholder
        if (iot_ssid[0] == ' ' && iot_ssid[1] == ' ') {
            Center_String("Connected", iot_ssid);
        }

        iot_display_ready = 1;
        current_message[0] = '\0';
        display_changed = TRUE;
        return;
    }

    // --- Incoming TCP data ---
    // Format: +IPD,<conn>,<len>:<payload>
    p = strstr(line, "+IPD,");
    if (p) {
        while (*p && *p != ':') p++;
        if (*p == ':') {
            p++;
            i = 0;
            while (*p && *p != '\r' && *p != '\n' && i < 31) extract[i++] = *p++;
            extract[i] = '\0';

            // Show on enlarged middle line
//            lcd_BIG_mid();
            strcpy(display_line[1], "          ");
            strncpy(display_line[1], extract, 10);
            display_line[1][10] = '\0';
            display_changed = TRUE;

            // Dispatch motion command(s); support two chained with ';'
            // e.g. "^1234F1000;^1234B1000"
            if (extract[0] == CMD_START_CHAR) {
                char *semi = strchr(extract, ';');
                if (semi) {
                    *semi = '\0';                      // split string at ';'
                    IOT_Command_Dispatch(extract + 1); // first cmd -> slot 1
                    // Queue second command into slot 2 directly
                    // (skip leading '^' if present)
                    char *cmd2 = semi + 1;
                    if (*cmd2 == CMD_START_CHAR) cmd2++;
                    // cmd2 is now e.g. "0630B1000" - parse PIN+DIR+TIME
                    // Use the same SECRET_PIN as the outer motion command check
                    if ((int)strlen(cmd2) >= 6 && strncmp(cmd2, SECRET_PIN, 4) == 0) {
                        unsigned int t2 = 0;
                        int jj = 5;
                        while (cmd2[jj] >= '0' && cmd2[jj] <= '9')
                            t2 = t2 * 10 + (cmd2[jj++] - '0');
                        iot_motion_dir2      = cmd2[4];
                        iot_motion_duration2 = t2;
                        iot_motion_pending2  = TRUE;
                    }
                    else if (strcmp(cmd2, "B") == 0) {
                        bl_pending = TRUE;
                        Reset_Driveup();
                        Backlite_On();
                    }
                } else {
                    IOT_Command_Dispatch(extract + 1);
                }
            }
        }
        current_message[0] = '\0';
        return;
    }

    // --- AT+PING response ---
    // Success: "+PING:XX"  (XX = round-trip ms)  -> WiFi confirmed up
    // Failure: "+PING:-1"                         -> WiFi unreachable, trigger reconnect
    if (strstr(line, "+PING:")) {
        if (strstr(line, "+PING:-1")) {
            // Ping failed - mark disconnected so IOT_Check_Reconnect re-inits
            iot_connected    = 0;
            iot_reconnect_at = iot_elapsed + IOT_RECONNECT_DELAY;
        } else {
            // Ping succeeded - connection still alive
            iot_connected = 1;
        }
        current_message[0] = '\0';
        return;
    }
}

// ------------------------------------------------------------------------------
//  IOT_Command_Dispatch
//  Receives the string AFTER the '^'.
//  Single-letter commands: ^ ^ F S I G W
//  Motion commands: <PIN><DIR><TIME>   e.g. "1234F0200"
// ------------------------------------------------------------------------------
void IOT_Command_Dispatch(char *cmd) {

    if (!cmd || cmd[0] == '\0') return;

    // ^^ - connection check
    if (cmd[0] == CMD_START_CHAR) {
        USCI_A1_transmit("I'm here\r\n");
        strcpy(display_line[0], "IOT Check ");
        strcpy(display_line[1], " I'm here ");
        display_changed = TRUE;
        return;
    }

    // ^F - fast baud
    if (cmd[0] == 'F' && cmd[1] == '\0') {
        Set_Baud_Rate(BAUD_115200);
        USCI_A1_transmit("115,200\r\n");
        strcpy(display_line[2], " 115,200  ");
        display_changed = TRUE;
        return;
    }

    // ^S - slow baud
    if (cmd[0] == 'S' && cmd[1] == '\0') {
        Set_Baud_Rate(BAUD_9600);
        USCI_A1_transmit("9,600\r\n");
        strcpy(display_line[2], "  9,600   ");
        display_changed = TRUE;
        return;
    }

    // ^I - init IOT
    if (cmd[0] == 'I' && cmd[1] == '\0') { IOT_Init_Sequence(); return; }

    // ^G - get IP
    if (cmd[0] == 'G' && cmd[1] == '\0') { Send_IOT_Command("AT+CIFSR"); return; }

    // ^E - emergency stop (halts all motion immediately, including blackline SM)
    if (cmd[0] == IOT_ESTOP_CMD && cmd[1] == '\0') {
        IOT_Emergency_Stop();
        Blackline_Emergency_Exit();  // halts blackline SM
        Reset_Driveup();             // halts driveup SM
        return;
    }

    // ^A<N> - mark arrived at pad N (e.g. ^A3 -> "Arrived 03" on display line 1)
    // Per spec: display "Arrived 0X" where X is the pad number 1-8
    if (cmd[0] == 'A' && cmd[1] >= '0' && cmd[1] <= '8') {
        bl_status = TRUE;           // flash the backlite for 10 seconds
        current_menu = MENU_IOT;    // switch current menu to IOT (in case remote reboot is necessary)
        Demo_Set_Phase(2);          // set the menu to go into WiFi/Black Line mode
        menu_en      = TRUE;

        unsigned char pad = (unsigned char)(cmd[1] - '0');
        Demo_Set_Arrived(pad);
        // Also show on the display immediately
        lcd_4line();
        strncpy(display_line[0], demo_arrived, 10);
        display_line[0][10] = '\0';
        display_changed = TRUE;
        // Switch to demo IOT menu so the arrived line is visible
        if (current_menu != MENU_DEMO) {
            current_menu = MENU_DEMO;
        }
        return;
    }

    // ^C - clear arrived display (car is moving between pads)
    if (cmd[0] == 'C' && cmd[1] == '\0') {
        Demo_Clear_Arrived();
        display_changed = TRUE;
        return;
    }
    // ^BR - position for RIGHT track board (drive over pad 8, turn right, align, stop)
    // ^BL - position for LEFT  track board (drive over pad 8, turn left,  align, stop)
    if (cmd[0] == 'B' && (cmd[1] == 'R' || cmd[1] == 'L') && cmd[2] == '\0') {
        driveup_side  = cmd[1];     // 'R' or 'L'
        Driveup_State = 1;
        Driveup_Run   = 1;
        Time_Sequence = 0;
        // Ensure blackline SM is idle - it starts only after ^B
        Statemachine_Run   = 0;
        Statemachine_State = 0;
        Demo_Set_Phase(2);          // Switch display to BL phase so line 0 is free
        strcpy(display_line[0], cmd[1] == 'R' ? " Setup R  " : " Setup L  ");
        display_changed = TRUE;
        return;
    }

    // ^B - begin black line intercept from current position
    //  Car should already be positioned facing the track (by ^BR/^BL or manual moves).
    //  Drives forward until line detected, then runs full intercept/follow/exit.
    if (cmd[0] == 'B' && cmd[1] == '\0') {
        Reset_Driveup();            // stop positioning SM if still running
        Reset_IOT_Wheels();
        Backlite_On();
        if (!demo_started) {
            demo_started    = 1;
            demo_start_time = iot_elapsed;
        }
        Demo_Set_Phase(2);
        Time_Sequence      = 0;
        Statemachine_State = 'a';     // skip drive-up, go straight to intercept
        Statemachine_Run   = 1;
        BL_Sequence        = 0;
        Backlite_On();
        return;
    }
    if (cmd[0] == 'X' && cmd[1] == '\0') {
        Blackline_Exit_Command();
        strcpy(display_line[0], " BL Exit  ");
        display_changed = TRUE;
        return;
    }

    // ^R - remote software reboot (triggers watchdog reset)
    if (cmd[0] == 'R' && cmd[1] == '\0') {
        USCI_A1_transmit("Rebooting\r\n");
        lcd_BIG_mid();
        strcpy(display_line[0], "IOT Remote");
        strcpy(display_line[1], " Rebooting");
        strcpy(display_line[2], "  System  ");
        strcpy(display_line[3], "          ");
        display_changed = TRUE;
        update_display  = TRUE;
        Display_Process();

        IOT_Wait(100);
        // Unlock and trigger watchdog reset immediately
        WDTCTL = WDTPW | WDTCNTCL | WDTSSEL__ACLK | WDTIS__64;
        while(1);   // wait for WDT to fire (< 2ms)
        return;     // never reached
    }

    // ^W<ssid>,<pw> - connect WiFi
    if (cmd[0] == 'W') {
        char ssid[32] = {0}, pw[32] = {0};
        char *comma = strchr(cmd + 1, ',');
        if (comma) {
            int slen = (int)(comma - (cmd + 1));
            if (slen > 0 && slen < 32) {
                strncpy(ssid, cmd + 1, slen); ssid[slen] = '\0';
                strncpy(pw, comma + 1, 31);   pw[31] = '\0';
                IOT_Connect_WiFi(ssid, pw);
            }
        }
        return;
    }

    // Motion command: <PIN><DIR><TIME>  (min 6 chars: 4 PIN + 1 DIR + 1 digit)
    if ((int)strlen(cmd) >= 6 && strncmp(cmd, SECRET_PIN, 4) == 0) {
        unsigned int time_ms = 0;
        int j = 5;
        while (cmd[j] >= '0' && cmd[j] <= '9') time_ms = time_ms * 10 + (cmd[j++] - '0');

        // Pass direction and duration to the IOT motion state machine
        iot_motion_dir      = cmd[4];
        iot_motion_duration = time_ms;
        iot_motion_pending  = 1;
        iot_estop           = 0;  // clear any previous e-stop on new valid command

        // Start the seconds counter on the very first motion command received
        if (!demo_started) {
            demo_started    = 1;
            demo_start_time = iot_elapsed;
        }
        // Update last-command display for demo day line 4
        Demo_Set_Last_Cmd(cmd[4], time_ms);
        // Hold the command display for 2 seconds (200 TB0 ticks x 10ms)
        iot_cmd_display_until = iot_elapsed + 200;   // use free-running counter, not Time_Sequence

        // Show on enlarged middle line - blank all lines first to clear stale content
//        lcd_BIG_mid();
        strcpy(display_line[0], "          ");
        strcpy(display_line[1], "          ");
        strcpy(display_line[2], "          ");
        strcpy(display_line[3], "          ");
        display_line[3][0] = cmd[4];
        display_line[1][1] = '0' + (time_ms / 1000) % 10;
        display_line[1][2] = '0' + (time_ms / 100)  % 10;
        display_line[1][3] = '0' + (time_ms / 10)   % 10;
        display_line[1][4] = '0' + (time_ms)        % 10;
        // positions 6-9 already set to spaces by strcpy above
        display_changed = TRUE;
        return;
    }

    // Bad PIN or unknown command
    if ((int)strlen(cmd) >= 6) {
        strcpy(display_line[0], " Bad PIN  ");
    } else {
        strcpy(display_line[0], " Unk CMD  ");
        strncpy(display_line[1], cmd, 10);
        display_line[1][10] = '\0';
    }
    display_changed = TRUE;
}
