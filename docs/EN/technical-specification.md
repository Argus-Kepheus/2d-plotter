# Technical specification — 2D Plotter

## 1. Document control

| Field | Value |
|---|---|
| Project | Two-axis Cartesian plotter controller |
| Target | Arduino Uno R3 / ATmega328P |
| Language | Arduino C++ |
| Simulation | Wokwi |
| Firmware | `sketch.ino` |
| Circuit | `diagram.json` |
| Dependencies | `libraries.txt` |

## 2. Scope

The system manually controls an X/Y plotter, places the pen at three heights,
and reports position and state on an OLED. It also models speed indication,
table suction, and cooling. This scope covers simulated control logic; it does
not qualify a physical machine for safe operation.

## 3. Functional requirements

1. Convert both joystick axes into X/Y speed commands.
2. Accelerate and decelerate without blocking calls.
3. Restrict X to `0..4000` and Y to `0..2400` steps.
4. Cycle Z through `HIGH → MIDDLE → LOW → HIGH`.
5. Raise Z and return both axes to logical origin on HOME.
6. Show position, Z, load state, and virtual table on the OLED.
7. Display actual axis speed on two ten-segment bars.
8. Keep suction active during a work session and clear it on HOME.
9. Run the fan during motion/HOME and for 2 s after motion stops.
10. On E-STOP, disable drivers and loads, latch the fault, and require reset.

## 4. Cooperative architecture

The firmware uses a non-blocking superloop. Urgent safety checks run first;
motion generation runs on every iteration; user interfaces have independent
refresh periods:

```text
inputs -> safety -> HOME/Z states -> X/Y commands -> AccelStepper.run()
              \-> driver/load state        \-> bars (30 ms), OLED (80 ms)
```

No `delay()` is used because `AccelStepper::run()` must be called frequently.

## 5. X/Y control

The ADC range is `0..1023`, centered at 512 with a ±70 dead zone. Values
outside the dead zone are normalized to ±800 steps/s. A nonzero command moves
toward the corresponding software boundary. Returning to center invokes
`stop()` and clamps its deceleration target to the valid interval.

HOME raises the pen, clears the suction work session, and moves both axes to
zero at up to 500 steps/s. Completion is declared only after both distances
and speeds reach zero. This is a logical return, not mechanical homing; it
cannot detect lost steps or establish a physical reference after power-up.

## 6. Z, telemetry, and auxiliary outputs

The debounced joystick selector maps Z to 180°, 90°, and 0°. Selection is
blocked during HOME. The OLED at address `0x3C` refreshes every 80 ms and maps
X/Y position into a table rectangle. Each speed bar is derived from the actual
`AccelStepper::speed()` result and refreshed every 30 ms through three 74HC595
devices.

Manual motion or a Z change starts a work session. Suction follows that
session, except during HOME. The fan follows motion/HOME and uses a 2000 ms
off-delay. Relay-connected LEDs are only visual load models.

## 7. E-STOP behavior

E-STOP is checked first and without debounce. A LOW input latches emergency,
drives the shared active-low A4988 ENABLE high, clears both auxiliary outputs,
zeros the bars, and shows a persistent reset-required screen. Remaining loop
work is skipped.

This is control logic, not a safety-rated function. A physical machine needs
a hardwired stop circuit, suitable switching devices, guards, and a formal
risk assessment.

## 8. Verification

`tests/test_project.py` checks the JSON structure, expected component and
connection counts, critical wiring, pin constants, library declarations,
absence of `delay()`, and emergency-control ordering. Run it with:

```powershell
python -m unittest discover -s tests -v
```

Static verification does not replace compilation, simulation, or physical
testing. Wokwi acceptance should exercise all four motion directions, three Z
states, HOME, load timing, both limits, and E-STOP during motion.

## 9. Recommended evolution

- add physical home and end-stop switches;
- calibrate steps/mm and real work-area dimensions;
- add a queued trajectory or G-code parser;
- monitor driver faults and temperature;
- design power distribution, grounding, and EMC controls;
- replace demonstrator outputs with correctly rated interfaces;
- measure stop latency, repeatability, and lost-step behavior on a bench.

## 10. References

- [Wokwi project](https://wokwi.com/projects/472168224663835649)
- [Arduino Uno Rev3](https://docs.arduino.cc/hardware/uno-rev3/)
- [AccelStepper](https://www.airspayce.com/mikem/arduino/AccelStepper/)
- [Adafruit SSD1306](https://github.com/adafruit/Adafruit_SSD1306)
- [Wokwi diagram format](https://docs.wokwi.com/diagram-format)
