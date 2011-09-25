
#include <LiquidCrystal.h>
#include <Wire.h>
#include <DateTime.h>
#include <Menu.h>
#include <Settings.h>

//  constants
#define LED_PIN 13
#define RELAY_PIN 4

//  forward declarations
void menuExit();
void turnOn();
void turnOff();


//  Strings for the menu and UI
PROGMEM char ps_ReplaceBattery[] =  "Replace Battery";
PROGMEM char ps_AndSetClock[] =     " And Set Clock";
PROGMEM char ps_PleaseWait[] =      "Please Wait...";
PROGMEM char ps_MainPage[] =        "%a %b %d %H:%M";
PROGMEM char ps_Menu[] =            "Menu          On";
PROGMEM char ps_ChargingMenu[] =    "Menu Stop  00:00";
PROGMEM char ps_SetRunTime[] =      "1. Set Run Time";
PROGMEM char ps_SetDayOfMonth[] =   "2. Day of Month";
PROGMEM char ps_SetDayOfWeek[] =    "3. Day of Week";
PROGMEM char ps_SetClock[] =        "4. Set Date/Time";
PROGMEM char ps_RunTimePage[] =     "Run Time: HH:MM";
PROGMEM char ps_AdjustControls[] =  "Back  -  +  Next";
PROGMEM char ps_EEPROMDead[] =      "Timer is broken.";
//  globals
DateTime lastDateTime;
unsigned long lastSeconds;
unsigned long onUntilTime;
unsigned long lastMillis;
bool colonBlink;

//  EEPROM/settings
struct Prefs
{
  unsigned char startMday;
  unsigned char startWday;
  unsigned char startHour;
  unsigned char startMinute;
  unsigned char runHours;
  unsigned char runMinutes;
};
//  initialize prefs in setup, not as a global
#define prefs (*(Settings<Prefs> *)prefs_)
unsigned char prefs_[sizeof(Settings<Prefs>)];

//  My configuration of LCD
unsigned char lcd_[sizeof(LiquidCrystal)];
#define lcd (*(LiquidCrystal *)lcd_)
inline void *operator new(unsigned int sz, void *ptr) { return ptr; }

//  return the number of seconds the prefs want us to run
unsigned long prefsRunTime()
{
  return (unsigned long)b2d(prefs->runMinutes) * 60UL + (unsigned long)b2d(prefs->runHours) * 3600UL;
}

//  return true if the prefs were properly loaded from eeprom (else it's probably an initial run)
bool eepromInvalid()
{
  return !prefs.lastOk();
}

//  Is a particular button pressed? (for start-up UI)
unsigned int readKey(unsigned int key)
{
  return analogRead(key) > 100 ? BTN_UP : BTN_DOWN;
}

//  Is any button pressed? (for start-up UI)
bool anyKeyDown()
{
  return readKey(A0) == BTN_DOWN || 
    readKey(A1) == BTN_DOWN || 
    readKey(A2) == BTN_DOWN || 
    readKey(A3) == BTN_DOWN;
}

//  Convert BCD to decimal
static unsigned char b2d(unsigned char b)
{
  return ((b & 0xf0) >> 4) * 10 + (b & 0xf);
}

//  Top line of main menu/display -- current time
class MainText : public Paint
{
public:
  void paint(char *buf)
  {
    fmtTime_P(buf, 17, ps_MainPage, lastDateTime);
    if (!colonBlink)
    {
        buf[13] = 32;  //  flash the colon
    }
    buf[16] = 0;
  }
};
MainText mainText;

//  Bottom line of main menu/display -- soft buttons, and run time
class MainAction : public Action
{
public:
  virtual void paint(char *buf)
  {
    lcd.setCursor(0, 1);
    if (onUntilTime)
    {
      strcpy_P(buf, ps_ChargingMenu);
      fmtHrsMins(onUntilTime - lastSeconds, &buf[11]);
      if (!colonBlink)
      {
        buf[13] = 32;  //  flash the colon
      }
      buf[16] = 0;
    }
    else
    {
      strcpy_P(buf, ps_Menu);
    }
  }
  void action(unsigned char btn, Menu *m)
  {
    if (btn == 0)
    {
      m->gotoPage(m->firstChild->firstChild);
    }
    else if (onUntilTime != 0 && btn == 1)
    {
      //  cancel the active charge
      onUntilTime = 0;
      turnOff();
    }
    else if (onUntilTime == 0 && btn == 3)
    {
      //  turn on charge
      turnOn();
      onUntilTime = lastSeconds + prefsRunTime();
    }
  }
};
MainAction mainAction;

//  Menus
Menu menu(lcd, &menuExit);
Page first(&menu, &mainText, &mainAction);
NavPage nm1(&first, ps_SetRunTime);
NavPage nm2(&first, ps_SetDayOfMonth);
NavPage nm3(&first, ps_SetDayOfWeek);
NavPage nm4(&first, ps_SetClock);

