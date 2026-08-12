# 2D Plotter

![Ilustração conceitual de uma plotter cartesiana de dois eixos](report/figures/front-cover.png)

Sistema de controle de uma plotter cartesiana de dois eixos, desenvolvido para
Arduino Uno e simulado no Wokwi. O firmware controla movimento X/Y com drivers
A4988, altura da caneta com servo, interface por joystick, retorno à origem,
parada de emergência, telemetria em OLED, barras de velocidade e cargas
auxiliares por relé.

**Documentação completa:** [Português](docs/PT/README.md) · [English](docs/EN/README.md)

**Repositório no GitHub:** <https://github.com/Argus-Kepheus/2d-plotter>

**Simulação no Wokwi:** <https://wokwi.com/projects/472168224663835649>

## Execução rápida

1. Abra o link do Wokwi ou importe `diagram.json`, `sketch.ino` e
   `libraries.txt` em um projeto Arduino Uno.
2. Inicie a simulação.
3. Use o joystick para movimentar X/Y e pressione seu seletor para alternar a
   caneta entre alta, meia e baixa.
4. Use `H` para HOME e `E` para E-STOP. Após E-STOP, reinicie a simulação.

## Estrutura

| Caminho | Conteúdo |
|---|---|
| `sketch.ino` | Firmware Arduino/C++ |
| `diagram.json` | Circuito e layout do Wokwi |
| `libraries.txt` | Dependências instaladas pelo Wokwi |
| `docs/PT/` e `docs/EN/` | Documentação técnica bilíngue |
| `tests/` | Verificações estáticas de consistência |
| `report/` | Relatório técnico em LaTeX e PDF |

## Validação local

```powershell
python -m unittest discover -s tests -v
```

## Limitações de segurança

Os limites X/Y e a origem são lógicos: o circuito não possui chaves físicas de
fim de curso. Em hardware real, use sensores de referência, fonte independente
para `VMOT`, limitação de corrente nos A4988, resistores em todos os segmentos
LED e um circuito de E-STOP independente do firmware.

**Licença:** CC0 1.0 Universal — consulte [`LICENSE`](LICENSE).
