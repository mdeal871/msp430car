# Autonomous WiFi/Black-Line RC Car — MSP430 Firmware

**Author:** Mason Deal
**Platform:** MSP430FR2355 (or similar FRAM-based MSP430), Code Composer Studio 12.8.1.00005
**Project:** Project 10 — Demo Day 🎉 *(Passed on the first try!)*

## Overview

This firmware runs an autonomous RC car that can be driven remotely over WiFi from a
TCP client (e.g. an iOS/Java app), positioned onto a physical track via scripted
positioning maneuvers, and then handed off to fully autonomous black-line-following
navigation — including line intercept, circular tracking, and a controlled exit.

The system runs on a cooperative **Background/Foreground ("While") operating
system**: a single `main()` while-loop repeatedly polls each subsystem's
process function, while hardware interrupts (timers, ADC, UART, switches)
capture events and set flags/values for the main loop to consume.

---

## Hardware / Peripherals Used

| Peripheral | Purpose |
|---|---|
| **ADC (12-bit, 3-channel)** | Thumbwheel (menu navigation), left/right line detectors |
| **Timer B0** | 10 ms system tick, switch debounce (one-shot CCR1/CCR2), ADC trigger |
| **Timer B3** | Motor PWM (4 channels: L/R forward, L/R reverse) + LCD backlight dimming |
| **eUSCI_A0** | UART to ESP32-WROOM IOT module (WiFi) |
| **eUSCI_A1** | UART back-channel to PC (Termite / serial terminal) |
| **eUSCI_B1** | SPI to the LCD display |
| **GPIO Ports 1–6** | Motor drivers, switches, LEDs, LCD control, IOT module control lines |

### Motor Control (Port 6 / Timer B3 PWM)
| Signal | Pin | PWM Channel |
|---|---|---|
| LCD Backlight | P6.0 | TB3.1 |
| Right Forward | P6.1 | TB3.2 |
| Left Forward | P6.2 | TB3.3 |
| Right Reverse | P6.3 | TB3.4 |
| Left Reverse | P6.4 | TB3.5 |

### Sensors (Port 1, ADC)
| Signal | Pin | ADC Channel |
|---|---|---|
| V_DETECT_L (left line sensor) | A2 | ADCINCH_2 |
| V_DETECT_R (right line sensor) | A3 | ADCINCH_3 |
| V_THUMB (thumbwheel) | A5 | ADCINCH_5 |

---

## Feature Summary

### 1. Background/Foreground Main Loop
`main.c` initializes every subsystem, displays a splash screen, then enters an
infinite loop that calls each subsystem's `*_Process()` / `Run_*()` function once
per iteration:

- `Display_Process()` — flushes changed display content to the LCD
- `Menu_Process()` — drives the on-device menu system
- `Serial_Process()` — drains UART ring buffers and parses IOT responses
- `ADC_Process()` — updates line-detector flags from the latest ADC readings
- `Run_Blackline()` — autonomous line-following state machine
- `Backlite_Flash()` — timed backlight flash effect
- `Run_IOT_Wheels()` / `Run_Driveup()` — remote motion and auto-positioning state machines (only while the black-line SM is idle)
- Periodic WiFi ping and auto-reconnect logic
- `TEST_PROBE` toggled every iteration for scope-based loop-timing debug

### 2. Menu System (`menus.c`)
An LCD-based menu navigated with the thumbwheel (ADC) and two push-button switches
(SW1 = select/action, SW2 = back):

- **Main Menu** — select between ADC Monitor, Serial Monitor, IOT status, and Demo Day
- **ADC Monitor** — live raw/normalized readings from both line detectors (SW1 toggles view)
- **Serial Monitor** — last received line from the PC and from the IOT module (SW1 clears buffers)
- **IOT Menu** — SSID, IP address, and a 2-second hold display of the last motion command received
- **Demo Day Menu** — a single, phase-driven menu covering the entire competition flow (see below)

### 3. Demo Day Menu — 3-Phase Competition Flow
A purpose-built menu that walks the car through the entire demo sequence:

- **Phase 0 — Calibration:** live left/right sensor readings while positioning over
  white and black surfaces; SW1 captures white, SW2 captures black, then auto-advances.
- **Phase 1 — WiFi Course Operation:** shows the last "Arrived at pad" status, IP
  address, last motion command, and an elapsed-time counter that starts on the first
  motion command received.
