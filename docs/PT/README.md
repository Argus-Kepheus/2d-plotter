# 2D Plotter — documentação

**Language / Idioma:** [English](../EN/README.md) | [Português](README.md)

Este projeto simula uma plotter cartesiana X/Y com caneta posicionada por
servo. Um Arduino Uno coordena dois motores de passo, a interface do operador,
as indicações de velocidade e duas saídas auxiliares.

**Simulação:** <https://wokwi.com/projects/472168224663835649>

## Visão funcional

| Subsistema | Implementação |
|---|---|
| Movimento X/Y | 2 motores de passo, 2 drivers A4988 e `AccelStepper` |
| Comando manual | Joystick analógico em A0/A1 |
| Caneta Z | Servo em D9; estados alta, meia e baixa |
| Telemetria | OLED SSD1306 128×64 por I2C |
| Velocidade | 2 barras de 10 segmentos por 3 × 74HC595 |
| Segurança | HOME lógico em D11 e E-STOP travado em D12 |
| Auxiliares | Relés para sucção e ventilação em A2/A3 |

## Documentos

- [`hardware-reference.md`](hardware-reference.md): pinagem, alimentação,
  restrições elétricas e orientação de montagem física.
- [`component-specifications.md`](component-specifications.md): inventário dos
  componentes e seus identificadores no Wokwi.
- [`technical-specification.md`](technical-specification.md): requisitos,
  arquitetura, algoritmos, estados e critérios de verificação.
- [`revision-notes.md`](revision-notes.md): registro da revisão recebida com o
  projeto original.

## Operação

- Desloque o joystick para comandar a velocidade de X e Y.
- Pressione o seletor do joystick para percorrer `ALTA → MEIA → BAIXA → ALTA`.
- Pressione HOME (`H`) para elevar a caneta e retornar à origem lógica `(0,0)`.
- Pressione E-STOP (`E`) para desabilitar os drivers e as saídas auxiliares. O
  estado é travado e exige reinicialização.

## Limites do modelo

A posição inicial é assumida como `(0,0)` e os limites são implementados por
software. HOME não realiza referenciamento mecânico. O projeto Wokwi é uma
demonstração funcional e não substitui proteções elétricas e mecânicas de uma
máquina real.
