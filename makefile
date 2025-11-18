CC = arm-none-eabi-gcc
LD = arm-none-eabi-ld
OBJCOPY = arm-none-eabi-objcopy
CFLAGS = -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard -Wall
LDSCRIPT = STM32F410RBTX_FLASH.ld

build:
	$(CC) -I CMSIS -I device -I driver -c example/main.c $(CFLAGS) -o build/main.o
	$(CC) -I CMSIS -I device -I driver -c driver/i2c.c $(CFLAGS) -o build/i2c.o
	$(CC) -I CMSIS -I device -I driver -c example/syscalls.c $(CFLAGS) -o build/syscalls.o
	$(CC) -I CMSIS -I device -I driver -c example/system_stm32f4xx.c $(CFLAGS) -o build/system_stm32f4xx.o
	$(CC) -I CMSIS -I device -I driver -c startup/startup_stm32f410rbtx.s $(CFLAGS) -o build/startup_stm32f410rbtx.o
	$(CC) -T $(LDSCRIPT) build/*.o -o build/firmware.elf -lc -lm -lnosys -u _printf_float -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard
	$(OBJCOPY) -O binary build/firmware.elf build/firmware.bin

debug:
	$(CC) -I CMSIS -I device -I driver -g -c example/main.c $(CFLAGS) -o build/main.o
	$(CC) -I CMSIS -I device -I driver -g -c driver/i2c.c $(CFLAGS) -o build/i2c.o
	$(CC) -I CMSIS -I device -I driver -g -c example/syscalls.c $(CFLAGS) -o build/syscalls.o
	$(CC) -I CMSIS -I device -I driver -g -c example/system_stm32f4xx.c $(CFLAGS) -o build/system_stm32f4xx.o
	$(CC) -I CMSIS -I device -I driver -g -c startup/startup_stm32f410rbtx.s $(CFLAGS) -o build/startup_stm32f410rbtx.o
	$(CC) -T $(LDSCRIPT) build/*.o -o build/firmware.elf -lc -lm -lnosys -u _printf_float -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard
	$(OBJCOPY) -O binary build/firmware.elf build/firmware.bin

flash:
	st-flash write ./build/firmware.bin 0x8000000

clean:
	rm -f build/*.o build/*.elf build/*.bin

.PHONY: build debug flash clean
