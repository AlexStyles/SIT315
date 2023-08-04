// Code modified by Alex Styles S/No. 216090648

// https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/
// https://www.electrosoftcloud.com/en/pcint-interrupts-on-arduino/

static constexpr uint8_t buttonPin = 3;
static constexpr uint16_t debounceMs = 100;

static volatile uint8_t ledState = 0;
static volatile uint8_t buttonState = 0;
static uint32_t lastButtonPressMs = 0;

static constexpr uint8_t tiltPin = 2;
static volatile uint8_t tiltDetected = 0;
static constexpr uint8_t piezoPin = 7;

static volatile uint8_t doPing = 0;
static constexpr uint8_t signalPin = 12;
static constexpr uint8_t pingLedPin = 9;
static constexpr uint8_t minProximityCm = 3;
static constexpr uint16_t maxProximityCm = 300;

static volatile uint8_t analogButtonPinInput = 255;
static constexpr uint8_t ledA2Button = 4;
static constexpr uint8_t ledA3Button = 5;
static uint8_t ledA2State = 0;
static uint8_t ledA3State = 0;
static uint32_t lastA2ButtonPressMs = 0;
static uint32_t lastA3ButtonPressMs = 0;

void changeButtonState(void) {
  ledState = !ledState;
  buttonState = 1;
}

void changeTiltState(void) {
  tiltDetected = 1;
}


ISR(TIMER1_COMPA_vect)
{
  doPing = 1;
}

ISR(PCINT1_vect) {
  // Interrupt triggers from any input on A2/A3
  // Meaning we need to filter for the correct one
  if (digitalRead(A2)) {
    analogButtonPinInput = A2;
  } else if (digitalRead(A3)) {
    analogButtonPinInput = A3;
  }
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
  // Mask interrupts for only A2 & A3
  PCMSK1 |= 0b00001100;
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
  attachInterrupt(digitalPinToInterrupt(buttonPin), changeButtonState, RISING);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(tiltPin, INPUT);
  pinMode(piezoPin, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(pingLedPin, OUTPUT);
  pinMode(A2, INPUT_PULLUP);
  pinMode(A3, INPUT_PULLUP);
  pinMode(ledA2Button, OUTPUT);
  pinMode(ledA3Button, OUTPUT);
  Serial.begin(9600);
  interrupts();
}

void loop()
{
  HandleButtonPress();
  HandleTiltDetected();
  HandlePing();
  HandleAnalogButtonPress();
}

void HandleButtonPress() {
  if (buttonState) {
    if ((millis() - lastButtonPressMs) > debounceMs) {
      digitalWrite(LED_BUILTIN, ledState);
      lastButtonPressMs = millis();
      Serial.println("Changing LED state!");
      Serial.print("Last button press = ");
      Serial.print(lastButtonPressMs);
      Serial.println("ms");
    } else {
      Serial.println("Button state changed within debounce period!");
    }
    buttonState = 0;
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

void HandleAnalogButtonPress() {
  if ((millis() - lastA2ButtonPressMs) <= debounceMs) {
    return;
  }
  switch (analogButtonPinInput) {
    case A2:
    ledA2State = ledA2State ? 0 : 1;
    digitalWrite(ledA2Button, ledA2State);
    lastA2ButtonPressMs = millis();
    Serial.println("A2 Button LED state changed");
    break;
    case A3:
    ledA3State = ledA3State ? 0 : 1;
    digitalWrite(ledA3Button, ledA3State);
    lastA3ButtonPressMs = millis();
    Serial.println("A3 Button LED state changed");
    break;
  }
  if (analogButtonPinInput) {
    analogButtonPinInput = 255;
  }
}
