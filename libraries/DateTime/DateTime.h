
#if !defined(DATETIME_H)
#define DATETIME_H

#include <avr/pgmspace.h>

//  Note: time in this struct is BCD!
struct DateTime
{
  unsigned char second;
  unsigned char minute;
  unsigned char hour;
  unsigned char wday;
  unsigned char mday;
  unsigned char month;
  unsigned char year;
};

void readTime(DateTime &o);
void writeTime(DateTime const &i);
bool timeIsInvalid();

unsigned long fromTime(DateTime const &i);
void toTime(unsigned long i, DateTime &o);

void bcdDec(char *dst, unsigned char o, unsigned char b);
//%Y, %m, %d, %H, %M, %S from strfmt
//%y == 2digit year
//%a == 3-letter weekday
//%b == 3-letter month
void fmtTime(char *dst, unsigned char len, char const *fmt, DateTime const &dt);
void fmtTime_P(char *dst, unsigned char len, PROGMEM prog_char *fmt, DateTime const &dt);

unsigned long yearStart(unsigned char bcdYear);
unsigned long monthStart(unsigned char bcdYear, unsigned char bcdMonth);
unsigned long dayStart(unsigned char bcdDay);
unsigned long hourStart(unsigned char bcdHour);
unsigned long minuteStart(unsigned char bcdMinute);

char const *weekdayStr(unsigned char bcdWday);
char const *monthStr(unsigned char bcdMonth);

//buf needs 5 chars; will not be zero terminated
void fmtHrsMins(unsigned long secs, char *buf);

#endif // DATETIME_H
