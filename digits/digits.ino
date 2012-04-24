
void setup() {
  for (int i = 2; i <= 13; ++i) {
    pinMode(i, OUTPUT);
    digitalWrite(i, LOW);
  }
  Serial.begin(57600);
}

unsigned char digitBits[16] = {
  0xd7,
  0x11,
  0xcd,
  0x5d,
  0x1b,
  0x5e,
  0xde,
  0x15,
  0xdf,
  0x5f,
  0xaf,
  0xda,
  0xc6,
  0xd9,
  0xce,
  0x8e
};
unsigned char bits[3] = { 1, 0xf, 0xff };

int curDigit = 0;  //  which digit is being refreshed
int curPos = 0;    //  what the last rotary reading was
int delta = 0;     //  accumulator for rotary reading
int minutes = 0;   //  what to display
bool isAlarm;

long millisecondsEnd = 0;

void alarm() {
  isAlarm = true;
}

void setMinutes(int mins) {
  minutes = mins;
  millisecondsEnd = millis() + ((long)minutes * 60000L) + 999L;
  isAlarm = false;
  Serial.println(minutes);
}

void loop() {
  for (int i = 6; i <= 13; ++i) {
    digitalWrite(i, 0);
  }
  digitalWrite(2 + curDigit, LOW);
  curDigit++;
  if (curDigit == 3) {
    curDigit = 0;
  }
  if (!isAlarm || (millis() & 0x80)) {
    digitalWrite(2 + curDigit, HIGH);
    for (int i = 6; i <= 13; ++i) {
      digitalWrite(i, (bits[curDigit] & (1 << (i - 6))) ? HIGH : LOW);
    }
    digitalWrite(5, LOW);
  }
  else {
    analogWrite(5, 50);
  }
  
  bool curA = analogRead(A0) > 500;
  bool curB = analogRead(A1) > 500;
  curA = curA ^ curB;
  int newPos = (curA ? 1 : 0) | (curB ? 2 : 0);
  if (((newPos - curPos) & 3) == 1) {
    delta = delta + 1;
  }
  if (((newPos - curPos) & 3) == 3) {
    delta = delta - 1;
  }
  if (delta >= 4) {
    delta = 0;
    minutes = minutes + 5;
    if (minutes > 120) {
      minutes = 120;
    }
    setMinutes(minutes);
  }
  if (delta <= -4) {
    delta = 0;
    minutes = minutes - 1;
    if (minutes < 0) {
      minutes = 0;
    }
    setMinutes(minutes);
  }
  long millisecondsLeft = millisecondsEnd - millis();
  if (millisecondsEnd) {
    if (millisecondsLeft <= 0) {
      alarm();
      minutes = 0;
      millisecondsEnd = 0;
    }
    else {
      minutes = millisecondsLeft / 60000L;
    }
  }
  if (isAlarm) {
    bits[0] = bits[1] = bits[2] = digitBits[0xe];
  }
  else if (minutes >= 10) {
    bits[0] = digitBits[minutes % 10];
    bits[1] = digitBits[(minutes / 10) % 10];
    bits[2] = digitBits[(minutes / 100) % 10];
  }
  else {
    int seconds = millisecondsLeft / 1000L;
    bits[0] = digitBits[seconds % 10];
    bits[1] = digitBits[(seconds / 10) % 6];
    bits[2] = digitBits[seconds / 60] | 0x20;
  }
  curPos = newPos;
  delay(1);
}

