# Referência de hardware — 2D Plotter

## 1. Controlador

O alvo é o **Arduino Uno R3**, com ATmega328P, lógica de 5 V e recursos de
memória limitados. D0/D1 permanecem livres para UART. A4/A5 são reservados ao
barramento I2C do OLED.

## 2. Mapa de pinos

| Função | Pino | Direção | Estado relevante |
|---|---:|---|---|
| X STEP / DIR | D2 / D3 | saída | pulsos e sentido do A4988 X |
| Y STEP / DIR | D4 / D5 | saída | pulsos e sentido do A4988 Y |
| 74HC595 DATA / CLOCK / LATCH | D6 / D7 / D8 | saída | três registradores em cascata |
| Servo da caneta | D9 | saída PWM | 180°, 90° ou 0° |
| Seletor do joystick | D10 | entrada | `INPUT_PULLUP`, ativo em LOW |
| HOME | D11 | entrada | `INPUT_PULLUP`, ativo em LOW |
| E-STOP | D12 | entrada | `INPUT_PULLUP`, ativo em LOW |
| ENABLE comum dos A4988 | D13 | saída | ativo em LOW |
| Joystick X / Y | A0 / A1 | entrada analógica | centro 512; zona morta ±70 |
| Relé de sucção / ventilador | A2 / A3 | saída digital | HIGH aciona o modelo Wokwi |
| OLED SDA / SCL | A4 / A5 | I2C | endereço `0x3C` |

## 3. Movimento

Cada eixo usa um A4988 no modo STEP/DIR e um motor de passo de 200 passos por
revolução. O firmware limita a velocidade a 800 passos/s (240 rpm no modelo) e
a aceleração a 450 passos/s². Os limites lógicos são:

| Eixo | Mínimo | Máximo |
|---|---:|---:|
| X | 0 | 4000 passos |
| Y | 0 | 2400 passos |

Os pinos RESET e SLEEP de cada A4988 estão unidos no diagrama. Em montagem
física, devem permanecer em nível alto durante a operação. O pino `VMOT` não é
usado pelo modelo Wokwi; na máquina real, requer fonte adequada, desacoplamento
próximo ao driver e ajuste de corrente antes de conectar o motor.

## 4. Barras de velocidade

Três 74HC595 fornecem 24 saídas: 20 alimentam os segmentos das barras X/Y e
quatro ficam sem uso. CLOCK e LATCH são compartilhados; Q7S encadeia os dados.
O byte do registrador mais distante é transmitido primeiro.

No diagrama, os segmentos estão ligados diretamente para reduzir a poluição
visual. Na montagem real, **cada segmento necessita de resistor próprio** e a
corrente total por registrador deve respeitar a folha de dados. Considere
drivers de corrente quando vários segmentos permanecerem acesos.

## 5. Alimentação e segurança física

- Una os terras da lógica, drivers, fonte dos motores e periféricos.
- Não alimente motores a partir do pino 5 V do Uno.
- Instale capacitor de desacoplamento em `VMOT` próximo a cada A4988.
- Ajuste o limite de corrente conforme motor, driver e refrigeração.
- Use chaves físicas de fim de curso para referenciamento real.
- Implemente E-STOP cabeado, capaz de retirar energia dos atuadores sem
  depender do microcontrolador.
- Garanta estado seguro da caneta e das cargas na energização e na falha.

## 6. Checklist de montagem

- confirmar a continuidade de GND e a polaridade das fontes;
- confirmar pares das bobinas A/B antes de energizar;
- ajustar corrente dos A4988 com motores desconectados quando aplicável;
- testar ENABLE e E-STOP antes de instalar a ferramenta;
- validar HOME e limites em baixa velocidade;
- instalar resistores nos 20 LEDs;
- testar sucção e ventilação sem carga mecânica;
- só então liberar movimentos em toda a área útil.
