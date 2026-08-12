#include <AccelStepper.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Servo.h>
#include <math.h>

// ============================================================
// 2D PLOTTER - Arduino Uno / Wokwi
// Pinagem revisada para manter D0/D1 livres para a porta serial.
// ============================================================

// ------------------------- OLED ------------------------------
constexpr uint8_t SCREEN_WIDTH  = 128;
constexpr uint8_t SCREEN_HEIGHT = 64;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ----------------------- Pinagem ------------------------------
// Movimento X/Y (A4988)
constexpr uint8_t X_STEP_PIN = 2;
constexpr uint8_t X_DIR_PIN  = 3;
constexpr uint8_t Y_STEP_PIN = 4;
constexpr uint8_t Y_DIR_PIN  = 5;

// Barras de velocidade - 3 x 74HC595 em cascata
constexpr uint8_t BAR_DATA_PIN  = 6;  // DS
constexpr uint8_t BAR_CLOCK_PIN = 7;  // SHCP
constexpr uint8_t BAR_LATCH_PIN = 8;  // STCP

// Caneta / eixo Z
constexpr uint8_t SERVO_Z_PIN = 9;

// Comandos
constexpr uint8_t JOY_SEL_PIN = 10;
constexpr uint8_t HOME_PIN    = 11;
constexpr uint8_t ESTOP_PIN   = 12;

// Habilitacao conjunta dos dois A4988 (ENABLE e ativo em LOW)
constexpr uint8_t STEPPERS_ENABLE_PIN = 13;

// Entradas analogicas do joystick
constexpr uint8_t JOY_X_PIN = A0;
constexpr uint8_t JOY_Y_PIN = A1;

// Saidas auxiliares (A2/A3 usados como digitais)
constexpr uint8_t RELAY_SUCTION_PIN = A2;
constexpr uint8_t RELAY_FAN_PIN     = A3;
// A4 = SDA e A5 = SCL do OLED I2C

// ------------------------ Movimento ---------------------------
AccelStepper stepperX(AccelStepper::DRIVER, X_STEP_PIN, X_DIR_PIN);
AccelStepper stepperY(AccelStepper::DRIVER, Y_STEP_PIN, Y_DIR_PIN);

constexpr float MAX_SPEED_STEPS_S = 800.0f;
constexpr float ACCEL_STEPS_S2    = 450.0f;
constexpr float HOMING_SPEED      = 500.0f;

// Wokwi: motor de passo padrao = 200 passos/revolucao em passo inteiro.
constexpr float STEPS_PER_REV = 200.0f;
constexpr float MAX_RPM = MAX_SPEED_STEPS_S * 60.0f / STEPS_PER_REV;

// Limites virtuais da mesa: as bordas do OLED representam estes limites.
constexpr long X_MIN_STEPS = 0;
constexpr long X_MAX_STEPS = 4000;
constexpr long Y_MIN_STEPS = 0;
constexpr long Y_MAX_STEPS = 2400;

// Joystick
constexpr int JOY_CENTER   = 512;
constexpr int JOY_DEADZONE = 70;

// ------------------------- Eixo Z -----------------------------
Servo servoZ;

enum ZState : uint8_t {
  Z_HIGH = 0,
  Z_MID  = 1,
  Z_LOW  = 2
};

ZState zState = Z_HIGH;

constexpr uint8_t Z_ANGLE_HIGH = 180;
constexpr uint8_t Z_ANGLE_MID  = 90;
constexpr uint8_t Z_ANGLE_LOW  = 0;

// ---------------------- Estado geral --------------------------
bool emergency  = false;
bool isHoming   = false;
bool workActive = false;

int8_t lastCommandDirX = 0;
int8_t lastCommandDirY = 0;

unsigned long lastMotionMs = 0;
constexpr unsigned long FAN_HOLD_MS = 2000;

