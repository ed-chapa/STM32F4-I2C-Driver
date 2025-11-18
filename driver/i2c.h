#ifndef I2C_H
#define I2C_H

#include <stdbool.h>
#include <stdint.h>
#include "stm32f4xx.h"

#define I2C_MAX_DEVICES 128 // Max I2C allowed devices on the bus

typedef enum {
    I2C_OK = 0,
    I2C_ERROR_TIMEOUT,
    I2C_ERROR_BUSY,
    I2C_ERROR_BUS,
    I2C_ERROR_ACKFAIL,
    I2C_ERROR_NULLPTR,
    I2C_ERROR_INVALID_FREQ,
    I2C_ERROR_INVALID_MODE,
    I2C_ERROR_TIMING,
    I2C_ERROR_ARBITRATION,
    I2C_ERROR_INIT
} I2C_Status;

typedef enum {
    I2C_MODE_STANDARD = 0,
    I2C_MODE_FAST = 1
} I2C_Mode;

typedef enum {
    I2C_DUTY_2 = 0,
    I2C_DUTY_16_9 = 1
} I2C_Duty;

typedef enum {
    I2C_ACK_DISABLE = 0,
    I2C_ACK_ENABLE = 1
} I2C_Ack;

typedef enum {
    I2C_WRITE = 0,
    I2C_READ = 1
} I2C_Direction;

typedef struct {
    uint8_t addresses[I2C_MAX_DEVICES];
    uint32_t count;
} I2C_ScanResult;

typedef struct {
    I2C_TypeDef *instance;
    uint32_t clockFrequency;
    I2C_Mode mode;
    I2C_Duty duty;
    bool enableAck;
    uint16_t ownAddress;
    uint32_t timeout;
} I2C_Config;

void I2C_EnableClock(I2C_TypeDef *i2c);
void I2C_DisableClock(I2C_TypeDef *i2c);
I2C_Status I2C_Configure(I2C_TypeDef *i2c, const I2C_Config *config);
I2C_Status I2C_Start(I2C_TypeDef *i2c);
I2C_Status I2C_Stop(I2C_TypeDef *i2c);
I2C_Status I2C_SendAddress(I2C_TypeDef *i2c, uint8_t address, I2C_Direction dir);
I2C_Status I2C_WriteByte(I2C_TypeDef *i2c, uint8_t data);
I2C_Status I2C_Write(I2C_TypeDef *i2c, const uint8_t *data, uint32_t size);
I2C_Status I2C_ReadByte(I2C_TypeDef *i2c, uint8_t *data);
I2C_Status I2C_Read(I2C_TypeDef *i2c, uint8_t *buffer, uint32_t size);
I2C_Status I2C_MasterTransmit(I2C_TypeDef *i2c, uint8_t address, uint8_t *data, uint32_t size);
I2C_Status I2C_MasterReceive(I2C_TypeDef *i2c, uint8_t address, uint8_t *buffer, uint32_t size);
I2C_Status I2C_Scan(I2C_TypeDef *i2c, I2C_ScanResult *result);

#endif
