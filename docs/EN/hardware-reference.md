# Hardware reference — 2D Plotter

## 1. Controller

The target is an **Arduino Uno R3**, based on the ATmega328P and 5 V logic.
D0/D1 remain available for UART, while A4/A5 are reserved for the OLED I2C
bus.

## 2. Pin map

| Function | Pin | Direction | Relevant state |
|---|---:|---|---|
| X STEP / DIR | D2 / D3 | output | X A4988 pulse and direction |
| Y STEP / DIR | D4 / D5 | output | Y A4988 pulse and direction |
| 74HC595 DATA / CLOCK / LATCH | D6 / D7 / D8 | output | three cascaded registers |
| Pen servo | D9 | PWM output | 180°, 90°, or 0° |
| Joystick selector | D10 | input | `INPUT_PULLUP`, active LOW |
| HOME | D11 | input | `INPUT_PULLUP`, active LOW |
| E-STOP | D12 | input | `INPUT_PULLUP`, active LOW |
| Shared A4988 ENABLE | D13 | output | active LOW |
| Joystick X / Y | A0 / A1 | analog input | center 512; ±70 dead zone |
| Suction / fan relay | A2 / A3 | digital output | HIGH drives the Wokwi model |
| OLED SDA / SCL | A4 / A5 | I2C | address `0x3C` |

## 3. Motion hardware

Each axis uses one A4988 in STEP/DIR mode and one simulated 200-step/revolution
motor. Firmware limits are 800 steps/s and 450 steps/s². Logical travel is X
`0..4000` and Y `0..2400` steps.

RESET and SLEEP are tied together for each driver. In a physical build, keep
them high during operation. `VMOT` is omitted only because it is not involved
in the simulation: actual drivers require a suitable motor supply, local bulk
decoupling, and current-limit adjustment before connecting motors.

## 4. Speed bars

Three cascaded 74HC595 devices provide 24 outputs; 20 drive the X/Y bars.
CLOCK and LATCH are shared and Q7S carries the serial stream forward. The
farthest register byte is shifted first.

The simulation omits series resistors from the 20 bar segments for visual
clarity. A physical implementation requires one resistor per segment and must
respect per-pin and package current limits. Use current drivers when needed.

## 5. Power and physical safety

- Join logic, driver, motor-supply, and peripheral grounds.
- Never power the motors from the Uno 5 V pin.
- Add local `VMOT` bulk decoupling to each A4988.
- Set driver current according to motor, driver, and cooling requirements.
- Add physical HOME/end-stop sensors for real referencing.
- Implement a hardwired E-STOP that removes actuator power independently of
  firmware.
- Ensure safe pen and load states at power-up and after faults.

## 6. Bring-up checklist

Verify ground continuity and supply polarity; identify both motor coil pairs;
set the driver current; test ENABLE and E-STOP without a tool installed; test
HOME and limits at low speed; fit all LED resistors; validate auxiliary loads;
then release full-area travel.