// ---------------------- Debounce ------------------------------
struct DebouncedButton {
  uint8_t pin;
  bool stableState = HIGH;
  bool lastRawState = HIGH;
  unsigned long lastChangeMs = 0;

  void begin() {
    pinMode(pin, INPUT_PULLUP);
    stableState = digitalRead(pin);
    lastRawState = stableState;
  }

  bool fell() {
    const bool raw = digitalRead(pin);

    if (raw != lastRawState) {
      lastRawState = raw;
      lastChangeMs = millis();
    }

    if ((millis() - lastChangeMs) >= 35 && raw != stableState) {
      const bool previous = stableState;
      stableState = raw;
      return previous == HIGH && stableState == LOW;
    }

    return false;
  }
};

DebouncedButton homeButton{HOME_PIN};
DebouncedButton zButton{JOY_SEL_PIN};

// ============================================================
// Funcoes auxiliares
// ============================================================

void setZState(ZState state) {
  zState = state;

  switch (zState) {
    case Z_HIGH: servoZ.write(Z_ANGLE_HIGH); break;
    case Z_MID:  servoZ.write(Z_ANGLE_MID);  break;
    case Z_LOW:  servoZ.write(Z_ANGLE_LOW);  break;
  }
}

void cycleZState() {
  // Sequencia fisica: Alta -> Meia -> Baixa -> Alta
  switch (zState) {
    case Z_HIGH: setZState(Z_MID);  break;
    case Z_MID:  setZState(Z_LOW);  break;
    case Z_LOW:  setZState(Z_HIGH); break;
  }
}

float joystickToSpeed(int raw) {
  const int lowEdge  = JOY_CENTER - JOY_DEADZONE;
  const int highEdge = JOY_CENTER + JOY_DEADZONE;

  if (raw > highEdge) {
    const float ratio = (float)(raw - highEdge) / (float)(1023 - highEdge);
    return ratio * MAX_SPEED_STEPS_S;
  }

  if (raw < lowEdge) {
    const float ratio = (float)(lowEdge - raw) / (float)lowEdge;
    return -ratio * MAX_SPEED_STEPS_S;
  }

  return 0.0f;
}

int8_t speedDirection(float speedCommand) {
  if (speedCommand > 0.5f) return 1;
  if (speedCommand < -0.5f) return -1;
  return 0;
}

void commandAxis(AccelStepper &stepper,
                 float speedCommand,
                 long minPosition,
                 long maxPosition,
                 int8_t &lastDirection) {
  const int8_t direction = speedDirection(speedCommand);

  if (direction != 0) {
    stepper.setMaxSpeed(fabs(speedCommand));
    stepper.moveTo(direction > 0 ? maxPosition : minPosition);
  } else if (lastDirection != 0) {
    // stop() calcula uma rampa de desaceleracao. Depois limitamos o alvo
    // para impedir que a rampa ultrapasse a mesa virtual.
    stepper.stop();
    long stopTarget = stepper.targetPosition();
    if (stopTarget < minPosition) stopTarget = minPosition;
    if (stopTarget > maxPosition) stopTarget = maxPosition;
    stepper.moveTo(stopTarget);
  }

  lastDirection = direction;
}

void startHoming() {
  isHoming = true;
  workActive = false;
  lastCommandDirX = 0;
  lastCommandDirY = 0;

  setZState(Z_HIGH);

  stepperX.setMaxSpeed(HOMING_SPEED);
  stepperY.setMaxSpeed(HOMING_SPEED);
  stepperX.moveTo(X_MIN_STEPS);
  stepperY.moveTo(Y_MIN_STEPS);

  Serial.println(F("HOMING iniciado"));
}

void triggerEmergency() {
  emergency = true;

  // Desabilita fisicamente (na simulacao) os dois A4988 imediatamente.
  digitalWrite(STEPPERS_ENABLE_PIN, HIGH);

  // Saidas auxiliares em estado seguro.
  digitalWrite(RELAY_SUCTION_PIN, LOW);
  digitalWrite(RELAY_FAN_PIN, LOW);

  Serial.println(F("EMERGENCIA: reset necessario"));
}

