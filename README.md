# stm32f7-rtc
This is a RTC driver for the STM32F7xx family of controllers.

## Version
* 1.2.0 (19 January 2020)

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

The solution is to enable the RTC in CubeMX, but don't generate the function call at start-up: check the `MX_RTC_INIT` checkbox in Project -> Settings...-> Advanced Settings. The corect initialization is done in the rtc-driver (`power ()`), that you may call in your program at start-up; everything else is done by the CubeMX.

You may also need to implement the alarm and wake-up call-back functions. These calls are defined as weak in HAL and can be overriden by your own implementations.

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


