RTClib
======

A fork of Jeelab's fantastic RTC library / Adafruit / DS1307new by Peter Schmelzer by Oliver Kraus

I have some programs running with Adafruit RTC library, but I needed UTC time zone and Summer time (+1 hour) detection.

I found DS1307new library having Summer Time for Europe, but involved too much code modifications in several projects.

I just merged this funtionality into a fork of Adafruit's library.

This library is conceived to work with a clock initialized to UTC+0, as obtained for example from NTP.

The header file includes two #define. With both #defines undefined it should be almost the same as Adafruit's library.

If RTC_UTC is defined, an static member indicates your UTC zone, and this gets added to the clock in now();
If RTC_EUROPE is defined, the Summer time detection is executed everytime the now() function is triggered and +1 added if
in summer time.

Both cpp and h contain conditional compilation. Arduino IDE can not (directly/easily/at all) handle setting this in CFLAGS
so I am stuck with having to set those two in the library header, instead of doing it in my project, or only the conditional
compilation is executed in the header file, not in the cpp implementation (or I could change to a single h file containing
also the implementation, which I do not like for c++).

This is quite untested, so may be buggy.
