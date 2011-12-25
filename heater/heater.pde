
/* Simple temperature controlled relay (heater thermostat)
 * Turn on the relay if temperature is < 38 degrees.
 * Run the relay for at most 5 minutes before a 5 minute cooling period.
 * A single-button interface turns on the backlight and lets you switch 
 * read-outs:
 *   Current temperature/status
 *   Min/Max temperature
 *   Elapsed time on
 * Long. elapsed/min/max values
 * The LCD sleeps with no backlight until button pushed
 *
 * Pinout:
 * LCD: digital pins 2, 3, 5, 6, 7, 8, 22k pull-ups
 * Signal LED: n/a
 * Power relay: MOSFET transistor on pin 9, driving relay, 100k pull-down
 * LCD backlight: MOSFET transistor on pin 10, 100k pull-down
 * Pushbutton: digital pin 4, button pulls low, 47k pull-up
 * Temperature sensor: analog input pin 0
 * 47k pull-up for and 1 (serial RX)
 * resonator, power, reset logic
 */
#include <LiquidCrystal.h>
#include <Menu.h>
#include <stdio.h>
#include <Settings.h>
#include <avr/wdt.h>

#define TEMP_APIN 0
#define BTN_DPIN 4
#define RELAY_DPIN 9
#define BACKLIGHT_DPIN 10

#define NUM_PAGES 3

//  After turning on, stay on no longer than this
#define RELAY_ON_MILLIS 300000
//  After turning off because temp went up, wait at list this many millis before turning on again
#define TEMP_RELAY_HYSTERESIS_TIME 300000
//  After turning off, stay off for this long until allowing on again
#define RELAY_OFF_MILLIS 300000
//  After turning backlight on, stay on for this long
#define BACKLIGHT_ON_MILLIS 7000
//  Long button press is button pressed this long
#define LONG_BUTTON_PRESS_MILLIS 1000
//  Button debounce time
#define BUTTON_DEBOUNCE_MILLIS 50
//  Runtime save interval (must be power of two)
#define RUNTIME_SAVE_SECONDS 4096

const float trimMvPerC = 10.0f;
const float trimOffsetC = -7.5f;
const float triggerTempF = 38.0f;
//  If below this temperature, don't use the off cycle
const float offDisallowedTempF = 30.0f;

inline void *operator new(size_t, void *arg) { return arg; }

LiquidCrystal lcd(2, 3, 5, 6, 7, 8);
Menu menu(lcd);
char displayData[2][20];
float temp = 150.0f;
long lastMillis = 0;
long backlightOnUntil = 0;
long relayOnUntil = 0;
long relayOffUntil = 0;
unsigned char page = 0;
bool buttonState = false;
bool relayIsOn = false;
long buttonDebounceMillis = 0;
long buttonChangeMillis = 0;
long relayOnTime = 0;
long secondsRelayOn = 0;
long millisecondsRelayOn = 0;
float maxTempF = -100;
float minTempF = 200;

struct SavePrefs
{
  unsigned long onSeconds;
  unsigned char version;
};

Settings<SavePrefs> prefs;

void saveRuntime()
{
  prefs->onSeconds = secondsRelayOn;
  prefs->version = 1;
  prefs.save();
}

void setup() {
  wdt_enable(WDTO_8S);
  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("F/w " __DATE__);
  pinMode(BTN_DPIN, INPUT);
  pinMode(RELAY_DPIN, OUTPUT);
  digitalWrite(RELAY_DPIN, 0);
  pinMode(BACKLIGHT_DPIN, OUTPUT);
  digitalWrite(BACKLIGHT_DPIN, 1);
  prefs.load();
  if (!prefs.lastOk())
  {
    //  write as 0
    saveRuntime();
  }
  delay(1000);
  digitalWrite(BACKLIGHT_DPIN, 0);
  lastMillis = millis();
  wdt_enable(WDTO_2S);
}

float numToTempF(float num)
{
      //  750 mV at 25 C
  //  10 mV per C
  //  5000 mV reference == 1023
  float mV = 5000 * num / 1023;
  float C = 25 + (mV - 750) / trimMvPerC + trimOffsetC;
  return C * 9 / 5 + 32;
}

void accumulateRelayTime(unsigned long dTime)
{
  millisecondsRelayOn += dTime;
  if (millisecondsRelayOn >= 1000)
  {
    unsigned long delta = millisecondsRelayOn / 1000;
    secondsRelayOn += delta;
    millisecondsRelayOn -= delta * 1000;
    if (!(secondsRelayOn & (RUNTIME_SAVE_SECONDS - 1)))
    {
      saveRuntime();
    }
  }
}

void presentFix(short fix, char *dst)
{
  if (fix < 0)
  {
    *dst++ = '-';
  }
  else if (fix >= 1000)
  {
    *dst++ = (fix / 10) + '0';
  }
  else
  {
    *dst++ = ' ';
  }
  *dst++ = (fix / 100) % 10 + '0';
  *dst++ = (fix / 10) % 10 + '0';
  *dst++ = '.';
  *dst++ = fix % 10 + '0';
}

