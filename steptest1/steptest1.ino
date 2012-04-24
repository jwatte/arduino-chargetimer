
void setup()
{
  for (int i = 2; i < 14; ++i)
  {
    pinMode(i, OUTPUT);
    digitalWrite(i, LOW);
  }
}

int sdelay = 0;

void ctl(int phase, int pin) {
  digitalWrite(pin, LOW);
  digitalWrite(pin+1, LOW);
  digitalWrite(pin+2, LOW);
  digitalWrite(pin+3, LOW);
  digitalWrite(13, LOW);
  switch (phase & 7) {
    case 0: digitalWrite(pin, HIGH); digitalWrite(13, HIGH); break;
    case 1: digitalWrite(pin, HIGH); digitalWrite(13, HIGH); digitalWrite(pin+1, HIGH); break;
    case 2: digitalWrite(pin+1, HIGH); break;
    case 3: digitalWrite(pin+1, HIGH); digitalWrite(pin+2, HIGH); break;
    case 4: digitalWrite(pin+2, HIGH); break;
    case 5: digitalWrite(pin+2, HIGH); digitalWrite(pin+3, HIGH); break;
    case 6: digitalWrite(pin+3, HIGH); break;
    case 7: digitalWrite(pin+3, HIGH); digitalWrite(pin, HIGH); break;
  }
}

class Target
{
  public:
    Target(int p, int d) : pin(p), pos(0), dest(0), start(0), delay(0), deccel(d) {}
    void step() {
      if (delay > 0) {
        --delay;
        return;
      }
      if (pos < dest) {
        ++pos;
      }
      else if (pos > dest) {
        --pos;
      }
      else {
        start = dest;
        dest = rand() & 1023;
        delay = 400;
        return;
      }
      ctl(pos, pin);
      calcdelay();
    }
    void calcdelay()
    {
      int n = start - pos;
      if (n < 0) n = -n;
      int m = dest - pos;
      if (m < 0) m = -m;
      if (m < n) n = m;
      m = deccel + 2 - n;
      if (m < 2) {
        delay = 2;
      }
      else {
        delay = m;
      }
    }
    int pin;
    int pos;
    int dest;
    int start;
    int delay;
    int deccel;
};

Target a(2, 5);
Target b(6, 5);

void loop()
{
  a.step();
  b.step();
  delayMicroseconds(700);
}

