![GitHub package.json version](https://img.shields.io/github/package-json/v/lixpaulian/stm32f7-rtc)
![GitHub Tag](https://img.shields.io/github/v/tag/lixpaulian/stm32f7-rtc)
![GitHub License](https://img.shields.io/github/license/lixpaulian/stm32f7-rtc)

# stm32f7-rtc
This is an RTC driver for the STM32F7xx family of controllers.

## Package
The class is provided as an **xPack** (for more details on xPacks see https://xpack.github.io). It can be installed in a project using either `xpm` or the attached script. Of course, it can be installed without using the xPacks tools, either by linking the class as a Git submodule or by copying it in your project, but then updating it later might be more difficult.

Note that the xPacks project evolved with the time. Initially it was based on shell scripts, but the current version is based on a set of utilities, `xpm` and a JSON description file. You will still find the `xpacks-helper.sh` script in the `scripts` subdirectory, but it is not recommened as it is deprecated and will not be supported in the future. Instead use the procedure described below.

To install the package using `xpm` you must make sure that you have already `nodejs` and `xpm` installed on your computer (see also [xPack install](https://xpack.github.io/install/)). Then, in your project directory issue the commands:

```sh
cd my-project
xpm init # Add a package.json if not already present
xpm install github:lixpaulian/stm32f7-rtc#v1.2.2 --copy
```

Note: Without `--copy`, the default is to create a link to a read-only instance of the package in the `xpm` central store.

## Dependencies
The driver depends on the following software packages, all available as xPacks:
* STM32F7 CMSIS (https://github.com/xpacks/stm32f7-cmsis)
* STM32F7xx HAL Library (https://github.com/xpacks/stm32f7-hal)
* µOS++ (https://github.com/micro-os-plus/micro-os-plus-iii)

Note that the hardware initialisations (µController clock, peripherals clocks, etc.) must be separately performed, normaly in, or called from the `initialize_hardware.c` file of a GNU MCU Eclipse project. You may also do this using the CubeMX generator from ST (recommended, as the STM32xxx xPacks will probably be discontinued). You may find helpful to check the following projects as references:
* https://github.com/micro-os-plus/eclipse-demo-projects/tree/master/f746gdiscovery-blinky-micro-os-plus
* https://github.com/micro-os-plus/eclipse-demo-projects/tree/master/f746gdiscovery-blinky-micro-os-plus/cube-mx which details how to integrate the CubeMX generated code into a µOS++ based project.

There are however several issues if using the cubeMX generator: if the RTC OSC is enabled in cubeMX, then the oscillator initialization is made in the `SystemClock_Config ()` function (normally in cubeMX's "main.c" file"). This means that each time the system is started, the 32 Khz oscillator will be also restarted, which introduces a delay of several hundred of milliseconds whereas the RTC will not count. Thus, after a couple of resets, the RTC may already have lost several seconds.

The solution is to enable the RTC in CubeMX, but don't generate the function call at start-up: check the `MX_RTC_INIT` checkbox in Project -> Settings...-> Advanced Settings. The corect initialization is done in the rtc-driver (`power ()`), that you may call in your program at start-up; everything else is done by the CubeMX.

You may also need to implement the alarm and wake-up call-back functions. These calls are defined as weak in HAL and can be overriden by your own implementations.

The driver was designed for the µOS++ ecosystem, but it can be easily ported to other RTOSes, as it uses only a mutex.

## Time Zone
The driver assumes that all date/time information is UTC (the time_t datatype refers to UTC). This might be a problem when setting the alarms, as they __must__ be referenced in UTC too. For recurring alarms set at intervals defined only in seconds and minutes this is not an issue. However, if hours, days, months are used to define alarms, then the data in the tm structure must be first converted to UTC.

The time zone in your application should be set using the `setenv ()` and `tzset ()` functions. Then all standard time functions would properly operate (`localtime ()`, `gmtime ()`, `mktime ()`, etc.). The integration with your system should be made using the `gettimeofday ()` and `settimeofday ()` syscall functions. For µOS++ this will follow soon.

## Tests
A test is also provided as example. The test was compiled and run on the STM32F746G-Disco board, as well as on a proprietary board. The test is doing the following:
* Initializes the RTC
* Optionally sets the date and time (especially useful if the hardware includes a battery backup)
* Sets two alarms: one every second, the other every minute at second 0
* Optionally de-initializes the RTC