void printTempPage(char *temp, float f)
{
  memcpy(temp, "Temp:       F", 13);
  short sh = (short)(f * 10);
  presentFix(sh, &temp[6]);
}

void printMinMaxPage(char *minMax)
{
  presentFix((short)(minTempF * 10), &minMax[2]);
  presentFix((short)(maxTempF * 10), &minMax[10]);
  minMax[8] = '-';
}

char *iprint(char *dst, unsigned long i)
{
  unsigned long mod = 1000000000;
  bool on = false;
  while (mod != 0)
  {
    if (i >= mod || on || mod == 1)
    {
      *dst++ = '0' + i / mod;
      on = true;
    }
    mod = mod / 10;  
  }
  return dst;
}

char *sprint(char *dst, char const *s)
{
  while (*s)
  {
    *dst++ = *s++;
  }
  return dst;
}


void printRuntimePage(char *runtime)
{
  unsigned long hrs = secondsRelayOn / 3600;
  runtime = iprint(runtime, hrs);
  runtime = sprint(runtime, "h ");
  unsigned long mins = (secondsRelayOn - hrs * 3600) / 60;
  runtime = iprint(runtime, mins);
  runtime = sprint(runtime, "m ");
}

void loop() {

  delay(10);

  unsigned short tempval = analogRead(TEMP_APIN);
  temp = temp * 0.99f + tempval * 0.01f;
  unsigned short btnval = digitalRead(BTN_DPIN);
  float tempF = numToTempF(temp);
  lastMillis = millis();
  if (tempF > maxTempF)
  {
    maxTempF = tempF;
  }
  if (tempF < minTempF)
  {
    minTempF = tempF;
  }

  if (relayOffUntil != 0)
  {
    if (relayOffUntil - lastMillis <= 0)
    {
      relayOffUntil = 0;
    }
  }
  if (relayOnUntil != 0)
  {
    if (relayOnUntil - lastMillis <= 0)
    {
      relayOnUntil = 0;
      if (tempF > offDisallowedTempF)
      {
        relayOffUntil = RELAY_OFF_MILLIS;
      }
    }
  }
  if (backlightOnUntil != 0)
  {
    if (backlightOnUntil - lastMillis <= 0)
    {
      backlightOnUntil = 0;
      page = 0;
    }
  }

  if (tempF <= triggerTempF)
  {
    if (relayOffUntil == 0 && relayOnUntil == 0)
    {
      relayOnUntil = lastMillis + RELAY_ON_MILLIS;
    }
  }
  else if (relayOnUntil != 0)
  {
    relayOnUntil = 0;
    // don't cycle the relay too often
    relayOffUntil = TEMP_RELAY_HYSTERESIS_TIME;
  }
  
  bool btn = digitalRead(BTN_DPIN) == 0;
  if (btn != buttonState)
  {
    //  todo: first press should turn on light;
    //  further presses should switch page
    backlightOnUntil = lastMillis + BACKLIGHT_ON_MILLIS;
    if (buttonDebounceMillis == 0)
    {
      buttonDebounceMillis = lastMillis + BUTTON_DEBOUNCE_MILLIS;
    }
    else if (buttonDebounceMillis - lastMillis <= 0)
    {
      buttonState = btn;
      if (buttonState)
      {
        buttonChangeMillis = lastMillis;
      }
    }
  }

  if (buttonChangeMillis != 0)
  {
    if (buttonState)
    {
      if (lastMillis - buttonChangeMillis > LONG_BUTTON_PRESS_MILLIS)
      {
        // long-press
        buttonChangeMillis = 0;
        minTempF = tempF;
        maxTempF = tempF;
        page = 1;
        saveRuntime();
      }
    }
    else
    {
      buttonChangeMillis = 0;
      //  short press
      page = (page + 1) % NUM_PAGES;
    }
  }

  char data[17] = "                ";
  switch (page)
  {
    case 0:
      printTempPage(data, tempF);
      break;
    case 1:
      printMinMaxPage(data);
      break;
    case 2:
      printRuntimePage(data);
      break;
  }
  deltaUpdate(lcd, data, displayData[0], 0, 0);
  deltaUpdate(lcd, relayOffUntil != 0 ? "Waiting" :
    relayOnUntil != 0 ? "Heating" : "Idle",
    displayData[1], 0, 1);

  if (relayIsOn)
  {
    accumulateRelayTime(lastMillis - relayOnTime);
  }
  
  if (relayOnUntil != 0)
  {
    digitalWrite(RELAY_DPIN, 1);
    relayIsOn = true;
    relayOnTime = lastMillis;
  }
  else
  {
    digitalWrite(RELAY_DPIN, 0);
    relayIsOn = false;
  }
  if (backlightOnUntil != 0)
  {
    digitalWrite(BACKLIGHT_DPIN, 1);
  }
  else
  {
    digitalWrite(BACKLIGHT_DPIN, 0);
  }
  
  wdt_reset();
}


