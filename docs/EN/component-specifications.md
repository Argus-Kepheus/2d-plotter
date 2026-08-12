# Component specifications — 2D Plotter

This inventory corresponds to `diagram.json`. `wokwi-text` objects are visual
labels and are not functional components.

| Qty. | Wokwi component | IDs | Purpose |
|---:|---|---|---|
| 1 | `wokwi-arduino-uno` | `uno` | main controller |
| 2 | `wokwi-stepper-motor` | `motX`, `motY` | Cartesian motion |
| 2 | `wokwi-a4988` | `drvX`, `drvY` | STEP/DIR drivers |
| 3 | `wokwi-74hc595` | `sr0`, `sr1`, `sr2` | bar expansion |
| 2 | `wokwi-led-bar-graph` | `barX`, `barY` | ten-segment X/Y speed |
| 1 | `wokwi-analog-joystick` | `joy1` | X/Y and Z selection |
| 1 | `wokwi-servo` | `servoZ` | pen height |
| 1 | `board-ssd1306` | `oled1` | position, Z, and output state |
| 2 | `wokwi-pushbutton` | `btnHome`, `btnEStop` | HOME and E-STOP |
| 2 | `wokwi-relay-module` | `relaySuction`, `relayFan` | auxiliary loads |
| 2 | `wokwi-led` | `loadSuction`, `loadFan` | visual load models |
| 2 | `wokwi-resistor` | `rSuction`, `rFan` | 220 Ω load LED resistors |

## Libraries

| Library | Use |
|---|---|
| AccelStepper | axis speed and acceleration profiles |
| Adafruit GFX Library | OLED graphics primitives |
| Adafruit SSD1306 | I2C display driver |
| Servo | Z-axis position signal |

## Firmware parameters

| Item | Value |
|---|---:|
| Maximum speed | 800 steps/s |
| Acceleration | 450 steps/s² |
| HOME speed | 500 steps/s |
| Simulated motor | 200 steps/revolution |
| Bar refresh | 30 ms |
| OLED refresh | 80 ms |
| Fan hold time | 2000 ms |
| Debounce | 35 ms |

The Wokwi relay modules use `transistor="pnp"`, so HIGH represents an active
load in this project. Physical modules may be active LOW and must be
characterized before integration. Verify that an actual SSD1306 module accepts
the Uno supply and logic levels.