// ============================================================
// Barras de velocidade - 20 segmentos / 3 x 74HC595
// ============================================================

uint16_t levelMask10(uint8_t level) {
  if (level == 0) return 0;
  if (level >= 10) return 0x03FF;
  return (1U << level) - 1U;
}

uint8_t speedToBarLevel(float speedStepsS) {
  const float rpm = fabs(speedStepsS) * 60.0f / STEPS_PER_REV;
  if (rpm < 0.1f) return 0;

  int level = (int)ceil((rpm / MAX_RPM) * 10.0f);
  if (level < 0) level = 0;
  if (level > 10) level = 10;
  return (uint8_t)level;
}

void writeBarGraphs(uint8_t levelX, uint8_t levelY) {
  const uint16_t x = levelMask10(levelX);
  const uint16_t y = levelMask10(levelY);

  // Registrador 0 (mais proximo do Arduino): X1..X8
  const uint8_t sr0 = x & 0xFF;

  // Registrador 1: X9..X10 em Q0..Q1 e Y1..Y6 em Q2..Q7
  const uint8_t sr1 = ((x >> 8) & 0x03) | ((y & 0x3F) << 2);

  // Registrador 2 (mais distante): Y7..Y10 em Q0..Q3
  const uint8_t sr2 = (y >> 6) & 0x0F;

  digitalWrite(BAR_LATCH_PIN, LOW);
  // O byte do registrador mais distante deve ser enviado primeiro.
  shiftOut(BAR_DATA_PIN, BAR_CLOCK_PIN, MSBFIRST, sr2);
  shiftOut(BAR_DATA_PIN, BAR_CLOCK_PIN, MSBFIRST, sr1);
  shiftOut(BAR_DATA_PIN, BAR_CLOCK_PIN, MSBFIRST, sr0);
  digitalWrite(BAR_LATCH_PIN, HIGH);
}

// ============================================================
// Reles / cargas simuladas
// ============================================================

void updateAuxiliaryLoads() {
  const bool moving = fabs(stepperX.speed()) > 0.5f || fabs(stepperY.speed()) > 0.5f;

  if (moving || isHoming) {
    lastMotionMs = millis();
  }

  // A mesa de succao fica ligada durante uma sessao de trabalho.
  // A sessao e armada quando o operador movimenta um eixo ou altera Z,
  // e e encerrada quando HOME e solicitado.
  const bool suctionOn = workActive && !isHoming;

  // O ventilador acompanha o movimento e permanece ligado por 2 s
  // depois que os motores param.
  const bool fanOn = moving || isHoming || ((millis() - lastMotionMs) < FAN_HOLD_MS);

  // No diagram.json os modulos usam transistor="pnp", portanto HIGH
  // conecta COM a NO e acende a carga visual ligada ao contato NO.
  digitalWrite(RELAY_SUCTION_PIN, suctionOn ? HIGH : LOW);
  digitalWrite(RELAY_FAN_PIN, fanOn ? HIGH : LOW);
}

// ============================================================
// OLED: mesa virtual + finais de carreira por software
// ============================================================

constexpr int TABLE_LEFT   = 2;
constexpr int TABLE_TOP    = 18;
constexpr int TABLE_RIGHT  = 125;
constexpr int TABLE_BOTTOM = 62;

int mapLongToInt(long value, long inMin, long inMax, int outMin, int outMax) {
  if (value < inMin) value = inMin;
  if (value > inMax) value = inMax;
  return (int)(outMin + ((value - inMin) * (long)(outMax - outMin)) / (inMax - inMin));
}

