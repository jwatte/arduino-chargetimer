
#include <DateTime.h>
#include <Wire.h>

#define VERIFY(x, y) \
  if (!((x) == (y))) { \
    Serial.println(""); \
    Serial.print("line: "); \
    Serial.print(__LINE__); \
    Serial.print(" Failed: " #x " == " #y " ; "); \
    Serial.print((long)x); \
    Serial.print(" != "); \
    Serial.println((long)y); \
    return false; \
  }
  
void setup()
{
  Serial.begin(9600);
  pinMode(13, OUTPUT);
  Serial.print("started "); Serial.println(__LINE__);
}

unsigned char x = 1;
int nTested = 0;
int nPassed = 0;
DateTime dt = { 0x30, 0x20, 0x18, 2, 0x12, 0x09, 0x11 };

bool test(char const *fmt, char const *r)
{
  char buf[5];
  fmtTime(buf, 5, fmt, dt);
  return strcmp(buf, r) == 0;
}

bool test_Y()
{
  return test("%Y", "2011");
}

bool test_y()
{
  return test("%y", "11");
}

bool test_m()
{
  return test("%m", "09");
}

bool test_d()
{
  return test("%d", "12");
}

bool test_H()
{
  return test("%H", "18");
}

bool test_M()
{
  return test("%M", "20");
}

bool test_S()
{
  return test("%S", "30");
}

bool test_a()
{
  return test("%a", "Mon");
}

bool test_b()
{
  return test("%b", "Sep");
}

bool test_pct()
{
  return test("%%", "%");
}

PROGMEM prog_char tProg[] = "%Y-%m-%d %H:%M:%S";

bool test_pgm()
{
  char oot[30];
  fmtTime_P(oot, 30, tProg, dt);
  return strcmp(oot, "2011-09-12 18:20:30") == 0;
}

bool test_yearStart()
{
  VERIFY(yearStart(0x09), 0);
  VERIFY(yearStart(0x10), 365*86400);
  VERIFY(yearStart(0x11), 2*365*86400);
  VERIFY(yearStart(0x12), 3*365*86400);
  VERIFY(yearStart(0x13), (4*365+1)*86400);
  return true;
}

bool test_monthStart()
{
  VERIFY(monthStart(0x09, 0x03), (31+28)*86400);
  VERIFY(monthStart(0x10, 0x03), (31+28)*86400);
  VERIFY(monthStart(0x11, 0x03), (31+28)*86400);
  VERIFY(monthStart(0x12, 0x03), (31+29)*86400);
  VERIFY(monthStart(0x13, 0x03), (31+28)*86400);
  return true;
}

bool test_dayStart()
{
  VERIFY(dayStart(0x01), 0);
  VERIFY(dayStart(0x31), 30*86400);
  return true;
}

bool test_fromTime()
{
  DateTime dt = { 0x00, 0x00, 0x00, 1, 0x01, 0x01, 0x09 };
  unsigned long l = fromTime(dt);
  VERIFY(l, 0);
  dt.second = 3;
  dt.minute = 0x15;
  dt.hour = 7;
  dt.mday = 2;
  dt.month = 3;
  l = fromTime(dt);
  unsigned long want = 3 + 15*60 + 7 * 3600 + (31 + 28 + 1)*86400;
  VERIFY(l, want);
  want += (3 * 365 + 1) * 86400;
  dt.year += 3;
  l = fromTime(dt);
  VERIFY(l, want);
  return true;
}

bool test_toTime()
{
  unsigned long want = 3 + 15*60 + 7 * 3600 + (31 + 28 + 1)*86400;
  DateTime dt = { 0 };
  toTime(want, dt);
  VERIFY(dt.year, 0x09);
  VERIFY(dt.month, 0x03);
  VERIFY(dt.mday, 0x2);
  VERIFY(dt.hour, 0x07);
  VERIFY(dt.minute, 0x15);
  VERIFY(dt.second, 0x03);
  VERIFY(dt.wday, 0x02);
  return true;
}

void runOneTest(char const *n, bool (*f)())
{
  Serial.print("Testing: ");
  Serial.print(n);
  bool ok = (*f)();
  nTested++;
  if (ok)
  {
    nPassed++;
    Serial.println(" OK");
  }
  else
  {
    Serial.println(" FAIL");
  }
}

#define TEST(x) \
  runOneTest(#x, x)

void runTests()
{
  TEST(test_Y);
  TEST(test_y);
  TEST(test_m);
  TEST(test_d);
  TEST(test_H);
  TEST(test_M);
  TEST(test_S);
  TEST(test_a);
  TEST(test_b);
  TEST(test_pct);
  TEST(test_pgm);
  TEST(test_yearStart);
  TEST(test_monthStart);
  TEST(test_dayStart);
  TEST(test_fromTime);
  TEST(test_toTime);
}

void loop()
{
  if (nTested == 0)
  {
    runTests();
  }
  x = 1 - x;
  digitalWrite(13, x);
  delay(x ? (100 * (nPassed + 1)) : (100 * (nTested - nPassed + 1)));
}

