
void setup()
{
  for (int i = 0; i <= 13; ++i)
  {
    pinMode(i, OUTPUT);
    digitalWrite(i, LOW);
  }
  pinMode(A3, OUTPUT);
  digitalWrite(A3, LOW);
  pinMode(A5, OUTPUT);
  digitalWrite(A5, LOW);
}

/*
   0x01
0x20  0x02
   0x40
0x10  0x04
   0x08    0x80
*/

unsigned char font[16] = {
  0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 
  0x77 | 0x80, 0x7C | 0x80, 0x39 | 0x80, 0x5F | 0x80, 0x79 | 0x80, 0x71 | 0x80
};
unsigned char fontPin[8] = {
  10, 9, 8, 7, 6, 5, 4, 2
};
unsigned char digitPin[3] = {
  A3, 0, 1
};
unsigned char displayBits[3];
int outDigit = 0;

void setNumbers(int i)
{
  int hundreds = i / 100;
  i = i - (hundreds * 100);
  int tens = i / 10;
  i = i - (tens * 10);
  int ones = i;
  displayBits[0] = font[hundreds];
  displayBits[1] = font[tens];
  displayBits[2] = font[ones];
}

void tickOutput()
{
  digitalWrite(digitPin[outDigit], LOW);
  ++outDigit;
  if (outDigit == 3)
  {
    outDigit = 0;
  }
  for (int i = 0; i < 8; ++i)
  {
    if (displayBits[outDigit] & (1 << i))
    {
      digitalWrite(fontPin[i], HIGH);
    }
    else
    {
      digitalWrite(fontPin[i], LOW);
    }
  }
  digitalWrite(digitPin[outDigit], HIGH);
}

static float filterVoltage = 3.7f;
static bool isCharging = true;

void tickChargeController()
{
  int i = analogRead(A4);
  float val = i * 5.35f / 1023;
  filterVoltage = filterVoltage * 0.997f + val * 0.003f;
  if (filterVoltage > 4.1f)
  {
    isCharging = false;
  }
  else if (filterVoltage < 4.0f)
  {
    isCharging = true;
  }
  digitalWrite(A5, isCharging ? HIGH : LOW);
}

static int lastNumbers = 0;

void tickChargeDisplay()
{
  /* display detected voltage */
  int num = (int)(filterVoltage * 100);
  /* Some hysteresis in display to avoid psycho digits.
     Use 0.6f as the quanta because that maps to a little 
     more than one least significant bit of the ADC. */
  if ((int)(filterVoltage * 100 + 0.6f) == lastNumbers)
  {
    num = lastNumbers;
  }
  else if ((int)(filterVoltage * 100 - 0.6f) == lastNumbers)
  {
    num = lastNumbers;
  }
  lastNumbers = num;
  setNumbers(num);
  /* set the decimal point */
  displayBits[0] |= 0x80;
}

void loop()
{
  /* important! don't overcharge battery! */
  tickChargeController();
  /* display charge voltage */
  tickChargeDisplay();
  
  /* tick the display, as each digit needs refresh */
  tickOutput();
}





















































































































