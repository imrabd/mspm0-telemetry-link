# MSPM0 Point-to-Point Sensor Telemetry Link

A two-board telemetry link on MSPM0G3507 microcontrollers (ARM Cortex-M0+,
80 MHz), written bare-metal with no RTOS. One board samples a position sensor
and transmits readings over UART; the second receives them and renders the
value on an ST7735 LCD.

## Engineering notes

**Framed message protocol.** Readings are sent as a sync byte (`0x3C`)
followed by three ASCII digits. The sync byte is the point: a receiver that
powers up mid-stream, or drops bytes under load, resynchronizes on the next
frame boundary instead of silently decoding misaligned data as valid readings.

**Interrupt-driven UART over a circular FIFO.** Transmit and receive both run
from interrupts, with a circular FIFO decoupling the ISR from the display
loop. An LCD write over SPI takes far longer than the gap between incoming
bytes, so a polled receiver drops data under load; the FIFO absorbs bursts
while the main loop drains them at its own pace.

**Fixed-point conversion instead of floating point.** Raw 12-bit ADC counts
convert to centimeters with integer arithmetic. The Cortex-M0+ has no
hardware FPU, so I measured the software-emulation cost and found it too slow
to fit inside the sampling period. Integer math keeps the conversion well
within budget.

## Layout

| Path | Contents |
|---|---|
| `telemetry_main.c` | Transmitter and receiver main loops, protocol framing, ADC sampling |
| `UART1.c` | UART1 driver, interrupt-driven transmit path |
| `UART2.c` | UART2 driver, interrupt-driven receive path |

## Building

Targets the MSPM0G3507 LaunchPad, built with TI Code Composer Studio.

Depends on the MSPM0 peripheral driver set — `ST7735`, `Clock`, `LaunchPad`,
`Timer`, `ADC1`, `TExaS`, `FIFO1`, and the `UART1.h` / `UART2.h` interface
headers — which are **not redistributed here**. Supply them from an MSPM0
ValvanoWare distribution, along with the CCS project files and the
MSPM0G3507 linker command file, then drop these sources in alongside.

## Attribution

The protocol design, FIFO and interrupt structure, driver implementations, and
fixed-point conversion here are my own work. The underlying peripheral drivers
and project scaffolding are by Jonathan Valvano and are not included in this
repository.
