# szczupak-light-firmware

Firmware for the Szczupak Light board. Board drives 3x 10W LED
channels on an underwater autonomous robot, measures power on each channel, and
detects and handles LED faults. The board support communication 
over UART, USB, and CAN.

The project includes a custom bootloader with the ability 
to flash firmware via CAN. The application includes fault recovery mechanism that records fatal errors and resets MCU into the bootloader. 
The bootloader prevents faulty application from restarting, reports the error status over CAN and remains ready to receive new firmware.


---

## Requirements

* STM32CubeMX
* CMake 3.25 or newer
* Ninja
* ARM GNU Toolchain
* OpenOCD

---
