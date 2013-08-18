// Code by JeeLabs http://news.jeelabs.org/code/
// Released to the public domain! Enjoy!

#include <Wire.h>
#include <avr/pgmspace.h>
#include "RTClib.h"

#define DS1307_ADDRESS 0x68
#define SECONDS_PER_DAY 86400L

#define SECONDS_FROM_1970_TO_2000 946684800

#if (ARDUINO >= 100)
 #include <Arduino.h> // capital A so it is error prone on case-sensitive filesystems
#else
 #include <WProgram.h>
#endif

int i = 0; //The new wire library needs to take an int when you are sending for the zero register
////////////////////////////////////////////////////////////////////////////////
// utility code, some of this could be exposed in the DateTime API if needed

const uint8_t daysInMonth [] PROGMEM = { 31,28,31,30,31,30,31,31,30,31,30,31 }; //has to be const or compiler compaints

// number of days since 2000/01/01, valid for 2001..2099
static uint16_t date2days(uint16_t y, uint8_t m, uint8_t d) {
    if (y >= 2000)
        y -= 2000;
    uint16_t days = d;
    for (uint8_t i = 1; i < m; ++i)
        days += pgm_read_byte(daysInMonth + i - 1);
    if (m > 2 && y % 4 == 0)
        ++days;
    return days + 365 * y + (y + 3) / 4 - 1;
}

static long time2long(uint16_t days, uint8_t h, uint8_t m, uint8_t s) {
    return ((days * 24L + h) * 60 + m) * 60 + s;
}

////////////////////////////////////////////////////////////////////////////////
// DateTime implementation - ignores time zones and DST changes
// NOTE: also ignores leap seconds, see http://en.wikipedia.org/wiki/Leap_second

DateTime::DateTime (uint32_t t) {
  t -= SECONDS_FROM_1970_TO_2000;    // bring to 2000 timestamp from 1970

  *this=DateTime::fromTime2000(t);
}

DateTime DateTime::fromTime2000(uint32_t t)
{
    uint8_t yOff, m, d, hh, mm, ss;
    
    ss = t % 60;
    t /= 60;
    mm = t % 60;
    t /= 60;
    hh = t % 24;
    uint16_t days = t / 24;
    uint8_t leap;
    for (yOff = 0; ; ++yOff) {
        leap = yOff % 4 == 0;
        if (days < 365 + leap)
            break;
        days -= 365 + leap;
    }
    for (m = 1; ; ++m) {
        uint8_t daysPerMonth = pgm_read_byte(daysInMonth + m - 1);
        if (leap && m == 2)
            ++daysPerMonth;
        if (days < daysPerMonth)
            break;
        days -= daysPerMonth;
    }
    d = days + 1;
    
    return DateTime(2000+yOff,m,d,hh,mm,ss);
  
}

DateTime::DateTime (uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec) {
    if (year >= 2000)
        year -= 2000;
    yOff = year;
    m = month;
    d = day;
    hh = hour;
    mm = min;
    ss = sec;
}

static uint8_t conv2d(const char* p) {
    uint8_t v = 0;
    if ('0' <= *p && *p <= '9')
        v = *p - '0';
    return 10 * v + *++p - '0';
}

// A convenient constructor for using "the compiler's time":
//   DateTime now (__DATE__, __TIME__);
// NOTE: using PSTR would further reduce the RAM footprint
DateTime::DateTime (const char* date, const char* time) {
    // sample input: date = "Dec 26 2009", time = "12:34:56"
    yOff = conv2d(date + 9);
    // Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec 
    switch (date[0]) {
        case 'J': m = date[1] == 'a' ? 1 : m = date[2] == 'n' ? 6 : 7; break;
        case 'F': m = 2; break;
        case 'A': m = date[2] == 'r' ? 4 : 8; break;
        case 'M': m = date[2] == 'r' ? 3 : 5; break;
        case 'S': m = 9; break;
        case 'O': m = 10; break;
        case 'N': m = 11; break;
        case 'D': m = 12; break;
    }
    d = conv2d(date + 4);
    hh = conv2d(time);
    mm = conv2d(time + 3);
    ss = conv2d(time + 6);
}

uint8_t DateTime::dayOfWeek() const {    
    uint16_t day = date2days(yOff, m, d);
    return (day + 6) % 7; // Jan 1, 2000 is a Saturday, i.e. returns 6
}

uint32_t DateTime::unixtime(void) const {
  uint32_t t;
  uint16_t days = date2days(yOff, m, d);
  t = time2long(days, hh, mm, ss);
  t += SECONDS_FROM_1970_TO_2000;  // seconds from 1970 to 2000

  return t;
}

////////////////////////////////////////////////////////////////////////////////
// RTC_DS1307 implementation

static uint8_t bcd2bin (uint8_t val) { return val - 6 * (val >> 4); }
static uint8_t bin2bcd (uint8_t val) { return val + 6 * (val / 10); }

#ifdef RTC_UTC
    int8_t RTC_DS1307::UTC=1;
#endif

#ifdef RTC_EUROPE
bool RTC_DS1307::isMEZSummerTime(const DateTime &date)
{
  uint32_t current_time, summer_start, winter_start;
  current_time = date.unixtime();
  
  DateTime summer=DateTime(date.year(),3,31,2,0,0);
  DateTime winter=DateTime(date.year(),10,31,3,0,0);
  
  summer_start = summer.unixtime()-summer.dayOfWeek()*24*3600;
  winter_start = winter.unixtime()-winter.dayOfWeek()*24*3600;
   
  // return result
  if ( summer_start <= current_time && current_time < winter_start )
    return true;
  else
    return false;  
}
#endif
    
    
uint8_t RTC_DS1307::begin(void) {
  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write((uint8_t)0x00);
  if (Wire.endTransmission() == 0) 
    return 1;
  else
    return 0;
}