void drawTableAndPen() {
  const long x = stepperX.currentPosition();
  const long y = stepperY.currentPosition();

  display.drawRect(TABLE_LEFT,
                   TABLE_TOP,
                   TABLE_RIGHT - TABLE_LEFT + 1,
                   TABLE_BOTTOM - TABLE_TOP + 1,
                   SSD1306_WHITE);

  // Destaca a borda correspondente quando um final de carreira virtual
  // esta ativo.
  if (x <= X_MIN_STEPS) {
    display.drawFastVLine(TABLE_LEFT + 1, TABLE_TOP + 1,
                          TABLE_BOTTOM - TABLE_TOP - 1, SSD1306_WHITE);
  }
  if (x >= X_MAX_STEPS) {
    display.drawFastVLine(TABLE_RIGHT - 1, TABLE_TOP + 1,
                          TABLE_BOTTOM - TABLE_TOP - 1, SSD1306_WHITE);
  }
  if (y <= Y_MIN_STEPS) {
    display.drawFastHLine(TABLE_LEFT + 1, TABLE_BOTTOM - 1,
                          TABLE_RIGHT - TABLE_LEFT - 1, SSD1306_WHITE);
  }
  if (y >= Y_MAX_STEPS) {
    display.drawFastHLine(TABLE_LEFT + 1, TABLE_TOP + 1,
                          TABLE_RIGHT - TABLE_LEFT - 1, SSD1306_WHITE);
  }

  const int px = mapLongToInt(x, X_MIN_STEPS, X_MAX_STEPS,
                              TABLE_LEFT + 3, TABLE_RIGHT - 3);
  // Y positivo sobe na representacao da mesa.
  const int py = mapLongToInt(y, Y_MIN_STEPS, Y_MAX_STEPS,
                              TABLE_BOTTOM - 3, TABLE_TOP + 3);

  // O marcador muda de forma conforme a altura da caneta.
  switch (zState) {
    case Z_HIGH:
      display.drawCircle(px, py, 2, SSD1306_WHITE);
      break;
    case Z_MID:
      display.drawCircle(px, py, 2, SSD1306_WHITE);
      display.drawPixel(px, py, SSD1306_WHITE);
      break;
    case Z_LOW:
      display.fillCircle(px, py, 2, SSD1306_WHITE);
      break;
  }
}

void drawNormalScreen() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  display.setCursor(0, 0);
  if (isHoming) {
    display.print(F("HOMING "));
  } else {
    display.print(F("X:"));
    display.print(stepperX.currentPosition());
    display.print(F(" Y:"));
    display.print(stepperY.currentPosition());
  }

  display.setCursor(0, 9);
  display.print(F("Z:"));
  switch (zState) {
    case Z_HIGH: display.print(F("ALTA"));  break;
    case Z_MID:  display.print(F("MEIA"));  break;
    case Z_LOW:  display.print(F("BAIXA")); break;
  }

  display.print(F(" S:"));
  display.print(digitalRead(RELAY_SUCTION_PIN) == HIGH ? '1' : '0');
  display.print(F(" F:"));
  display.print(digitalRead(RELAY_FAN_PIN) == HIGH ? '1' : '0');

  drawTableAndPen();
  display.display();
}

void drawEmergencyScreen() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);
  display.setCursor(4, 16);
  display.println(F("EMERGENCIA"));
  display.setTextSize(1);
  display.setCursor(17, 44);
  display.println(F("RESET NECESSARIO"));
  display.display();
}

// ============================================================
// Setup / loop
// ============================================================