//  RunTime menu
class SetRunTimeText : public Paint
{
public:
  SetRunTimeText() : set_(false) {}
  bool set_; // was set
  unsigned char hours;
  unsigned char minutes;
  unsigned char *edit;
  void enter()
  {
    hours = prefs->runHours;
    minutes = prefs->runMinutes;
    set_ = false;
    edit = &hours;
  }
  void exit()
  {
    if (set_)
    {
      set_ = false;
      prefs->runHours = hours;
      prefs->runMinutes = minutes;
      prefs.save();
    }
  }
  void paint(char *buf)
  {
    strcpy_P(buf, ps_RunTimePage);
    fmtHrsMins(b2d(hours) * 3600 + b2d(minutes) * 60, &buf[10]);
    if (!colonBlink)
    {
      char *ptr = buf + 10 + 3 * (edit - &hours);
      ptr[0] = ptr[1] = ' ';
    }
    Serial.println(buf);
  }
  void action(unsigned char btn, Menu *m)
  {
    Serial.println(btn, HEX);
    switch (btn)
    {
      case 0:
        if (edit == &minutes)
        {
          edit = &hours;
        }
        else
        {
          m->gotoPage(m->curPage->parent);
        }
        break;
      case 1:
        increment();
        break;
      case 2:
        decrement();
        break;
      case 3:
        if (edit == &hours)
        {
          edit = &minutes;
        }
        else
        {
          set_ = true;
          m->gotoPage(m->curPage->parent);
        }
        break;
    }
  }
  void increment()
  {
    *edit += 1;
    if ((*edit & 0xf) > 9)
    {
      *edit = (*edit & 0xf0) + 0x10;
      if (*edit > (edit == &hours) ? 0x99 : 0x59)
      {
        *edit = 0;
      }
    }
  }
  void decrement()
  {
    *edit -= 1;
    if ((*edit & 0xf) > 9)
    {
      *edit = (*edit & 0xf0) - 0x10;
      unsigned char top = (edit == &hours) ? 0x99 : 0x59;
      if (*edit > top)
      {
        *edit = top;
      }
    }
  }
};
SetRunTimeText srtText;

//  RunTime adjusting
class SetRunTimeAction : public Action
{
public:
  void paint(char *buf)
  {
    strcpy_P(buf, ps_AdjustControls);
  }
  void action(unsigned char btn, Menu *m)
  {
    srtText.action(btn, m);
  }
};
SetRunTimeAction srtAction;

Page setRunTimePage(&nm1, &srtText, &srtAction);


//  Call menuExit() to go back to beginning
void menuExit()
{
  menu.reset();
}

//  During start-up, when detecting a user reset, wait to proceed
void waitForAnyButtonPress()
{
  //  wait for key up, if down
  while (anyKeyDown())
    delay(10);
  //  wait for key down
  while (!anyKeyDown())
    delay(10);
  //  wait for key up
  while (anyKeyDown())
    delay(10);
}


//  Diagnostic blink sequence
void blinkTimes(unsigned char times, unsigned int onTime, unsigned int offTime)
{
  while (true)
  {
    for (unsigned char ix = 0; ix != times; ++ix)
    {
      digitalWrite(LED_PIN, HIGH);
      delay(onTime);
      digitalWrite(LED_PIN, LOW);
      delay(offTime);
    }
    delay(onTime+offTime+1000);
  }
}

//  leftmost and rightmost button brings up user reset
bool isResetKeyCombo()
{
  return readKey(A0) == BTN_DOWN && readKey(A3) == BTN_DOWN;
}


void setup()
{
  //  Turn on indicator, turn off relay
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  delay(10);

  //  Initialize serial port  
  Serial.begin(9600); // for debugging
  delay(10);
  
  //  Initialize settings; read from EEPROM  
  new ((void *)prefs_) Settings<Prefs>();
  prefs.load();
  delay(10);

  //  Initialize LCD  
  new ((void *)lcd_) LiquidCrystal(7, 8, 9, 10, 11, 12);
  delay(10);
  //  print the firmware version
  lcd.begin(16, 2);  //  for display
  lcd.clear();
  Serial.println("Firmware " __TIME__ " " __DATE__);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("F/w " __DATE__);
  lcd.setCursor(3, 1);
  lcd.print(__TIME__);

  //  some time for user to read FW version  
  delay(600);
  //  at this point, Wire should be safe to start
  Wire.begin();  //  for RTC
  //  some time to get I2C time to stabilize (?)
  delay(200);

  //  Is there any reason to not just go to loop()?  
  bool timeNotOk = timeIsInvalid();
  bool eeNotOk = eepromInvalid();
  bool isReset = isResetKeyCombo();
  if (timeNotOk || eeNotOk || isReset)
  {
    if (timeNotOk) Serial.println("timeNotOk");
    if (eeNotOk) Serial.println("eeNotOk");
    if (isReset) Serial.println("isReset");

    //  for debugging, to force a "bad eeprom" checksum, uncomment this
    //  prefs.nukeStore();

    // For some reason, reset the settings    
    delay(1500);
    lcd.clear();
    paintProgmem(lcd, ps_ReplaceBattery, 0, 0);
    paintProgmem(lcd, ps_AndSetClock, 0, 1);

    prefs->startMday = 0;
    prefs->startWday = 1;  //  by default, start running Sundays
    prefs->startHour = 0x20;  //  by default, start at 8:15pm
    prefs->startMinute = 0x15;  //  "
    prefs->runHours = 0x04;  //  by default, run 4 hrs 30 minutes
    prefs->runMinutes = 0x30;  //  "
    
    waitForAnyButtonPress();

    lcd.clear();
    paintProgmem(lcd, ps_PleaseWait, 0, 0);
    DateTime dt = {
      0x00, 0x42, 0x22, 4, 0x21, 0x09, 0x11
    };
    writeTime(dt);
    prefs.save();
    delay(500);
    prefs.load();
    if (!prefs.lastOk())
    {
      //  This probably means that the eeprom is dead
      Serial.println("save and load prefs failed");
      lcd.clear();
      paintProgmem(lcd, ps_EEPROMDead, 0, 0);
      blinkTimes(3, 200, 400);
    }
  }
  
  //  OK, we're done!
  lcd.clear();
  menu.reset();
  Serial.println("end setup()");
  //  turn off the indicator light
  digitalWrite(LED_PIN, LOW);
}