void RTC_DS1307::stopClock(void)         // set the ClockHalt bit high to stop the rtc
{
  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write((uint8_t)0x00);                      // Register 0x00 holds the oscillator start/stop bit
  Wire.endTransmission();
  Wire.requestFrom(DS1307_ADDRESS, 1);
  uint8_t second = Wire.read() | 0x80;       // save actual seconds and OR sec with bit 7 (sart/stop bit) = clock stopped
  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write((uint8_t)0x00);
  Wire.write((uint8_t)second);                    // write seconds back and stop the clock
  Wire.endTransmission();
}

void RTC_DS1307::startClock(void)        // set the ClockHalt bit low to start the rtc
{
  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write((uint8_t)0x00);                      // Register 0x00 holds the oscillator start/stop bit
  Wire.endTransmission();
  Wire.requestFrom(DS1307_ADDRESS, 1);
  uint8_t second = Wire.read() & 0x7f;       // save actual seconds and AND sec with bit 7 (sart/stop bit) = clock started
  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write((uint8_t)0x00);
  Wire.write((uint8_t)second);                    // write seconds back and start the clock
  Wire.endTransmission();
}



#if (ARDUINO >= 100)

uint8_t RTC_DS1307::isrunning(void) {
  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(i);	
  Wire.endTransmission();

  Wire.requestFrom(DS1307_ADDRESS, 1);
  uint8_t ss = Wire.read();
  return !(ss>>7);
}

void RTC_DS1307::adjust(const DateTime& dt) {
    Wire.beginTransmission(DS1307_ADDRESS);
    Wire.write(i);
    Wire.write(bin2bcd(dt.second()));
    Wire.write(bin2bcd(dt.minute()));
    Wire.write(bin2bcd(dt.hour()));
    Wire.write(bin2bcd(0));
    Wire.write(bin2bcd(dt.day()));
    Wire.write(bin2bcd(dt.month()));
    Wire.write(bin2bcd(dt.year() - 2000));
    Wire.write(i);
    Wire.endTransmission();
}

DateTime RTC_DS1307::now() {
  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(i);	
  Wire.endTransmission();
  
  Wire.requestFrom(DS1307_ADDRESS, 7);
  uint8_t ss = bcd2bin(Wire.read() & 0x7F);
  uint8_t mm = bcd2bin(Wire.read());
  uint8_t hh = bcd2bin(Wire.read());
  Wire.read();
  uint8_t d = bcd2bin(Wire.read());
  uint8_t m = bcd2bin(Wire.read());
  uint16_t y = bcd2bin(Wire.read()) + 2000;
  
  #ifdef RTC_UTC || RTC_EUROPE    
    uint16_t days2000=date2days(y, m, d);
    long time2000=time2long(days2000, hh, mm, ss);
    time2000+=UTC*3600; 
    
    // Construct DateTime with UTC
    DateTime temp=DateTime::fromTime2000(time2000);

    #ifdef RTC_EUROPE
    if(isMEZSummerTime(temp))
    {
      time2000+=3600;
      temp=DateTime::fromTime2000(time2000);
    }
    #endif
    
    return temp;  
  #else
  return DateTime (y, m, d, hh, mm, ss);
  #endif
}

#else

uint8_t RTC_DS1307::isrunning(void) {
  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.send(i);	
  Wire.endTransmission();

  Wire.requestFrom(DS1307_ADDRESS, 1);
  uint8_t ss = Wire.receive();
  return !(ss>>7);
}

void RTC_DS1307::adjust(const DateTime& dt) {
    Wire.beginTransmission(DS1307_ADDRESS);
    Wire.send(i);
    Wire.send(bin2bcd(dt.second()));
    Wire.send(bin2bcd(dt.minute()));
    Wire.send(bin2bcd(dt.hour()));
    Wire.send(bin2bcd(0));
    Wire.send(bin2bcd(dt.day()));
    Wire.send(bin2bcd(dt.month()));
    Wire.send(bin2bcd(dt.year() - 2000));
    Wire.send(i);
    Wire.endTransmission();
}

DateTime RTC_DS1307::now() {
  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.send(i);	
  Wire.endTransmission();
  
  Wire.requestFrom(DS1307_ADDRESS, 7);
  uint8_t ss = bcd2bin(Wire.receive() & 0x7F);
  uint8_t mm = bcd2bin(Wire.receive());
  uint8_t hh = bcd2bin(Wire.receive());
  Wire.receive();
  uint8_t d = bcd2bin(Wire.receive());
  uint8_t m = bcd2bin(Wire.receive());
  uint16_t y = bcd2bin(Wire.receive()) + 2000;
  
  return DateTime (y, m, d, hh, mm, ss);
}

#endif


////////////////////////////////////////////////////////////////////////////////
// RTC_Millis implementation

long RTC_Millis::offset = 0;

void RTC_Millis::adjust(const DateTime& dt) {
    offset = dt.unixtime() - millis() / 1000;
}

DateTime RTC_Millis::now() {
  return (uint32_t)(offset + millis() / 1000);
}

////////////////////////////////////////////////////////////////////////////////
