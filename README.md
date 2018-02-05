# stm32f7-rtc
This is a RTC driver for the STM32F7xx family of controllers.

## Version
* 1.0 (5 February 2018)

## License
* MIT

## Package
The driver is provided as an _xpack_ and can be installed in an Eclipse based project using the attached script (however, the include and source paths must be manually added to the project in Eclipse). For more details on _xpacks_ see https://github.com/xpacks. The installation script requires the helper scripts that can be found at https://github.com/xpacks/scripts.

## Dependencies
The driver depends on the following software packages, all available as _xpacks_:
* STM32F7 CMSIS (https://github.com/xpacks/stm32f7-cmsis)
* STM32F7xx HAL Library (https://github.com/xpacks/stm32f7-hal)
* uOS++ (https://github.com/micro-os-plus/micro-os-plus-iii)

Note that the hardware initialisations (uController clock, peripherals clocks, etc.) must be separately performed, normaly in, or called from the initialize_hardware.c file of a gnuarmeclipse project. You may also do this using the CubeMX generator from ST. You may find helpful to check the following projects as references:
* https://github.com/micro-os-plus/eclipse-demo-projects/tree/master/f746gdiscovery-blinky-micro-os-plus
* https://github.com/micro-os-plus/eclipse-demo-projects/tree/master/f746gdiscovery-blinky-micro-os-plus/cube-mx which details how to integrate the CubeMX generated code into a uOS++ based project.

There are however several issues if using the cubeMX generator: if the RTC OSC is enabled in cubeMX, then the oscillator initialization is made in the `SystemClock_Config ()` function (normally in cubeMX's "main.c" file"). This means that each time the system is started, the 32 Khz oscillator will be also restarted, which introduces a delay of several hundred of milliseconds whereas the RTC will not count. Thus, after a couple of resets, the RTC may already have lost several seconds.

Therefore don't enable the RTC in cubeMX. The correct initialization is done in the rtc-driver; you must however define elsewhere the RTC alarm interrupt handler. If you use cubeMX, then the right place is in the stm32f7xx_it.c file in a "user code" section, as shown below:

```c
/* USER CODE BEGIN 1 */

/**
* @brief This function handles RTC alarms (A and B) interrupt through EXTI line 17.
*/
void RTC_Alarm_IRQHandler (void)
{
    HAL_RTC_AlarmIRQHandler (&hrtc);
}

/* USER CODE END 1 */
```

You must also define the rtc handle, and this can be done in the same file:

```c
/* USER CODE BEGIN 0 */

RTC_HandleTypeDef hrtc;

/* USER CODE END 0 */
```

Finally, you must add the line

```c
/* USER CODE BEGIN Private defines */

#define HAL_RTC_MODULE_ENABLED

/* USER CODE END Private defines */
```

in main.h.

The driver was designed for the µOS++ ecosystem, but it can be easily ported to other RTOSes, as it uses only a mutex.

## Time Zone
The driver assumes that all date/time information is UTC (the time_t datatype refers to UTC). This might be a problem when setting the alarms, as they __must__ be referenced in UTC too. For recurring alarms set at intervals defined only in seconds and minutes this is not an issue. However, if hours, days, months are used to define alarms, then the data in the tm structure must be first converted to UTC.

The time zone in your application should be set using the `setenv ()` and `tzset ()` functions. Then all standard time functions would properly operate (`localtime ()`, `gmtime ()`, `mktime ()`, etc.). The integration with your system should be made using the `gettimeofday ()` and `settimeofday ()` syscall functions. For uOS++ this will follow soon.

## Tests
A test is also provided as example. The test was compiled and run on the STM32F746G-Disco board, as well as on a proprietary board. The test is doing the following:
* Initializes the RTC
* Optionally sets the date and time (especially useful if the hardware includes a battery backup)
* Sets two alarms: one every second, the other every minute at second 0
* Optionally de-initializes the RTC


