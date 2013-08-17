// Code by JeeLabs http://news.jeelabs.org/code/
// Released to the public domain! Enjoy!
// Simple general-purpose date/time class (no TZ / DST / leap second handling!)
// AT: Inclusion of conditional compilation to handle EU Summer Time and UTC from :
// # Scriptname : DS1307new.h
// # Author     : Peter Schmelzer
// # Contributor: Oliver Kraus
// # contact    : schmelle2@googlemail.com
// # Date       : 2010-11-01
// # Version    : 0.2
// # License    : cc-by-sa-3.0
//
// Into Adafruit library (which is based on JeeLabs code)
//
// AT: I am just merging code from different sources, merit (for good) goes to the people above, you can blame me for my bugs ;-)
//
// AT: not an expert in the topic but this code is under cc-by-sa-3.0 unless other legal terms apply


#ifndef RTCLIB_H
#define RTCLIB_H

#define RTC_EUROPE
#define RTC_UTC

class DateTime {
public:
    DateTime (uint32_t t =0);
    DateTime (uint16_t year, uint8_t month, uint8_t day,
                uint8_t hour =0, uint8_t min =0, uint8_t sec =0);
    DateTime (const char* date, const char* time);
    uint16_t year() const       { return 2000 + yOff; }
    uint8_t month() const       { return m; }
    uint8_t day() const         { return d; }
    uint8_t hour() const        { return hh; }
    uint8_t minute() const      { return mm; }
    uint8_t second() const      { return ss; }
    uint8_t dayOfWeek() const;

    // 32-bit times as seconds since 1/1/2000
    long secondstime() const;   
    // 32-bit times as seconds since 1/1/1970
    uint32_t unixtime(void) const;

protected:
    uint8_t yOff, m, d, hh, mm, ss;
};

// RTC based on the DS1307 chip connected via I2C and the Wire library
class RTC_DS1307 {
public:
  static uint8_t begin(void);
  static void stopClock(void);
  static void startClock(void);
    static void adjust(const DateTime& dt);
    uint8_t isrunning(void);
    static DateTime now();

#ifdef RTC_UTC
    static int8_t UTC;
#endif
    
#ifdef RTC_EUROPE
    static bool isMEZSummerTime(const DateTime &date);
#endif
};

// RTC using the internal millis() clock, has to be initialized before use
// NOTE: this clock won't be correct once the millis() timer rolls over (>49d?)
class RTC_Millis {
public:
    static void begin(const DateTime& dt) { adjust(dt); }
    static void adjust(const DateTime& dt);
    static DateTime now();

protected:
    static long offset;
};
#endif
