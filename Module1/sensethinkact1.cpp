// https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/
// https://www.instructables.com/Arduino-Timer-Interrupts/

constexpr unsigned char SIGNAL_PIN = 2;
constexpr unsigned short MAX_PROXIMITY_CM = 400;
constexpr unsigned short MIN_PROXIMITY_CM = 3;

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

void DoRead()
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
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop()
{
  ResetPingState();
  // References: https://docs.arduino.cc/built-in-examples/sensors/Ping
  DoRead();
  delay(100);
}