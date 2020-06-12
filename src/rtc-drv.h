/*
 * rtc-drv.h
 *
 * Copyright (c) 2017, 2018, 2020 Lix N. Paulian (lix@paulian.net)
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

#include <cmsis-plus/rtos/os.h>

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
  static constexpr int alarm_ignored = -1;

  void
  get_version (uint8_t& version_major, uint8_t& version_minor,
               uint8_t& version_patch);

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
  get_alarm (int which, struct tm* when);

  rtc_result_t
  reset_alarm (int which);

  rtc_result_t
  set_wakeup (uint16_t seconds);

  uint32_t
  get_bk_register (uint8_t reg_nr);

  void
  set_bk_register (uint8_t reg_nr, uint32_t value);

private:
  static constexpr uint32_t RTC_ASYNC_PREDIV = 0x1F;
  static constexpr uint32_t RTC_SYNC_PREDIV = 0x3FF;

  static constexpr uint8_t VERSION_MAJOR = 1;
  static constexpr uint8_t VERSION_MINOR = 2;
  static constexpr uint8_t VERSION_PATCH = 2;

  // The mutex timeout is set to 100 ms
  static constexpr uint32_t RTC_TIMEOUT = 100 * 1000
      / os::rtos::sysclock.frequency_hz;

  RTC_HandleTypeDef* hrtc_;
  os::rtos::mutex mutex_
    { "rtc" };

};

/**
 * @brief  Return the version of the driver.
 * @param  version_major: major version.
 * @param  version_minor: minor version.
 */
inline void
rtc::get_version (uint8_t& version_major, uint8_t& version_minor,
                  uint8_t& version_patch)
{
  version_major = VERSION_MAJOR;
  version_minor = VERSION_MINOR;
  version_patch = VERSION_PATCH;
}

/**
 * @brief  Switch an alarm off.
 * @param  which: which alarm, rtc::alarm_a or rtc::alarm_b.
 * @return
 */
inline rtc::rtc_result_t
rtc::reset_alarm (int which)
{
  return (rtc_result_t) HAL_RTC_DeactivateAlarm (hrtc_, which);
}

#endif // (__cplusplus)

#endif /* INCLUDE_RTC_DRV_H_ */
