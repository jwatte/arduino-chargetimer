
#include "DateTime.h"
#include <WProgram.h>
#include <Wire.h>
#include <string.h>

#define RTC 0x68
#define SECONDS_PER_MINUTE 60L
#define SECONDS_PER_HOUR 3600UL
#define SECONDS_PER_DAY 86400UL
#define SECONDS_PER_YEAR (SECONDS_PER_DAY*365UL)

static char dateScratch[21];

//  there might be some instruction intrinsic for this...
static unsigned long b2d(unsigned char bcd)
{
    return ((bcd & 0xf0) >> 4) * 10 + (bcd & 0xf);
}

//  there might be some instruction intrinsic for this...
static unsigned long d2b(unsigned char d)
{
    unsigned char t = d / 10;
    return d - (t * 10) + (t << 4);
}

//  there might be some library function for this...
void bcdDec(char *dst, unsigned char o, unsigned char b)
{
  dst[o] = (b >> 4) + '0';
  dst[o+1] = (b & 0xf) + '0';
}

void readTime(DateTime &o)
{
  Wire.beginTransmission(RTC);
  Wire.send(0);
  Wire.endTransmission();
  Wire.requestFrom(RTC, 7);
  o.second = Wire.receive() & 0x7f;
  o.minute = Wire.receive();
  o.hour = Wire.receive() & 0x3f;
  o.wday = Wire.receive();
  o.mday = Wire.receive();
  unsigned char c = Wire.receive();
  o.month = c & 0x1f;
  o.year = Wire.receive();
  //    c & 0x80 is "century" flag for 2100+
}

void writeTime(DateTime const &i)
{
    Wire.beginTransmission(RTC);
    Wire.send(0);
    Wire.send(i.second);
    Wire.send(i.minute);
    Wire.send(i.hour);
    Wire.send(i.wday);
    Wire.send(i.mday);
    Wire.send(i.month);
    Wire.send(i.year);
    Wire.endTransmission();
    
    Wire.beginTransmission(RTC);
    Wire.send(0xe); // control
    Wire.send(0);
    Wire.send(0);   // clear time stopped flag
    Wire.endTransmission();
}

bool timeIsInvalid()
{
    Wire.beginTransmission(RTC);
    Wire.send(0xF); // status
    Wire.endTransmission();
    Wire.requestFrom(RTC, 1);
    unsigned char val = Wire.receive();
    return (val & 0x80) != 0;
}


unsigned long fromTime(DateTime const &i)
{
    unsigned long yt = yearStart(i.year) +
        monthStart(i.year, i.month) +
        dayStart(i.mday) +
        hourStart(i.hour) +
        minuteStart(i.minute) +
        i.second;
    return yt;
}

PROGMEM prog_char mTable[] = {
    31,
    28,
    31,
    30,
    31,
    30,
    31,
    31,
    30,
    31,
    30,
    31
};

//  i is seconds since 2009 in decimal
//  y is years since 2000 in decimal
void toTime(unsigned long i, DateTime &o)
{
    //  Jan 1, 2009, is a Thursday
    o.wday = (4 + i / 86400) % 7 + 1;
    
    //  take a guess at the year
    unsigned char y = i / SECONDS_PER_YEAR + 9;
    unsigned long q = yearStart(d2b(y));
    //  if I guessed wrong, adjust
    if (q > i)
    {
        --y;
        q = yearStart(d2b(y));
    }
    o.year = d2b(y);
    i -= q;

    //  take a guess at the month
    unsigned char m = i / (29 * SECONDS_PER_DAY) + 1;
    q = monthStart(o.year, d2b(m));
    //  If I guessed wrong, adjust
    if (q > i)
    {
        --m;
        q = monthStart(o.year, d2b(m));
    }
    o.month = d2b(m);
    i -= q;

    //  calculate the day
    unsigned char d = i / SECONDS_PER_DAY;
    o.mday = d2b(d + 1);
    i -= d * SECONDS_PER_DAY;
    
    //  calculate the hour
    unsigned char h = i / SECONDS_PER_HOUR;
    o.hour = d2b(h);
    i -= h * SECONDS_PER_HOUR;
    
    //  calculate the minute
    m = i / SECONDS_PER_MINUTE;
    o.minute = d2b(m);
    i -= m * SECONDS_PER_MINUTE;

    //  what's left is seconds
    o.second = d2b(i);
}

