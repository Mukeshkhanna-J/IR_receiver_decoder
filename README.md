# IR Receiver and Decoder
IR Remote Receiver using 8051 Microcontroller
This project implements an IR (Infrared) remote control receiver using the 8051 microcontroller. It captures and decodes 32-bit IR signals (NEC protocol) through external interrupts and stores them in memory. Key features include:

IR Signal Decoding: Detects and decodes 32-bit IR remote signals using INT0 (P3.2).
Code Storage: Stores up to 5 unique IR codes in a circular buffer.
Push-Button Trigger: Displays all stored codes via UART when a button on P1.0 is pressed.
LED Indication: Toggles an LED on P1.2 when a valid IR code is received.
UART Communication: Sends decoded and stored IR codes over UART at 9600 baud.

Hardware Requirements:
8051 Microcontroller (AT89C51 or compatible)
TSOP IR Receiver Module (connected to INT0 / P3.2)
Push Button (connected to P1.0)
LED (connected to P1.2)
UART interface for serial monitoring (baud rate: 9600)

Connections:
| Component              | 8051 Pin      | Description                                                        |
| ---------------------- | ------------- | ------------------------------------------------------------------ |
| **IR Receiver (TSOP)** | `P3.2 / INT0` | Connect the OUT pin of TSOP to INT0 (P3.2)                         |
| **Push Button**        | `P1.0`        | One end to P1.0, other end to GND (use pull-up resistor if needed) |
| **LED**                | `P1.2`        | Connect with series resistor (\~220Ω) to GND                       |
| **UART TX**            | `P3.1 (TXD)`  | Connect to RX of USB-to-Serial module or PC                        |
| **UART RX**            | `P3.0 (RXD)`  | Connect to TX of USB-to-Serial module or PC                        |
| **Crystal Oscillator** | `XTAL1/XTAL2` | 11.0592 MHz with two 33pF capacitors to GND                        |
| **Reset Circuit**      | `RST`         | 10μF capacitor to GND and 10kΩ pull-up resistor to Vcc             |
| **Power Supply**       | `Vcc / GND`   | 5V regulated power to microcontroller and components               |


Usage:
Upload the hex code to the 8051 microcontroller.
Point a compatible IR remote at the receiver and press a button.
The received code is shown via UART and stored.
