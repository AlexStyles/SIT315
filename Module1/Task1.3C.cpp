// Code modified by Alex Styles S/No. 216090648

// https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/

static constexpr uint8_t buttonPin = 2;
static constexpr uint16_t debounceMs = 1000;

static volatile uint8_t ledState = 0;
static volatile uint8_t buttonState = 0;
static uint32_t lastButtonPressMs = 0;

static constexpr uint8_t tiltPin = 3;
static volatile uint8_t tiltDetected = 0;
static constexpr uint8_t piezoPin = 7;

void changeButtonState(void) {
  ledState = !ledState;
  buttonState = 1;
}

void changeTiltState(void) {
  tiltDetected = 1;
}

void setup()
{
  noInterrupts();
  attachInterrupt(digitalPinToInterrupt(tiltPin), changeTiltState, RISING);
  attachInterrupt(digitalPinToInterrupt(buttonPin), changeButtonState, RISING);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(tiltPin, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
  interrupts();
}

void loop()
{
  HandleButtonPress();
  HandleTiltDetected();
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
    tone(piezoPin, 500, 2);
    tiltDetected = 0;
    noTone(piezoPin);
  }
}
