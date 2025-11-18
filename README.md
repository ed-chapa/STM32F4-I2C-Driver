# STM32F410RB Bare-Metal Weather Display

This project demonstrates a bare-metal implementation of a temperature and humidity display system using an STM32F410RB microcontroller, a DHT11 sensor, and an SSD1306 OLED display. It is built entirely with CMSIS, without relying on STM32 HAL or LL libraries.

## Overview

- **MCU**: STM32F410RB (ARM Cortex-M4)
- **Sensor**: DHT11
- **Display**: SSD1306 OLED
- **Framework**: CMSIS (no HAL/LL)
- **Language**: C

## Features

- Bare-metal firmware written using CMSIS headers and direct register access
- Periodic data acquisition from DHT11 every 1.25 seconds using TIM6
- TIM1 used for input capture with DMA2_Stream1 to timestamp DHT11 pulse widths
- Decoding of DHT11 data from captured timestamps
- Encoding and rendering of temperature and humidity data for SSD1306 OLED display
- I2C communication with SSD1306 using custom driver

## Architecture

### 1. **Data Acquisition**
- TIM6 is configured to trigger an interrupt every 1.25 seconds
- This interrupt generates an 18ms pulse on PA8 that initiates the DHT11 communication sequence

### 2. **Pulse Capture**
- TIM1 is enabled in input capture mode and PA8 is reconfigured to alternate funtion to record falling edges of DHT11 pulses
- DMA2_Stream1 transfers the captured timestamps to a memory buffer

### 3. **Data Decoding**
- Pulse widths are compared against a threshold to extract the 40-bit DHT11 data stream
- Data is parsed into a formatted string containing temperature and humidity values

### 4. **Display Output**
- The display is sent a sequence of commands for initialization
- Values are encoded into ASCII format and sent to SSD1306 OLED via I2C1 and pins PB8 and PB9 using a custom driver

## Tools Used

- **Compiler**: arm-none-eabi-gcc
- **Debugger**: GDB with ST-Link
- **Flashing**: st-flash
- **Editor**: VS Code / STM32CubeIDE (for reference only)

## Getting Started

1. Clone the repository
2. Connect STM32F410RB, DHT11, and SSD1306 OLED to appropriate GPIO pins
3. Build the project using `make`
4. Flash the firmware using st-flash
5. View temperature and humidity on the OLED display

## References

- [DHT11 Datasheet](https://www.sparkfun.com/datasheets/Sensors/Temperature/DHT11.pdf)
- [SSD1306 Datasheet](https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf)
- [CMSIS Documentation](https://arm-software.github.io/CMSIS_5/)

## Future Improvements

- Add error handling for sensor timeouts and I2C related errors
- Reduce clock frequency and implement power saving modes
- Improve modularity and abstraction
- Add graphical display elements
