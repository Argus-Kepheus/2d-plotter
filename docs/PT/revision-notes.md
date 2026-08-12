# Registro da revisão de pinagem e arquitetura

Este documento preserva as decisões que acompanhavam o pacote original.

## Pinagem consolidada

| Função | Pino |
|---|---:|
| X STEP / DIR | D2 / D3 |
| Y STEP / DIR | D4 / D5 |
| 74HC595 DATA / CLOCK / LATCH | D6 / D7 / D8 |
| Servo da caneta | D9 |
| Joystick SEL | D10 |
| HOME / E-STOP | D11 / D12 |
| ENABLE comum dos A4988 | D13 |
| Joystick X / Y | A0 / A1 |
| Relé de sucção / ventilador | A2 / A3 |
| OLED SDA / SCL | A4 / A5 |
| Serial | D0 / D1 livres |

## Decisões registradas

- três 74HC595 comandam os 20 segmentos das barras usando D6–D8;
- as barras GYR indicam a velocidade calculada por `AccelStepper::speed()`;
- D13 controla os dois ENABLE; E-STOP os desabilita e permanece travado;
- as bobinas seguem a nomenclatura A/B dos motores e A4988 do Wokwi;
- `VMOT` é omitido apenas na simulação e exige fonte própria no hardware;
- o OLED representa a mesa e suas bordas são limites virtuais;
- limites: X `0..4000`, Y `0..2400` passos;
- Z percorre alta, meia e baixa em 180°, 90° e 0°;
- LEDs após os relés representam sucção e ventilação;
- sucção permanece ligada durante a sessão e desliga em HOME;
- ventilação acompanha movimento/HOME e retém por 2 s.

## Ressalvas

1. HOME é retorno à origem lógica, não calibração por sensor.
2. Todos os segmentos LED precisam de limitação de corrente no hardware.
3. As cargas dos relés são apenas representações visuais.
4. Atalhos Wokwi: `H` para HOME e `E` para E-STOP.
