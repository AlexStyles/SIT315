// C++ code
//
// Reference: https://electronoobs.com/eng_arduino_tut140.php

constexpr uint32_t clockFrequencyHz = 16000000;
const byte LED_PIN = 13;
const byte METER_PIN = A4;
volatile int lastPotentiometerValue = 0;

typedef struct Prescaler {
  uint16_t csRegisterValue;
  uint16_t prescaler;
  Prescaler() = default;
} Prescaler;

void setup()
{
  pinMode(LED_PIN, OUTPUT);
  pinMode(METER_PIN, INPUT);
      
  Serial.begin(9600);
  
  startTimer(0.5);
}

void loop()
{
  int potentioVal = analogRead(METER_PIN);
  if (potentioVal != lastPotentiometerValue) {
    startTimer((double)potentioVal / 4.0);
    lastPotentiometerValue = potentioVal;
  }
}

struct Prescaler determinePrescaler(const double timerFrequencyHz) {
  Prescaler prescaler;
  // Comparison numbers are frequencies for each prescaler at maximum ticks
  // Values calculated manually using the formula
  // Freq (min) = Clock Rate Hz / (Max Ticks * Prescaler)
  // E.g. 16,000,000 / (65535 * 1024) = 0.238422217
  // 16,000,000 / (65535 * 8) = 30.518
  // 16,000,000 / (65535 * 1) = 244.14435
  // Verified using https://eleccelerator.com/avr-timer-calculator/
  if (timerFrequencyHz <= 0.95) {
    if (timerFrequencyHz <= 0.2384) {
      Serial.println("Warning: frequency is too low!");
    }
    prescaler.prescaler = 1024;
    prescaler.csRegisterValue = (1 << CS10) | (1 << CS12);
  } else if (timerFrequencyHz > 0.95 && timerFrequencyHz <= 3.814) {
    prescaler.prescaler = 256;
    prescaler.csRegisterValue = (1 << CS12);
  } else if (timerFrequencyHz > 3.814 && timerFrequencyHz <= 30.5176) {
    prescaler.prescaler = 64;
    prescaler.csRegisterValue = (1 << CS10) | (1 << CS11);
  } else if (timerFrequencyHz > 30.5176 && timerFrequencyHz <= 244.14) {
    prescaler.prescaler = 8;
    prescaler.csRegisterValue = (1 << CS11);
  } else {
    prescaler.prescaler = 1;
    prescaler.csRegisterValue |= (1 << CS10);
  }
  return prescaler;
}

void startTimer(const double timerFrequency){
  Serial.print("Setting frequency to ");
  Serial.println(timerFrequency);
  noInterrupts();
  const Prescaler prescaler = determinePrescaler(timerFrequency);
  
  unsigned short ticks = (clockFrequencyHz / (prescaler.prescaler * timerFrequency));
  Serial.println("ticks = ");
  Serial.println(ticks);
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  OCR1A = ticks;
  TCCR1B |= prescaler.csRegisterValue;
  TIMSK1 |= (1 << OCIE1A);
  TCCR1B |= (1 << WGM12);
  
  interrupts();
}


ISR(TIMER1_COMPA_vect){
   digitalWrite(LED_PIN, digitalRead(LED_PIN) ^ 1);
}

