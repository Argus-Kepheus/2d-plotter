# Especificações dos componentes — 2D Plotter

O inventário abaixo corresponde a `diagram.json`. Elementos do tipo
`wokwi-text` são rótulos visuais e não entram na contagem funcional.

| Quantidade | Componente Wokwi | Identificadores | Função |
|---:|---|---|---|
| 1 | `wokwi-arduino-uno` | `uno` | controlador principal |
| 2 | `wokwi-stepper-motor` | `motX`, `motY` | movimento cartesiano |
| 2 | `wokwi-a4988` | `drvX`, `drvY` | drivers STEP/DIR |
| 3 | `wokwi-74hc595` | `sr0`, `sr1`, `sr2` | expansão das barras |
| 2 | `wokwi-led-bar-graph` | `barX`, `barY` | velocidade X/Y, 10 segmentos |
| 1 | `wokwi-analog-joystick` | `joy1` | comando X/Y e seleção Z |
| 1 | `wokwi-servo` | `servoZ` | altura da caneta |
| 1 | `board-ssd1306` | `oled1` | posição, Z e auxiliares |
| 2 | `wokwi-pushbutton` | `btnHome`, `btnEStop` | HOME e E-STOP |
| 2 | `wokwi-relay-module` | `relaySuction`, `relayFan` | cargas auxiliares |
| 2 | `wokwi-led` | `loadSuction`, `loadFan` | representação das cargas |
| 2 | `wokwi-resistor` | `rSuction`, `rFan` | 220 Ω para LEDs das cargas |

## Bibliotecas

| Biblioteca | Uso |
|---|---|
| AccelStepper | perfis de velocidade e aceleração dos eixos |
| Adafruit GFX Library | primitivas gráficas do OLED |
| Adafruit SSD1306 | controlador do display I2C |
| Servo | sinal de posição do eixo Z |

## Parâmetros funcionais

| Item | Valor no firmware |
|---|---:|
| Velocidade máxima | 800 passos/s |
| Aceleração | 450 passos/s² |
| Velocidade de HOME | 500 passos/s |
| Motor simulado | 200 passos/revolução |
| Atualização das barras | 30 ms |
| Atualização do OLED | 80 ms |
| Retenção do ventilador | 2000 ms |
| Antirrepique | 35 ms |

## Observações para componentes reais

Os módulos de relé do Wokwi estão configurados com `transistor="pnp"`; por
isso o firmware usa HIGH para representar a carga ligada. Módulos físicos
podem ser ativos em LOW e devem ser caracterizados antes da integração. O
SSD1306 físico deve aceitar alimentação e níveis lógicos compatíveis com o
Uno. A biblioteca `Servo` usa temporização do ATmega328P; alterações de pinos,
bibliotecas ou temporizadores exigem nova verificação de compatibilidade.