void setup() {
  Serial.begin(115200);

  pinMode(STEPPERS_ENABLE_PIN, OUTPUT);
  digitalWrite(STEPPERS_ENABLE_PIN, LOW); // A4988 habilitados

  pinMode(BAR_DATA_PIN, OUTPUT);
  pinMode(BAR_CLOCK_PIN, OUTPUT);
  pinMode(BAR_LATCH_PIN, OUTPUT);
  writeBarGraphs(0, 0);

  pinMode(RELAY_SUCTION_PIN, OUTPUT);
  pinMode(RELAY_FAN_PIN, OUTPUT);
  digitalWrite(RELAY_SUCTION_PIN, LOW);
  digitalWrite(RELAY_FAN_PIN, LOW);

  pinMode(ESTOP_PIN, INPUT_PULLUP);
  homeButton.begin();
  zButton.begin();

  servoZ.attach(SERVO_Z_PIN);
  setZState(Z_HIGH);

  stepperX.setMaxSpeed(MAX_SPEED_STEPS_S);
  stepperX.setAcceleration(ACCEL_STEPS_S2);
  stepperY.setMaxSpeed(MAX_SPEED_STEPS_S);
  stepperY.setAcceleration(ACCEL_STEPS_S2);

  // Origem logica: canto inferior esquerdo da mesa virtual.
  stepperX.setCurrentPosition(0);
  stepperY.setCurrentPosition(0);

  // Evita que o ventilador ligue durante os primeiros 2 s apos o reset.
  lastMotionMs = millis() - FAN_HOLD_MS;

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    for (;;) {
      digitalWrite(STEPPERS_ENABLE_PIN, HIGH);
    }
  }

  display.clearDisplay();
  display.display();

  Serial.println(F("2D Plotter pronto"));
}

void loop() {
  // 1) E-STOP: leitura sem debounce para resposta imediata e estado latched.
  if (!emergency && digitalRead(ESTOP_PIN) == LOW) {
    triggerEmergency();
  }

  if (emergency) {
    static bool emergencyScreenDrawn = false;
    if (!emergencyScreenDrawn) {
      writeBarGraphs(0, 0);
      drawEmergencyScreen();
      emergencyScreenDrawn = true;
    }
    return;
  }

  // 2) HOME
  if (homeButton.fell() && !isHoming) {
    startHoming();
  }

  // 3) Controle Z. Durante homing, o SEL fica bloqueado.
  if (zButton.fell() && !isHoming) {
    cycleZState();
    workActive = true;
  }

  // 4) Movimento manual ou homing.
  if (!isHoming) {
    const float targetSpeedX = joystickToSpeed(analogRead(JOY_X_PIN));
    const float targetSpeedY = joystickToSpeed(analogRead(JOY_Y_PIN));

    const int8_t dirX = speedDirection(targetSpeedX);
    const int8_t dirY = speedDirection(targetSpeedY);
    if (dirX != 0 || dirY != 0) {
      workActive = true;
    }

    commandAxis(stepperX, targetSpeedX, X_MIN_STEPS, X_MAX_STEPS, lastCommandDirX);
    commandAxis(stepperY, targetSpeedY, Y_MIN_STEPS, Y_MAX_STEPS, lastCommandDirY);
  }

  stepperX.run();
  stepperY.run();

  if (isHoming &&
      stepperX.distanceToGo() == 0 &&
      stepperY.distanceToGo() == 0 &&
      fabs(stepperX.speed()) < 0.5f &&
      fabs(stepperY.speed()) < 0.5f) {
    stepperX.setCurrentPosition(0);
    stepperY.setCurrentPosition(0);
    isHoming = false;
    Serial.println(F("HOMING concluido em (0,0)"));
  }

  // 5) Cargas auxiliares.
  updateAuxiliaryLoads();

  // 6) Barras: velocidade angular real, apos a rampa do AccelStepper.
  static unsigned long lastBarUpdateMs = 0;
  if (millis() - lastBarUpdateMs >= 30) {
    writeBarGraphs(speedToBarLevel(stepperX.speed()),
                   speedToBarLevel(stepperY.speed()));
    lastBarUpdateMs = millis();
  }

  // 7) Interface OLED.
  static unsigned long lastDisplayUpdateMs = 0;
  if (millis() - lastDisplayUpdateMs >= 80) {
    drawNormalScreen();
    lastDisplayUpdateMs = millis();
  }
}