//  Turn off the relay and light
void turnOff()
{
  digitalWrite(LED_PIN, LOW);
  digitalWrite(RELAY_PIN, LOW);
}

//  Turn on the relay and light
void turnOn()
{
  digitalWrite(LED_PIN, HIGH);
  digitalWrite(RELAY_PIN, HIGH);
}

bool calculateTimerOn()
{
  //  I want to start, even if I missed the power-on date, for long
  //  run times. This gets annoyingly fiddly.
  //  Start by computing the interval it should be on, if it starts today.
  //  Then, while the interval still includes the current time, check 
  //  each previous day for whether the timer should be on.
  //  This properly handles intervals that wrap across midnight, intervals 
  //  longer than a day, and powering on in the middle of an on-interval.

  unsigned long stime = yearStart(lastDateTime.year) +
    monthStart(lastDateTime.year, lastDateTime.month) +
    dayStart(lastDateTime.mday) +
    hourStart(prefs->startHour) +
    minuteStart(prefs->startMinute);

  unsigned long etime = stime + prefsRunTime();

  DateTime checkDateTime = lastDateTime;

  while (etime > lastSeconds)
  {
    //  stime is the start time, if it started on the day in checkDateTime
    if (stime <= lastSeconds)
    {
      if ((checkDateTime.mday == prefs->startMday) || (checkDateTime.wday == prefs->startWday))
      {
        return etime;
      }
    }

    stime -= 86400;
    etime -= 86400;
    toTime(stime, checkDateTime);
  }
  return 0;
}


//  globals shared within the context of loop
unsigned long m;
bool changed;
bool cb;

//  from within loop, handle timer on/off events
void updatePeriodical()
{
  lastMillis = m;
  readTime(lastDateTime);
  unsigned long ls = fromTime(lastDateTime);
  if (ls != lastSeconds)
  {
    lastSeconds = ls;
    changed = true;
  }
  unsigned long etime = calculateTimerOn();
  unsigned long otime = onUntilTime;
  if (etime > otime)
  {
    Serial.print("Detect ON: "); Serial.println(etime);
    otime = etime;
  }
  //  todo: if I forced off, don't turn on just yet
  if (otime <= lastSeconds)
  {
    if (otime != 0)
    {
      Serial.print("Detect OFF: "); Serial.println(onUntilTime);
      otime = 0;
    }
  }
  if (otime != onUntilTime)
  {
    Serial.print("Change onUntilTime to "); Serial.print(otime); Serial.print(" from "); Serial.println(onUntilTime);
    onUntilTime = otime;
    changed = true;
    if (otime == 0)
    {
      turnOff();
    }
    else
    {
      turnOn();
    }  
  }
}

//  from within loop(), handle menu display and interaction
void updateMenu()
{
  //  If anything changed, invalidate the display
  if (changed)
  {
    menu.invalidate();
  }

  //  tell the menu about our button state  
  menu.button(0, readKey(A0));
  menu.button(1, readKey(A1));
  menu.button(2, readKey(A2));
  menu.button(3, readKey(A3));
  
  //  and update the UI, run any requested button actions
  menu.step();
}

void loop()
{
  //  Say, kids, what time is it?
  m = millis();
  changed = false;  //  did I change anything on the display?

  //  colon blinkage
  cb = ((m % 1000) < 800);
  if (cb != colonBlink)
  {
     colonBlink = cb;
     changed = true;
  }

  //  check for timers and time/date changing, every so often
  if ((long)m - (long)lastMillis >= 100)
  {
    updatePeriodical();
  }
  
  //  don't run out of control (?)
  delay(5);

  updateMenu();
}


