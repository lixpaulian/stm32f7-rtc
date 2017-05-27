/*
 * test-rtc.cpp
 *
 * Copyright (c) 2017 Lix N. Paulian (lix@paulian.net)
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Created on: 16 Apr 2017 (LNP)
 */

#include <stdio.h>
#include <stdint.h>
#include <cmsis-plus/rtos/os.h>
#include <cmsis-plus/diag/trace.h>

#include "rtc-drv.h"

#if defined M717
#include "io.h"
#endif

//#define SET_CLOCK

extern "C"
{
  RTC_HandleTypeDef hrtc;
}

using namespace os;

rtc my_rtc
  { &hrtc };

#if defined M717
led red
  { LED_RED };
led blue
  { LED_BLUE };
#endif

void
HAL_RTC_AlarmAEventCallback (RTC_HandleTypeDef *hrtc __attribute__ ((unused)))
{
  red.toggle ();
}

void
HAL_RTCEx_AlarmBEventCallback (RTC_HandleTypeDef *hrtc __attribute__ ((unused)))
{
  blue.toggle ();
}

/**
 * @brief  This is a test function that exercises the RTC driver.
 */
void
test_rtc (void)
{
  rtc::rtc_result_t result;
  uint8_t version_major, version_minor;
  time_t clock;
  struct tm tm_time;

#if defined M717
  red.power_up ();
  blue.power_up ();
#endif

  my_rtc.get_version (version_major, version_minor);
  trace::printf ("RTC driver version: %d.%d\n", version_major, version_minor);

  if ((result = my_rtc.power (true)) == rtc::ok)
    trace::printf ("RTC powered up\n");
  else
    trace::printf ("RTC power-up failed (%d)\n", result);

#ifdef SET_CLOCK
  tm_time.tm_year = 117;
  tm_time.tm_mon = 3;
  tm_time.tm_mday = 17;
  tm_time.tm_hour = 12;
  tm_time.tm_min = 42;
  tm_time.tm_sec = 0;
  tm_time.tm_isdst = true;
  clock = mktime (&tm_time);

  if ((result = my_rtc.set_time (&clock)) == rtc::ok)
  trace::printf ("RTC date/time set\n");
  else
  trace::printf ("Failed to set the RTC date/time\n");
#endif

  if ((result = my_rtc.get_time (&clock)) == rtc::ok)
    {
      trace::printf ("%s\n", ctime (&clock));
    }
  else
    trace::printf ("Failed to get the RTC date/time\n");

  // Set alarm A
  tm_time.tm_hour = rtc::alarm_ignored;
  tm_time.tm_mday = rtc::alarm_ignored;
  tm_time.tm_min = rtc::alarm_ignored;
  tm_time.tm_wday = rtc::alarm_ignored;
  tm_time.tm_sec = rtc::alarm_ignored;  // alarm A every second

  my_rtc.set_alarm (rtc::alarm_a, &tm_time);

  tm_time.tm_sec = 0;   // alarm B every minute at second 0
  my_rtc.set_alarm (rtc::alarm_b, &tm_time);

#if 0
  // This section is only for testing, but if you enable it be aware that
  // it will erase the RTC values. The date/time will be also lost.
  if ((result = my_rtc.power (false)) == rtc::ok)
  trace::printf ("RTC powered down\n");
  else
  trace::printf ("RTC power-down failed (%d)\n", result);
#endif
}
