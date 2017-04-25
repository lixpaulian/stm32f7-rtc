/*
 * rtc-drv.cpp
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
 * Created on: 15 Apr 2017 (LNP)
 */

/*
 * This file implements the low level functions to control the RTC.
 */

#include <cmsis-plus/rtos/os.h>
#include <cmsis-plus/diag/trace.h>
#include "rtc-drv.h"

using namespace os;

/**
 * @brief Constructor.
 * @param hrtc: HAL hrtc handle.
 */
rtc::rtc (RTC_HandleTypeDef* hrtc)
{
  trace::printf ("%s(%p) @%p\n", __func__, hrtc, this);
  hrtc_ = hrtc;
}

/**
 * @brief  Control the power state of the RTC peripheral.
 * @param  state: new state, either true (power on) or false (power off).
 * @return rtc::ok if successful, or a RTC error.
 */
rtc::rtc_result_t
rtc::power (bool state)
{
  rtc_result_t result;

  if (state == true)
    {
      hrtc_->Instance = RTC;

      hrtc_->Init.HourFormat = RTC_HOURFORMAT_24;
      hrtc_->Init.AsynchPrediv = RTC_ASYNC_PREDIV;
      hrtc_->Init.SynchPrediv = RTC_SYNC_PREDIV;
      hrtc_->Init.OutPut = RTC_OUTPUT_DISABLE;
      hrtc_->Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
      hrtc_->Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
      result = (rtc_result_t) HAL_RTC_Init (hrtc_);
    }
  else
    {
      result = (rtc_result_t) HAL_RTC_DeInit (hrtc_);
    }
  return result;
}

/**
 * @brief  Set the RTC from a Unix time value.
 * @param  u_time: pointer on a time_t Unix time value.
 * @return rtc::ok if successful, or a RTC error.
 */
rtc::rtc_result_t
rtc::set_time (time_t* u_time)
{
  RTC_TimeTypeDef RTC_TimeStructure;
  RTC_DateTypeDef RTC_DateStructure;

  rtc::rtc_result_t result = busy;
  struct tm *timeptr = localtime (u_time);

  RTC_TimeStructure.Seconds = timeptr->tm_sec;
  RTC_TimeStructure.Minutes = timeptr->tm_min;
  RTC_TimeStructure.Hours = timeptr->tm_hour;
  RTC_TimeStructure.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  RTC_TimeStructure.StoreOperation = RTC_STOREOPERATION_SET;
  RTC_TimeStructure.SubSeconds = 0;

  RTC_DateStructure.Date = timeptr->tm_mday;
  RTC_DateStructure.Month = timeptr->tm_mon + 1;
  RTC_DateStructure.Year = timeptr->tm_year - 100;
  RTC_DateStructure.WeekDay = timeptr->tm_wday + 1;

  if (mutex_.timed_lock (RTC_TIMEOUT) == rtos::result::ok)
    {
      result = (rtc_result_t) HAL_RTC_SetTime (hrtc_, &RTC_TimeStructure,
      FORMAT_BIN);
      if (result == ok)
        {
          result = (rtc_result_t) HAL_RTC_SetDate (hrtc_, &RTC_DateStructure,
          FORMAT_BIN);
        }
      mutex_.unlock ();
    }
  return result;
}

/**
 * @brief  Return the current RTC value as Unix time.
 * @param  u_time: pointer on a time_t.
 * @return rtc::ok if successful, or a RTC error.
 */
rtc::rtc_result_t
rtc::get_time (time_t* u_time)
{
  RTC_TimeTypeDef RTC_TimeStructure;
  RTC_DateTypeDef RTC_DateStructure;
  struct tm timestruct;

  rtc::rtc_result_t result = busy;

  memset (&timestruct, 0, sizeof(struct tm));

  if (mutex_.timed_lock (RTC_TIMEOUT) == rtos::result::ok)
    {
      result = (rtc_result_t) HAL_RTC_GetTime (hrtc_, &RTC_TimeStructure,
      FORMAT_BIN);
      if (result == ok)
        {
          result = (rtc_result_t) HAL_RTC_GetDate (hrtc_, &RTC_DateStructure,
          FORMAT_BIN);
          if (result == ok)
            {
              timestruct.tm_sec = RTC_TimeStructure.Seconds;
              timestruct.tm_min = RTC_TimeStructure.Minutes;
              timestruct.tm_hour = RTC_TimeStructure.Hours;

              timestruct.tm_mday = RTC_DateStructure.Date;
              timestruct.tm_mon = RTC_DateStructure.Month - 1;
              timestruct.tm_year = RTC_DateStructure.Year + 100;
              timestruct.tm_wday = RTC_DateStructure.WeekDay - 1;

              *u_time = mktime (&timestruct);
            }
        }
      mutex_.unlock ();
    }
  return result;
}

