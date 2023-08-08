// Code modified by Alex Styles S/No. 216090648

// https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/

static constexpr uint8_t pirSensorPin = 3;
static volatile uint8_t motionDetected = 0;
static constexpr uint16_t motionTimeoutMs = 2000;
static uint32_t lastMotionDetectionMs = 0;

static constexpr uint8_t tiltPin = 2;
static volatile uint8_t tiltDetected = 0;
static constexpr uint8_t piezoPin = 7;

void changeMotionDetectedState(void) {
  motionDetected = digitalRead(pirSensorPin);
}

void changeTiltState(void) {
  tiltDetected = 1;
}

void setup()
{
  noInterrupts();
  attachInterrupt(digitalPinToInterrupt(tiltPin), changeTiltState, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pirSensorPin), changeMotionDetectedState, CHANGE);
  pinMode(pirSensorPin, INPUT);
  pinMode(tiltPin, INPUT);
  pinMode(piezoPin, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
  interrupts();
}

void loop()
{
  HandleMotionDetected();
  HandleTiltDetected();
}

void HandleMotionDetected() {
  if (motionDetected) {
    if ((millis() - lastMotionDetectionMs) > motionTimeoutMs) {
      digitalWrite(LED_BUILTIN, HIGH);
      lastMotionDetectionMs = millis();
      Serial.println("Motion detected!");
      Serial.print("Last detection time = ");
      Serial.print(lastMotionDetectionMs);
      Serial.println("ms");
    }
  } else {
    digitalWrite(LED_BUILTIN, LOW);
  }
}

void HandleTiltDetected() {
  if (tiltDetected) {
    Serial.println("Tilt Detected!");
    tone(piezoPin, 494, 5);
    tiltDetected = 0;
  }
}
