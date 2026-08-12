# Especificação técnica — 2D Plotter

## 1. Controle do documento

| Campo | Valor |
|---|---|
| Projeto | Controle de plotter cartesiana de dois eixos |
| Plataforma | Arduino Uno R3 / ATmega328P |
| Linguagem | Arduino C++ |
| Simulação | Wokwi |
| Firmware | `sketch.ino` |
| Circuito | `diagram.json` |
| Dependências | `libraries.txt` |
| Idioma | Português do Brasil |

## 2. Objetivo e escopo

O sistema permite controlar manualmente uma plotter X/Y, posicionar a caneta
em três alturas e observar posição e estados no OLED. A simulação também
representa velocidade, sucção da mesa e ventilação. O escopo contempla lógica
de controle e integração simulada; não qualifica a máquina para operação real.

## 3. Requisitos funcionais

1. Converter os eixos analógicos do joystick em comandos de velocidade X/Y.
2. Aplicar aceleração e desaceleração sem chamadas bloqueantes.
3. Restringir X a `0..4000` e Y a `0..2400` passos.
4. Alternar Z em `ALTA → MEIA → BAIXA → ALTA` pelo seletor do joystick.
5. Retornar à origem lógica com a caneta alta ao solicitar HOME.
6. Exibir posição, Z e estado das cargas em OLED, incluindo a mesa virtual.
7. Indicar o módulo da velocidade real de cada eixo em barra de dez segmentos.
8. Manter sucção durante a sessão de trabalho e desligá-la ao iniciar HOME.
9. Manter ventilação durante movimento/HOME e por 2 s após a parada.
10. Ao detectar E-STOP, desabilitar drivers e cargas, travar o estado e exigir
    reinicialização.

## 4. Arquitetura

O firmware é um superlaço cooperativo sem `delay()`. Cada iteração executa as
funções urgentes primeiro e atualiza interfaces com períodos independentes:

```text
entradas ──> segurança ──> estados HOME/Z ──> comandos X/Y
                 │                              │
                 └──> ENABLE/cargas             ├──> AccelStepper.run()
                                                ├──> barras (30 ms)
                                                └──> OLED (80 ms)
```

`AccelStepper::run()` precisa ser chamado com frequência; por isso OLED,
barras, botões e auxiliares não bloqueiam o laço.

## 5. Controle X/Y

### 5.1 Conversão do joystick

O conversor A/D produz `0..1023`, com centro nominal em 512. A faixa
`442..582` é zona morta. Fora dela, a distância até a extremidade é
normalizada e multiplicada por 800 passos/s. O sinal determina o sentido.

### 5.2 Perfil e limites

Para comando diferente de zero, o alvo é o limite do sentido selecionado e a
velocidade máxima do eixo recebe o módulo do comando. Ao retornar à zona
morta, `stop()` calcula a desaceleração e o alvo resultante é limitado à área
válida. `run()` produz os pulsos STEP respeitando aceleração e posição-alvo.

### 5.3 HOME

HOME eleva Z, encerra a sessão de sucção e comanda simultaneamente os dois
eixos para zero a 500 passos/s. Quando ambos atingem a origem e ficam parados,
as posições são reafirmadas como zero.

Esse procedimento pressupõe que a coordenada interna é válida. Sem sensores,
não detecta perda de passos nem encontra uma referência física após ligar.

## 6. Controle Z

O servo representa a caneta em três estados:

| Estado | Ângulo | Representação no OLED |
|---|---:|---|
| `Z_HIGH` | 180° | círculo vazio |
| `Z_MID` | 90° | círculo com ponto central |
| `Z_LOW` | 0° | círculo preenchido |

O seletor possui antirrepique não bloqueante de 35 ms e fica inibido durante
HOME. Uma alteração de Z inicia uma sessão de trabalho.

## 7. Telemetria e sinalização

### 7.1 OLED

O SSD1306 usa I2C no endereço `0x3C`. A região inferior desenha a mesa, mapeia
as coordenadas X/Y e destaca a borda quando um limite lógico está ativo. O
quadro é atualizado no máximo a cada 80 ms. Em emergência, é substituído por
uma mensagem persistente de reinicialização obrigatória.

