// https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/
// https://www.instructables.com/Arduino-Timer-Interrupts/

constexpr unsigned char SIGNAL_PIN = 2;
constexpr unsigned short MAX_PROXIMITY_CM = 400;
constexpr unsigned short MIN_PROXIMITY_CM = 3;

static volatile char DoPing = 0;

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

void ResetPingState()
{
  pinMode(SIGNAL_PIN, OUTPUT);
  digitalWrite(SIGNAL_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(SIGNAL_PIN, HIGH);
  delayMicroseconds(5);
  digitalWrite(SIGNAL_PIN, LOW);
  pinMode(SIGNAL_PIN, INPUT);
}

ISR(TIMER1_COMPA_vect)
{
  DoPing = 1;
}

void HandleRead()
{
  // Speed of sound = 343 m/s == 34,300 cm/s
  // 34,300 / 1*10^-6 = 0.0343 cm/us
  // 1 / 0.0343 = 29.1 us/cm
  unsigned long durationUs = pulseIn(SIGNAL_PIN, HIGH);
  unsigned short distanceCm = (durationUs / 2) / 29;

  HandleProximity(distanceCm);
}

void HandleProximity(const unsigned short distanceCm)
{
  if (distanceCm <= MAX_PROXIMITY_CM && distanceCm >= MIN_PROXIMITY_CM)
  {
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.print(distanceCm);
    Serial.println(" cm");
  }
  else
  {
    digitalWrite(LED_BUILTIN, LOW);
  }
}

void setup()
{
  noInterrupts();
  InitializeTimers();
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  interrupts();
}

void loop()
{
  if (DoPing) {
    ResetPingState();
    HandleRead(); 
    DoPing = 0;
  }
}