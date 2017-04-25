/*
 * rtc-drv.h
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

#ifndef INCLUDE_RTC_DRV_H_
#define INCLUDE_RTC_DRV_H_

#include "cmsis_device.h"
#include "rtc.h"

#if defined (__cplusplus)

class rtc
{
public:
  rtc (RTC_HandleTypeDef* hrtc);

  ~rtc () = default;

  typedef enum
  {
    ok = HAL_OK,                // HAL errors
    error = HAL_ERROR,
    busy = HAL_BUSY,
    timeout = HAL_TIMEOUT,
    invalid_param = 10,        // RTC specific
  } rtc_result_t;

  static constexpr int alarm_a = RTC_ALARM_A;
  static constexpr int alarm_b = RTC_ALARM_B;

  void
  get_version (uint8_t& version_major, uint8_t& version_minor);

  rtc_result_t
  power (bool state);

  rtc_result_t
  set_time (time_t* u_time);

  rtc_result_t
  get_time (time_t* u_time);

  rtc_result_t
  set_cal_factor (int cal_factor);

  int
  get_cal_factor (void);

  rtc_result_t
  set_alarm (int which, struct tm* when);

  rtc_result_t
  reset_alarm (int which);

private:
  static constexpr uint32_t RTC_ASYNC_PREDIV = 0x1F;
  static constexpr uint32_t RTC_SYNC_PREDIV = 0x3FF;

  static constexpr uint8_t RTC_DRV_VERSION_MAJOR = 0;
  static constexpr uint8_t RTC_DRV_VERSION_MINOR = 4;

  // Some timeouts; all timeouts are in # of uOS++ ticks (normally 1 ms)
  static constexpr uint32_t RTC_TIMEOUT = 100;

  RTC_HandleTypeDef* hrtc_;
  os::rtos::mutex mutex_
    { "rtc" };

};

inline void
rtc::get_version (uint8_t& version_major, uint8_t& version_minor)
{
  version_major = RTC_DRV_VERSION_MAJOR;
  version_minor = RTC_DRV_VERSION_MINOR;
}

inline rtc::rtc_result_t
rtc::reset_alarm (int which)
{
  return (rtc_result_t) HAL_RTC_DeactivateAlarm (hrtc_, which);
}

#endif // (__cplusplus)

#endif /* INCLUDE_RTC_DRV_H_ */