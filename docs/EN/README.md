# 2D Plotter — documentation

**Language / Idioma:** [English](README.md) | [Português](../PT/README.md)

This project simulates an X/Y Cartesian plotter with a servo-positioned pen.
An Arduino Uno coordinates two stepper motors, the operator interface, speed
indicators, and two auxiliary outputs.

**Simulation:** <https://wokwi.com/projects/472168224663835649>

## Functional overview

| Subsystem | Implementation |
|---|---|
| X/Y motion | 2 stepper motors, 2 A4988 drivers, and `AccelStepper` |
| Manual control | Analog joystick on A0/A1 |
| Z pen | Servo on D9; high, middle, and low states |
| Telemetry | 128×64 SSD1306 OLED over I2C |
| Speed | 2 ten-segment bars through 3 × 74HC595 |
| Safety controls | Logical HOME on D11 and latched E-STOP on D12 |
| Auxiliary outputs | Suction and fan relays on A2/A3 |

## Documents

- [`hardware-reference.md`](hardware-reference.md): pin map, power, electrical
  constraints, and physical-build guidance.
- [`component-specifications.md`](component-specifications.md): component
  inventory and Wokwi identifiers.
- [`technical-specification.md`](technical-specification.md): requirements,
  architecture, algorithms, states, and verification criteria.

## Operation

- Move the joystick to command X/Y speed.
- Press the joystick selector to cycle `HIGH → MIDDLE → LOW → HIGH`.
- Press HOME (`H`) to raise the pen and return to logical origin `(0,0)`.
- Press E-STOP (`E`) to disable the drivers and auxiliary outputs. This state
  is latched and requires a reset.

## Model limitations

The initial position is assumed to be `(0,0)`, and travel boundaries are
software limits. HOME does not perform mechanical referencing. The Wokwi
project is a functional demonstration, not a substitute for the electrical
and mechanical protections required on a real machine.
