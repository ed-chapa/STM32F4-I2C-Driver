# Bare-metal I2C Driver for STM32F410

Minimal bare-metal project targeting STM32F410 (Nucleo-style) that includes:
- a simple I2C driver ([driver/i2c.c](driver/i2c.c), [driver/i2c.h](driver/i2c.h))
- example application ([example/main.c](example/main.c))
- CMSIS headers and cortex startup code ([cmsis/core_cm4.h](cmsis/core_cm4.h), [cmsis/cmsis_gcc.h](cmsis/cmsis_gcc.h))
- device headers for STM32F410 ([device/stm32f410tx.h](device/stm32f410tx.h), [device/stm32f410rx.h](device/stm32f410rx.h))
- linker script and startup ([STM32F410RBTX_FLASH.ld](STM32F410RBTX_FLASH.ld), [startup/startup_stm32f410rbtx.s](startup/startup_stm32f410rbtx.s))

Purpose
- Provide a small, portable reference I2C implementation and example to run on STM32F4-family Cortex-M4 devices.

Quick start

Requirements
- arm-none-eabi toolchain (gcc, objcopy)
- stlink / st-flash (for flashing)

Build
- Build the firmware (uses the included [makefile](makefile)):
```sh
make build
```

Debug build (includes debug symbols):
```sh
make debug
```

Flash
- Use the provided make target:
```sh
make flash
```

Project layout (key files)
- [makefile](makefile) — build and flash targets
- [driver/i2c.c](driver/i2c.c), [driver/i2c.h](driver/i2c.h) — I2C driver
- [example/main.c](example/main.c) — application entry and peripheral init
- [example/system_stm32f4xx.c](example/system_stm32f4xx.c) — system init
- [example/syscalls.c](example/syscalls.c) — minimal newlib hooks
- [startup/startup_stm32f410rbtx.s](startup/startup_stm32f410rbtx.s) — vector table / reset handler
- [STM32F410RBTX_FLASH.ld](STM32F410RBTX_FLASH.ld) — linker script
- [cmsis/*](cmsis/) — CMSIS core headers used by the project
- [device/*.h](device/) — MCU register definitions

License
- See [LICENSE](LICENSE) (Apache-2.0).
