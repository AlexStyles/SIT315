// Code modified by Alex Styles S/No. 216090648

// https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/

static constexpr uint8_t interruptPin = 2;
static constexpr uint16_t debounceMs = 1000;

static volatile uint8_t ledState = 0;
static volatile uint8_t buttonState = 0;
static uint32_t lastButtonPressMs = 0;

void changeState(void) {
  ledState = !ledState;
  buttonState = 1;
}

void setup()
{
  noInterrupts();
  attachInterrupt(digitalPinToInterrupt(interruptPin), changeState, RISING);
  pinMode(interruptPin, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
  interrupts();
}

void loop()
{
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