### 7.2 Barras

A velocidade real retornada por `AccelStepper::speed()` é convertida em rpm e
em nível `0..10`. Os 20 bits são distribuídos por três bytes enviados aos
74HC595. A atualização a cada 30 ms não interfere na geração de passos.

## 8. Saídas auxiliares

`workActive` é armado por movimento manual ou alteração de Z. A sucção fica
ativa enquanto a sessão estiver armada e HOME não estiver em curso. O
ventilador liga com movimento ou HOME e permanece ligado durante 2000 ms após
o último movimento detectado.

Os LEDs após os contatos normalmente abertos apenas representam as cargas; o
projeto não modela características de bombas, ventiladores ou suas proteções.

## 9. Segurança

E-STOP é verificado antes das demais funções e sem antirrepique para reduzir a
latência. Ao nível LOW, o firmware:

1. grava o estado de emergência;
2. coloca D13 em HIGH, desabilitando ambos os A4988;
3. desliga sucção e ventilação;
4. zera as barras e mostra a tela de emergência uma vez;
5. deixa de executar o restante do laço até reset.

Essa função é uma camada de controle, não uma categoria de segurança. Uma
máquina real deve possuir parada cabeada, contatores/relés apropriados,
proteções mecânicas e análise de risco.

## 10. Inicialização e falhas

Na inicialização, drivers são habilitados, barras e relés são desligados, Z é
elevado e posições X/Y são assumidas como zero. Se o OLED não inicializar, o
firmware entra em laço infinito com os drivers desabilitados. A falha não
aciona as cargas auxiliares.

## 11. Verificação

`tests/test_project.py` fornece verificações estáticas reprodutíveis:

- sintaxe e forma básica de `diagram.json`;
- unicidade e presença dos 31 elementos esperados;
- 113 conexões e ligações críticas entre Uno e periféricos;
- correspondência entre constantes de pinagem do firmware e o circuito;
- presença das quatro bibliotecas declaradas;
- ausência de `delay()` no firmware;
- prioridades estruturais de E-STOP e retornos de emergência.

A validação estática não substitui compilação, simulação e ensaios físicos. A
aceitação funcional requer percorrer movimentos nos quatro sentidos, os três
estados Z, HOME, temporização das cargas e E-STOP no Wokwi.

## 12. Critérios de aceitação

| Caso | Resultado esperado |
|---|---|
| Joystick central | eixos desaceleram e permanecem dentro dos limites |
| Joystick em uma direção | eixo acelera no sentido correspondente |
| Seleção Z | um avanço na sequência, sem múltiplos eventos por pressão |
| HOME | Z alta, sucção desligada e posição final `(0,0)` |
| Movimento cessa | ventilador permanece por aproximadamente 2 s |
| Limite atingido | posição não ultrapassa o intervalo configurado |
| E-STOP | drivers/cargas desligados e reset solicitado |

## 13. Limitações e evolução recomendada

- adicionar chaves de HOME e fim de curso por eixo;
- persistir/calibrar passos por milímetro e dimensões reais da área útil;
- acrescentar parser de trajetórias ou G-code com fila de movimentos;
- monitorar falhas e temperatura dos drivers;
- separar alimentação lógica, potência e aterramento conforme EMC;
- substituir acionamentos demonstrativos por interfaces dimensionadas;
- criar ensaios em bancada para latência, repetibilidade e perda de passos.

## 14. Referências

- [Repositório no GitHub](https://github.com/Argus-Kepheus/2d-plotter)
- [Projeto no Wokwi](https://wokwi.com/projects/472168224663835649)
- [Arduino Uno Rev3](https://docs.arduino.cc/hardware/uno-rev3/)
- [AccelStepper](https://www.airspayce.com/mikem/arduino/AccelStepper/)
- [Adafruit SSD1306](https://github.com/adafruit/Adafruit_SSD1306)
- [Formato do diagrama Wokwi](https://docs.wokwi.com/diagram-format)