- **Phase 2 — Autonomous Black Line:** line 0 is handed over to the black-line state
  machine's status text (e.g. "BL Turn", "BL Circle"); remaining lines keep showing IP
  and a live timer.

The phase machine auto-detects transitions (e.g. jumps to Phase 2 the instant the
black-line state machine starts) and can also be force-set remotely (see `^A` below)
so a mid-course reboot doesn't strand the display on the splash/main screen.

### 4. WiFi / IOT Command Protocol (`IOT.c`, `serial_process.c`)
The car connects to WiFi through an ESP32-WROOM module driven with AT commands over
UART, and exposes a simple TCP server (default port **8080**) for remote control.

- **Auto-connect boot sequence** (`IOT_Init_Sequence`): resets the ESP32, verifies it's
  alive, queries the last-used WiFi network from flash, fetches the IP, enables
  multi-connection mode, and opens the TCP server — all fully automatic on power-up.
- **Periodic connectivity ping** (`IOT_Ping` / `IOT_Check_Reconnect`): pings a known
  host every `IOT_PING_INTERVAL` ticks; on failure, automatically re-runs the full
  init sequence to reconnect without any user intervention.
- **Command parsing**: incoming TCP data arrives as `+IPD,<conn>,<len>:<payload>`
  frames, parsed line-by-line in `IOT_Parse_Response()` as they stream in (rather than
  waiting for the whole buffer), which is critical since the ESP32 can burst multiple
  response lines back-to-back.
- **PIN-gated motion commands**: every motion command requires a 4-digit PIN
  (`SECRET_PIN`) prefix so it can't be triggered by unrelated network traffic.
- **Command chaining**: two commands can be sent on a single line separated by `;`
  (e.g. `^0630F0500;^0630R0090` or `^0630R0050;^B`) — the second command is queued
  in a slot-2 buffer and automatically loaded once the first finishes.

#### Full Command Reference

| Command | Description |
|---|---|
| `^^` | Connection check — replies `I'm here` to the PC |
| `^F` | Switch UART to 115,200 baud |
| `^S` | Switch UART to 9,600 baud |
| `^I` | Re-run the full IOT init/connect sequence |
| `^W<ssid>,<pw>` | Connect to a specific WiFi network |
| `^G` | Query and display current IP address |
| `^E` | **Emergency stop** — halts all motion, blackline SM, and drive-up SM immediately |
| `^A<N>` (N = 0–8) | Mark arrival at pad N; forces the LCD to the Demo Day menu (Phase 2) even from the splash screen |
| `^C` | Clear the "Arrived" display (car is between pads) |
| `^BR` / `^BL` | Auto-position onto the right/left track board from pad 8 |
| `^B` | Begin autonomous black-line intercept from the current position |
| `^X` | Trigger the black-line exit sequence early (TA override) |
| `^R` | Remote software reboot (watchdog reset) |
| `^<PIN><DIR><TIME>` | Timed motion command, e.g. `^0630F0500` = forward 500 ms. `DIR` = F/B/L/R |
| `<cmd1>;<cmd2>` | Chain two of the above on one line |

### 5. Remote Motion State Machine (`Run_IOT_Wheels`, `statemachines.c`)
A 3-state machine (idle → apply direction → wait for duration) executes one timed
motion command at a time, converts millisecond durations to 10 ms timer ticks,
supports a second chained command, and includes a short post-execution cooldown
window to reject duplicate/retransmitted TCP packets.

### 6. Auto-Positioning State Machine (`Run_Driveup`)
Triggered by `^BR` / `^BL`, this 5-state sequence autonomously drives the car from
pad 8 onto the correct track board without any further remote input: forward over
the pad, a 90° turn toward the chosen side, forward to the board entry point, a
second 90° turn to square up with the line, then a full stop awaiting `^B`.

### 7. Autonomous Black-Line Following (`Run_Blackline`, `Line_Follow`)
A full autonomous sequence with staged, spec-timed pauses between events for TA
visibility:

1. **BL Start** — slow forward creep until either line sensor detects black
2. **Intercept** — brief reverse brake pulse to stop cleanly on the line
3. **BL Turn** — spin in place until both sensors read solidly on the line, then a
   short counter-spin to kill momentum
4. **BL Travel / Follow** — closed-loop PD line-following controller (see below)
5. **BL Circle** — continues following through the full loop until an exit
   trigger (`^X`) or automatic exit condition
6. **BL Exit** — spins away from the circle and drives 2+ feet clear, then stops
   and displays the total elapsed run time

