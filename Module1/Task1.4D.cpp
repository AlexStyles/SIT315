// Code modified by Alex Styles S/No. 216090648

// https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/
// https://www.electrosoftcloud.com/en/pcint-interrupts-on-arduino/

struct RGB {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
  RGB() = default;
  RGB(uint8_t inRed, uint8_t inGreen, uint8_t inBlue) : red(inRed), green(inGreen), blue(inBlue) {}
};

static constexpr uint8_t pirSensorPin = 3;
static volatile uint8_t motionDetected = 0;
static constexpr uint16_t motionTimeoutMs = 2000;
static uint32_t lastMotionDetectionMs = 0;

static constexpr uint8_t clusterSensor1Pin = A1;
static constexpr uint8_t clusterSensor2Pin = A2;
static constexpr uint8_t clusterSensor3Pin = A3;
static constexpr uint8_t clusterSensor4Pin = A4;
static uint8_t clusterSensorStates = 0;
static uint32_t clusterSensor1DetectionMs = 0;
static uint32_t clusterSensor2DetectionMs = 0;
static uint32_t clusterSensor3DetectionMs = 0;
static uint32_t clusterSensor4DetectionMs = 0;

static constexpr uint8_t rgbLedRedPin = 4;
static constexpr uint8_t rgbLedBluePin = 5;
static constexpr uint8_t rgbLedGreenPin = 6;
static const RGB clusterSensor1Colour(255, 153, 51);
static const RGB clusterSensor2Colour(128, 255, 0);
static const RGB clusterSensor3Colour(51, 51, 255);
static const RGB clusterSensor4Colour(204, 0, 204);

static constexpr uint8_t tiltPin = 2;
static volatile uint8_t tiltDetected = 0;
static constexpr uint8_t piezoPin = 7;

static volatile uint8_t doPing = 0;
static constexpr uint8_t signalPin = 12;
static constexpr uint8_t pingLedPin = 9;
static constexpr uint8_t minProximityCm = 3;
static constexpr uint16_t maxProximityCm = 300;

void changeMotionDetectedState(void) {
  motionDetected = digitalRead(pirSensorPin);
}

void changeTiltState(void) {
  tiltDetected = 1;
}


ISR(TIMER1_COMPA_vect)
{
  doPing = 1;
}

ISR(PCINT1_vect) {
  // Interrupt triggers from any input on A1-A4
  clusterSensorStates = (PINC & 0b00011110);
}

void InitializeTimers()
{
  // TCCRx: Timer/Counter Control Register. Configures prescalar
  TCCR1A = 0;
  TCCR1B = 0;
  
  // Timer/Counter Register, holds timer value
  TCNT1 = 0;
  
  // Output compare register - 20Hz / 50ms rate
  OCR1A = 3124;
  
  TCCR1B |= (1 << CS12);
  TIMSK1 |= (1 << OCIE1A);
  TCCR1B |= (1 << WGM12);
}

void InitInterrupts() {
  // Enable interrupts on PC port (Analog 0-5)
  PCICR |= 0b00000010;
  // Mask interrupts for A1 - A4
  PCMSK1 |= 0b00011110;
}

void ResetPingState()
{
  pinMode(signalPin, OUTPUT);
  digitalWrite(signalPin, LOW);
  delayMicroseconds(2);
  digitalWrite(signalPin, HIGH);
  delayMicroseconds(5);
  digitalWrite(signalPin, LOW);
  pinMode(signalPin, INPUT);
}

void setup()
{
  noInterrupts();
  InitializeTimers();
  InitInterrupts();
  attachInterrupt(digitalPinToInterrupt(tiltPin), changeTiltState, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pirSensorPin), changeMotionDetectedState, CHANGE);
  pinMode(tiltPin, INPUT);
  pinMode(piezoPin, OUTPUT);
  pinMode(clusterSensor1Pin, INPUT);
  pinMode(clusterSensor2Pin, INPUT);
  pinMode(clusterSensor3Pin, INPUT);
  pinMode(clusterSensor4Pin, INPUT);
  pinMode(rgbLedRedPin, OUTPUT);
  pinMode(rgbLedBluePin, OUTPUT);
  pinMode(rgbLedGreenPin, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(pingLedPin, OUTPUT);
  Serial.begin(9600);
  interrupts();
}

void loop()
{
  HandleMotionDetected();
  HandleTiltDetected();
  HandlePing();
  HandleClusterSensors();
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

void HandlePing() {
  if (doPing) {
    ResetPingState();
    HandleRead();
    doPing = 0;
  }
}

void HandleRead()
{
  // Speed of sound = 343 m/s == 34,300 cm/s
  // 34,300 / 1*10^-6 = 0.0343 cm/us
  // 1 / 0.0343 = 29.1 us/cm
  unsigned long durationUs = pulseIn(signalPin, HIGH);
  unsigned short distanceCm = (durationUs / 2) / 29;

  HandleProximity(distanceCm);
}

void HandleProximity(const unsigned short distanceCm)
{
  if (distanceCm <= maxProximityCm && distanceCm >= minProximityCm)
  {
    digitalWrite(pingLedPin, HIGH);
    Serial.print(distanceCm);
    Serial.println(" cm");
  }
  else
  {
    digitalWrite(pingLedPin, LOW);
  }
}

void HandleClusterSensors(void)
{
  if (clusterSensorStates & 0b00000010) {
    if ((millis() - clusterSensor1DetectionMs) > motionTimeoutMs) {
      analogWrite(rgbLedRedPin, clusterSensor1Colour.red);
      analogWrite(rgbLedGreenPin, clusterSensor1Colour.green);
      analogWrite(rgbLedBluePin, clusterSensor1Colour.blue);
      Serial.println("Motion detected on sensor 1");
      clusterSensor1DetectionMs = millis();
    }
  }
  if (clusterSensorStates & 0b00000100) {
    if ((millis() - clusterSensor2DetectionMs) > motionTimeoutMs) {
      analogWrite(rgbLedRedPin, clusterSensor2Colour.red);
      analogWrite(rgbLedGreenPin, clusterSensor2Colour.green);
      analogWrite(rgbLedBluePin, clusterSensor2Colour.blue);
      Serial.println("Motion detected on sensor 2");
      clusterSensor2DetectionMs = millis();
    }
  }
  if (clusterSensorStates & 0b00001000) {
    if ((millis() - clusterSensor3DetectionMs) > motionTimeoutMs) {
      analogWrite(rgbLedRedPin, clusterSensor3Colour.red);
      analogWrite(rgbLedGreenPin, clusterSensor3Colour.green);
      analogWrite(rgbLedBluePin, clusterSensor3Colour.blue);
      Serial.println("Motion detected on sensor 3");
      clusterSensor3DetectionMs = millis();
    }
  }
  if (clusterSensorStates & 0b00010000) {
    if ((millis() - clusterSensor4DetectionMs) > motionTimeoutMs) {
      analogWrite(rgbLedRedPin, clusterSensor4Colour.red);
      analogWrite(rgbLedGreenPin, clusterSensor4Colour.green);
      analogWrite(rgbLedBluePin, clusterSensor4Colour.blue);
      Serial.println("Motion detected on sensor 4");
      clusterSensor4DetectionMs = millis();
    }
  }
  if (!clusterSensorStates) {
    analogWrite(rgbLedRedPin, 0);
    analogWrite(rgbLedGreenPin, 0);
    analogWrite(rgbLedBluePin, 0);
  }
}
