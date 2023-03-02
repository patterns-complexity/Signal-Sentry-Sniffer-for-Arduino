# Signal Sentry - An Arduino Framework Signal Recorder and Repeater

[![License: AGPL v3](https://img.shields.io/badge/License-AGPL_v3-blue.svg)](https://www.gnu.org/licenses/agpl-3.0)

Welcome to Signal Sentry!

This is an embedded C++ project based on the Arduino framework that allows you to monitor, analyze, and repeat digital signals.

```C++
                  Signal Sentry
              __     __     __     __
      _______|  |___|  |___|  |___|  |_______

                      _______
                  +--|       |--+
      Pin 1   --+--+  o     o  +--+-   Pin 2
                | |  |       |  | |
      Pin 3   --+--+  o     o  +--+-   Pin 4
                | |  |       |  | |
      Pin 5   --+--+  o     o  +--+-   Pin 6
                  +--|_______|--+

              __     __     __     __
      _______|  |___|  |___|  |___|  |_______

An Arduino framework signal recorder and repeater
```
## Features

- Listen to digital signals and save the latest one in EEPROM
- Replay the saved signal at any point in time in the future
- Uses one pin for listening to a signal, one pin for signal output at replay time, and one pin per each of the two buttons that control the state of the program
- Simple configuration using the settings.h.dist file
- Should be compatible with most Arduino boards


## Installation and Usage
- Clone this repository to a directory of your choosing.
- Configure the `include/settings.h.dist` file and move it to `include/settings.h`.
- If you are using PlatformIO, configure the `platformio.ini` file.
- Compile the source for your microcontroller and upload it.
- Connect the pins for signal input, signal output, and the control buttons as specified in the settings.h file.
- Use the control buttons to switch between IDLE, RECORDING, SAVING, and REPLAYING modes.

## What is Signal Sentry?
After entering the `RECORDING` state, Signal Sentry starts listening for signals on the `PIN_SIGNAL_INPUT` pin. The `SAVING` state is used to store the signal in EEPROM. When the device enters the `REPLAY` state, the signal is played back on the output pin.

## Why Use Signal Sentry?
- Red teamers and pentesters can use Signal Sentry to analyze and debug digital signals in IoT devices.
- It provides a cost-effective and easy-to-use alternative to HackRF One, Flipper Zero, and similar devices.
- Signal Sentry can be used as a software oscilloscope to debug digital signals.

## Contribution
If you find a bug or have an idea for a new feature, please open an issue or submit a pull request. Your contribution is greatly appreciated!

## License
This project is licensed under the terms of the GNU Affero General Public License v3.0 (AGPL-3.0). See LICENSE for more information.

## Contact
For questions, comments, or suggestions, please contact patterns-complexity at [@patterns-complexity](github.com/patterns-complexity).