**PD Line-Follow Controller** (`Line_Follow`): normalizes both sensors against
calibrated white/black reference points, computes a proportional error term plus a
smoothed (averaged) derivative term, and differentially adjusts left/right motor
speeds to steer the car back onto the line. Includes a guardrail recovery mode that
forces a hard turn if both sensors momentarily lose the line.

### 8. Calibration
Two ADC calibration points (white surface / black surface) are captured per side via
the Demo Day menu's Phase 0, and used to normalize all line-detection and
line-following math to a 0–1000 scale regardless of ambient lighting conditions.

### 9. Serial Communication Architecture (`serial.c`, `serial_process.c`)
A dual-UART cross-link design bridges the PC and the IOT module:

```
PC  --> UCA1 RX --> PC_2_IOT ring (16B)  --> UCA0 TX --> IOT
IOT --> UCA0 RX --> IOT_2_PC ring (128B) --> UCA1 TX --> PC
```

- Both UARTs run interrupt-driven with ring buffers; a `pc_enabled` gate blocks
  transmission back to the PC until the PC has sent at least one character (per spec).
- FRAM-only commands (prefixed with `^`) are detected **inside the RX ISR** to close
  a race window, buffered for the main loop's command parser, and suppressed from
  being forwarded through to the IOT module.
- The main loop assembles received bytes into line-stage buffers, commits complete
  lines to flat, LCD-ready display buffers, and immediately parses IOT lines as they
  arrive (rather than after the whole burst drains) so fast multi-line AT responses
  (e.g. `+CWJAP:...` immediately followed by `OK`) are never overwritten before
  they're read.

### 10. Display System (`display.c`, LCD driver)
A dirty-flag driven display update system (`display_changed` / `update_display`)
avoids redundant SPI writes to the LCD, paired with a countdown-timeout backlight
that dims automatically after a period of inactivity, plus a one-shot "flash" effect
used to draw attention on pad arrival.

### 11. Switch Debounce (`interrupt_ports.c`, `interrupts_timers.c`)
Both push-buttons use a hardware-interrupt + one-shot-timer debounce scheme: a GPIO
edge interrupt fires once, immediately disables itself, and arms a ~1-second one-shot
Timer B0 compare interrupt that re-enables the GPIO interrupt only after the bounce
window has passed.

### 12. Safety
- **Emergency Stop (`^E`)** halts all motor output, clears every pending motion
  command (including chained slot-2 commands), stops both the black-line and
  drive-up state machines, and imposes a cooldown window to absorb any queued/
  in-flight TCP packets so a stopped car can't be accidentally re-triggered by stale
  traffic.
- **Remote reboot (`^R`)** issues a clean watchdog-triggered reset if the car needs
  to be recovered mid-course without physical access.

---

## File Structure

| File | Responsibility |
|---|---|
| `main.c` | Startup sequence and main Background/Foreground loop |
| `menus.c` | LCD menu system, including the 3-phase Demo Day menu |
| `statemachines.c` | Remote motion SM, drive-up positioning SM, black-line SM, PD line follower |
| `IOT.c` | AT command helpers, WiFi/TCP command protocol, response parsing |
| `serial.c` | UART (UCA0/UCA1) init and interrupt-driven ring buffer ISRs |
| `serial_process.c` | Main-loop serial line assembly and command dispatch |
| `adc.c` | ADC init, channel-cycling ISR, line-detection thresholding |
| `display.c` | Display refresh gating and backlight control |
| `init.c` | Startup initialization of ports, timers, and display state |
| `interrupt_ports.c` | Switch (SW1/SW2) debounce interrupt entry points |
| `interrupts_timers.c` | Timer B0/B3 ISRs — system tick, switch debounce timers, motor delay tick |
| `timers.c` | Timer B0 (system tick) and Timer B3 (motor/backlight PWM) configuration |
| `ports.c` / `ports.h` | GPIO pin configuration and pin-name macros for all six ports |
| `globals.c` / `globals.h` | All shared/extern global state |
| `macros.h` | Project-wide constants: timing values, thresholds, PWM speeds |
| `functions.h` | Master function prototype declarations |

---

## Build Environment

- **IDE:** Texas Instruments Code Composer Studio, version 12.8.1.00005
- **Target:** MSP430 (FRAM), configured with `PM5CTL0 &= ~LOCKLPM5` GPIO unlock on boot
- **Clock:** SMCLK-driven UART and PWM timers (8 MHz SMCLK referenced in serial baud tables)