/**
 * @brief  Set the calibration factor.
 * @param  cal_factor: calibration factor (from -511 to +512).
 * @return rtc::ok if successful, or a RTC error.
 */
rtc::rtc_result_t
rtc::set_cal_factor (int cal_factor)
{
  uint32_t calib_minus_pulses_val;
  uint32_t calib_plus_pulses;
  rtc::rtc_result_t result = invalid_param;

  if (cal_factor >= -511 && cal_factor <= 512)
    {
      if (cal_factor > 0)
        {
          calib_minus_pulses_val = 512 - cal_factor;
          calib_plus_pulses = RTC_SMOOTHCALIB_PLUSPULSES_SET;
        }
      else
        {
          cal_factor *= -1; // transform to absolute value
          calib_minus_pulses_val = cal_factor;
          calib_plus_pulses = RTC_SMOOTHCALIB_PLUSPULSES_RESET;
        }
      result = (rtc_result_t) HAL_RTCEx_SetSmoothCalib (
          hrtc_, //
          RTC_SMOOTHCALIB_PERIOD_32SEC, calib_plus_pulses,
          calib_minus_pulses_val);
    }
  return result;
}

/**
 * @brief  Get the current calibration factor.
 * @return The current calibration factor (-511 to +512).
 */
int
rtc::get_cal_factor (void)
{
  int cal_factor;
  uint32_t rtc_calr = RTC->CALR;

  cal_factor = (rtc_calr & 0x8000) ? 512 : 0;
  cal_factor -= (rtc_calr & 0x1FF);

  return cal_factor;
}

/**
 * @brief  Set an alarm.
 * @param  which: which alarm, for the STM32F7xxx there are two alarms, one of
 *      alarm_a or alarm_b.
 * @param  when: a struct tm containing the specs for the alarm point.
 * @return rtc::ok if successful, or a RTC error.
 */
rtc::rtc_result_t
rtc::set_alarm (int which, struct tm* when)
{
  RTC_AlarmTypeDef alarm;
  rtc::rtc_result_t result;

  result = (rtc_result_t) HAL_RTC_DeactivateAlarm (hrtc_, which);
  if (result == ok)
    {
      alarm.AlarmMask = 0;

      if (when->tm_wday < 0 && when->tm_mday < 0)
        {
          // no weekday, no month-day specified, therefore mask the day
          alarm.AlarmMask |= RTC_ALARMMASK_DATEWEEKDAY;
          alarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
        }
      else
        {
          if (when->tm_mday > 0)
            {
              // day of month specified
              alarm.AlarmDateWeekDay = (uint8_t) when->tm_mday;
              alarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
            }
          else
            {
              // day of week specified
              alarm.AlarmDateWeekDay = (uint8_t) when->tm_wday;
              alarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_WEEKDAY;
            }
        }

      if (when->tm_hour < 0)
        {
          // no hours specified, so we mask the hours
          alarm.AlarmMask |= RTC_ALARMMASK_HOURS;
          alarm.AlarmTime.Hours = 0;
        }
      else
        {
          alarm.AlarmTime.Hours = (uint8_t) when->tm_hour;
        }

      if (when->tm_min < 0)
        {
          // no minutes specified, so we mask the minutes
          alarm.AlarmMask |= RTC_ALARMMASK_MINUTES;
          alarm.AlarmTime.Minutes = 0;
        }
      else
        {
          alarm.AlarmTime.Minutes = when->tm_min;
        }

      if (when->tm_sec < 0)
        {
          // no seconds specified, so we mask the seconds
          alarm.AlarmMask |= RTC_ALARMMASK_SECONDS;
          alarm.AlarmTime.Seconds = 0;
        }
      else
        {
          alarm.AlarmTime.Seconds = when->tm_sec;
        }

      // initialize the rest
      alarm.Alarm = which;
      alarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_NONE;
      alarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
      alarm.AlarmTime.TimeFormat = RTC_HOURFORMAT12_AM;
      alarm.AlarmTime.SubSeconds = 0;
      alarm.AlarmTime.SecondFraction = 0;

      result = (rtc_result_t) HAL_RTC_SetAlarm_IT (hrtc_, &alarm, FORMAT_BIN);
    }
  return result;
}
