    
#include <LiquidCrystal.h>
#include <Wire.h>
#include <DateTime.h>
#include <Menu.h>
#include <Settings.h>

//  constants
#define LED_PIN 13
#define RELAY_PIN 12

#define SOLID_BLINK_TIME 800

#define BAD_BUTTON() \
    blinkTimes(2, 200, 400)

    #define BAD_STATE() \
    blinkTimes(2, 400, 200)
#define BAD_EEPROM() \
    blinkTimes(3, 200, 400)
    
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
PROGMEM char ps_SetRunTime[] =      "1. Run Duration";
PROGMEM char ps_SetDayOfMonth[] =   "2. Run Month-day";
PROGMEM char ps_SetDayOfWeek[] =    "3. Run Weekday";
PROGMEM char ps_SetRunAtTime[] =    "4. Run at Time";
PROGMEM char ps_SetTime[] =         "5. Set Time";
PROGMEM char ps_SetDate[] =         "6. Set Date";
PROGMEM char ps_RunTimePage[] =     "Run Time: HH:MM";
PROGMEM char ps_AdjustControls[] =  "Back  -  +  Next";
PROGMEM char ps_EEPROMDead[] =      "Timer is broken.";
PROGMEM char ps_DayOfMonthPage[] =  "Run on date: Xx";
PROGMEM char ps_DayOfWeekPage[] =   "Run on day: Xxx";
PROGMEM char ps_TimePage[] =        "  Time:  HH:MM  ";
PROGMEM char ps_DatePage[] =        "Date: 20YY-MM-DD";

PROGMEM char ps_DaysOfWeek[] =  "---\0Sun\0Mon\0Tue\0Wed\0Thu\0Fri\0Sat";

//  globals
DateTime lastDateTime;
unsigned long lastSeconds;
unsigned long onUntilTime;
unsigned long lastMillis;
bool colonBlink;
bool wasCancelled;
unsigned long loopMillis;

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
unsigned int readKey(unsigned int key, char serialKey)
{
    //    allow keyboard-driven debugging (ultimate in laziness!)
    if (serialKey == '1' + key - A0) {
        return BTN_ACTIVE;
    }
    if (analogRead(key) > 500) {
        return BTN_ACTIVE;
    }
    return BTN_INACTIVE;
}

//  Is any button pressed? (for start-up UI)
bool anyKeyDown()
{
    char ch = 0;
    /*
    if (Serial.available())
    {
        ch = Serial.read();
    }
    */
    return readKey(A0, ch) == BTN_ACTIVE || 
        readKey(A1, ch) == BTN_ACTIVE || 
        readKey(A2, ch) == BTN_ACTIVE || 
        readKey(A3, ch) == BTN_ACTIVE;
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
            //  cancel the active on-time
            wasCancelled = true;
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
NavPage nm4(&first, ps_SetRunAtTime);
NavPage nm5(&first, ps_SetTime);
NavPage nm6(&first, ps_SetDate);

#include <SetRunTime.h>
SetRunTimeText srtText;
SetGenericAction srtAction(&srtText);
Page setRunTimePage(&nm1, &srtText, &srtAction);

#include <SetDayOfMonth.h>
SetDayOfMonthText sdomText;
SetGenericAction sdomAction(&sdomText);
Page setDayOfMonthPage(&nm2, &sdomText, &sdomAction);

#include <SetDayOfWeek.h>
SetDayOfWeekText sdowText;
SetGenericAction sdowAction(&sdowText);
Page setDayOfWeekPage(&nm3, &sdowText, &sdowAction);

#include <SetRunAtTime.h>
SetRunAtTimeText srunAtTimeText;
SetGenericAction srunAtTimeAction(&srunAtTimeText);
Page setRunAtTimePage(&nm4, &srunAtTimeText, &srunAtTimeAction);

#include <SetTime.h>
SetTimeText stimeText;
SetGenericAction stimeAction(&stimeText);
Page setTimePage(&nm5, &stimeText, &stimeAction);

#include <SetDate.h>
SetDateText sdateText;
SetGenericAction sdateAction(&sdateText);
Page setDatePage(&nm6, &sdateText, &sdateAction);


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
    /*
    Serial.print("blinkTimes() ");
    Serial.println(times, DEC);
    */
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
    /*
    if (Serial.available())
    {
        if (Serial.read() == 'R')
        {
            return true;
        }
    }
    */
    return readKey(A0, 0) == BTN_ACTIVE && readKey(A3, 0) == BTN_ACTIVE;
}