void fmtTime(char *dst, unsigned char len, char const *fmt, DateTime const &dt)
{
    char const *sd = dst;
    char const *sf = fmt;
    while (len > 1)
    {
        char ch = *fmt;
        if (ch == 0)
        {
            break;
        }
        ++fmt;
        if (ch != '%')
        {
            *dst = ch;
            ++dst;
            --len;
            continue;
        }
        ch = *fmt;
        ++fmt;
        //  This means you can't use quoted percent without extra space at the end, 
        //  but it saves a lot of code space!
        unsigned char const *val = 0;
        char const *pf = 0;
        switch (ch)
        {
        case 'y':
            val = &dt.year;
            break;
        case 'Y':
            pf = "20";
            val = &dt.year;
            break;
        case 'm':
            val = &dt.month;
            break;
        case 'd':
            val = &dt.mday;
            break;
        case 'H':
            val = &dt.hour;
            break;
        case 'M':
            val = &dt.minute;
            break;
        case 'S':
            val = &dt.second;
            break;
        case 'a':
            pf = weekdayStr(dt.wday);
            break;
        case 'b':
            pf = monthStr(dt.month);
            break;
        default:
            //  even if fmt == dateScratch, this is safe, 
            //  because I'm already 2 chars into fmt
            dateScratch[0] = ch;
            dateScratch[1] = 0;
            pf = dateScratch;
            break;
        }
        unsigned char ulen = 0;
        if (pf)
        {
            ulen += strlen(pf);
        }
        if (val)
        {
            ulen += 2;
        }
        if (len <= ulen)
        {
            break;
        }
        if (pf)
        {
            while (*pf)
            {
                *dst = *pf;
                ++dst;
                ++pf;
            }
        }
        if (val)
        {
            bcdDec(dst, 0, *val);
            dst += 2;
        }
        len -= ulen;
    }
    *dst = 0;
}

void fmtTime_P(char *dst, unsigned char len, PROGMEM prog_char *fmt, DateTime const &dt)
{
    strncpy_P(dateScratch, fmt, sizeof(dateScratch)-1);
    dateScratch[sizeof(dateScratch)-1] = 0;
    fmtTime(dst, len, dateScratch, dt);
}


unsigned long yearStart(unsigned char bcdYear)
{
    //  start in 2009, for easy leap years -- note, year AFTER leap year starts later
    unsigned long y = b2d(bcdYear) - 9;
    return y * SECONDS_PER_YEAR + (y / 4) * SECONDS_PER_DAY;
}

unsigned long monthStart(unsigned char bcdYear, unsigned char bcdMonth)
{
    unsigned long l = 0;
    unsigned char dYear = b2d(bcdYear) - 9;
    unsigned char dMonth = b2d(bcdMonth) - 1;
    if (dMonth > 11)
    {
        dMonth = 11;
    }
    for (unsigned char i = 0; i < dMonth; ++i)
    {
        l += pgm_read_byte_near(mTable + i) * SECONDS_PER_DAY;
    }
    if (dMonth > 1 && (dYear & 3) == 3)
    {
        l += SECONDS_PER_DAY;
    }
    return l;
}

unsigned long dayStart(unsigned char bcdDay)
{
    return (b2d(bcdDay) - 1) * SECONDS_PER_DAY;
}

unsigned long hourStart(unsigned char bcdHour)
{
    return b2d(bcdHour) * SECONDS_PER_HOUR;
}

unsigned long minuteStart(unsigned char bcdMinute)
{
    return b2d(bcdMinute) * SECONDS_PER_MINUTE;
}

static char const *dName[] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

char const *weekdayStr(unsigned char bcdWday)
{
    unsigned char ix = b2d(bcdWday) - 1;
    if (ix > 6) {
        ix = 6;
    }
    return dName[ix];
}

static char const *mName[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

char const *monthStr(unsigned char bcdMonth)
{
    unsigned char ix = b2d(bcdMonth) - 1;
    if (ix > 11)
    {
        ix = 11;
    }
    return mName[ix];
}

void fmtHrsMins(unsigned long secs, char *buf)
{
    secs += 59;
    unsigned long hours = secs / 3600;
    secs -= hours * 3600;
    if (hours > 99)
    {
        hours = 99;
    }
    unsigned char mins = secs / 60;
    
    unsigned char c = hours / 10;
    buf[0] = '0' + c;
    hours -= c * 10;
    buf[1] = '0' + (unsigned char)hours;
    buf[2] = ':';
    
    c = mins / 10;
    buf[3] = '0' + c;
    mins -= c * 10;
    buf[4] = '0' + mins;
}