void setup()
{
    //  Turn on indicator, turn off relay
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);
    delay(10);

    /*
    //  Initialize serial port  
    Serial.begin(9600); // for debugging
    delay(10);
    */

    //  Initialize settings; read from EEPROM  
    new ((void *)prefs_) Settings<Prefs>();
    prefs.load();
    delay(10);

    //  Initialize LCD  
    new ((void *)lcd_) LiquidCrystal(2, 3, 4, 5, 6, 7);
    delay(10);
    //  print the firmware version
    lcd.begin(16, 2);  //  for display
    lcd.clear();
    /*
    Serial.println("Firmware " __TIME__ " " __DATE__);
    */
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
        /*
        if (timeNotOk) Serial.println("timeNotOk");
        if (eeNotOk) Serial.println("eeNotOk");
        if (isReset) Serial.println("isReset");
        */

        //  for debugging, to force a "bad eeprom" checksum, uncomment this
        //  prefs.nukeStore();

        // For some reason, reset the settings    
        delay(1500);
        lcd.clear();
        paintProgmem(lcd, ps_ReplaceBattery, 0, 0);
        paintProgmem(lcd, ps_AndSetClock, 0, 1);

        prefs->startMday = 0;
        prefs->startWday = 1;  //  by default, start running Sundays
        prefs->startHour = 0x20;  //  by default, start at some time
        prefs->startMinute = 0x15;  //  "
        prefs->runHours = 0x04;  //  by default, run 4 hrs 30 minutes
        prefs->runMinutes = 0x30;  //  "
        
        waitForAnyButtonPress();

        lcd.clear();
        paintProgmem(lcd, ps_PleaseWait, 0, 0);
        DateTime dt = {
            0x00, 0x35, 0x22, 2, 0x10, 0x10, 0x11
        };
        writeTime(dt);
        prefs.save();
        delay(500);
        prefs.load();
        if (!prefs.lastOk())
        {
            //  This probably means that the eeprom is dead
            /*
            Serial.println("prefs failed");
            */
            lcd.clear();
            paintProgmem(lcd, ps_EEPROMDead, 0, 0);
            BAD_EEPROM();
        }
    }

    //  OK, we're done!
    lcd.clear();
    menu.reset();
    /*
    Serial.println("end setup()");
    */
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

unsigned long calculateTimerEndTime()
{
    //  I want to start, even if I missed the power-on date, for long
    //  run times. This gets annoyingly fiddly.
    //  Start by computing the interval it should be on, if it starts today.
    //  Then, while the interval still includes the current time, check 
    //  each previous day for whether the timer should be on.
    //  This properly handles intervals that wrap across midnight, intervals 
    //  longer than a day, and powering on in the middle of an on-interval.

    unsigned long stime = yearStart(lastDateTime.year) +     //    start time today
    monthStart(lastDateTime.year, lastDateTime.month) +
    dayStart(lastDateTime.mday) +
    hourStart(prefs->startHour) +
    minuteStart(prefs->startMinute);

    unsigned long etime = stime + prefsRunTime();            //    end time

    DateTime checkDateTime = lastDateTime;

    while (etime > lastSeconds)
    {
        //  stime is the start time, if it started on the day in checkDateTime
        if (stime <= lastSeconds)
        {
            //  ok, the current time is within the interval -- 
            //  return running if the given start day matches the specification
            if ((checkDateTime.mday == prefs->startMday) || (checkDateTime.wday == prefs->startWday))
            {
                return etime;
            }
        }

        stime -= 86400;
        etime -= 86400;
        toTime(stime, checkDateTime);
    }
    //    return 0 if there's no need to be on
    return 0;
}


//  globals shared within the context of loop
bool changed;
bool colonIsBlinked;
char buf[70];

//  from within loop, handle timer on/off events
void updatePeriodical()
{
    lastMillis = loopMillis;
    readTime(lastDateTime);
    unsigned long ls = fromTime(lastDateTime);
    if (ls != lastSeconds)
    {
        lastSeconds = ls;
        changed = true;
    }
    unsigned long etime = calculateTimerEndTime();
    //    keep running if already on
    unsigned long otime = onUntilTime;
    if (etime > otime)
    {
        //    should be on -- so set new on-until-time to end time
        otime = etime;
    }
    if (otime <= lastSeconds)
    {
        /*
        if (etime != 0)
        {
            //    shouldn't be on and off at the same time!
            Serial.print("On and Off? etime=");
            Serial.print(etime);
            Serial.print("; otime=");
            Serial.println(otime);
        }
        */
        //    should be off -- so make sure we signal off-ness
        otime = 0;
        wasCancelled = false;
    }
    if (otime != onUntilTime)
    {
        //    switched state
        if (otime == 0 || !wasCancelled)
        {
            /*
            Serial.print("onUntilTime to "); Serial.print(otime); Serial.print(" from "); Serial.println(onUntilTime);
            */
            onUntilTime = otime;
            changed = true;
            if (otime == 0)
            {
                turnOff();
            }
            else
            {
                //    if not manually turned off, turn on!
                turnOn();
            }
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

    char ch = 0;
    /*
    if (Serial.available())
    {
        ch = Serial.read();
        Serial.print(ch);
    }
    */
    //  tell the menu about our button state  
    menu.button(0, readKey(A0, ch));
    menu.button(1, readKey(A1, ch));
    menu.button(2, readKey(A2, ch));
    menu.button(3, readKey(A3, ch));

    //  and update the UI, run any requested button actions
    menu.step();
}

void loop()
{
    //  Say, kids, what time is it?
    loopMillis = millis();
    changed = false;  //  did I change anything on the display?

    //  colon blinkage
    colonIsBlinked = ((loopMillis % 1000) < SOLID_BLINK_TIME);
    if (colonIsBlinked != colonBlink)
    {
        colonBlink = colonIsBlinked;
        changed = true;
    }

    //  check for timers and time/date changing, every so often
    if ((long)loopMillis - (long)lastMillis >= 100)
    {
        updatePeriodical();
    }

    updateMenu();
}


