#include "i2c.h"
#include "stm32f4xx.h"
#include "system_stm32f4xx.h"
#include <stddef.h>

// I2C standard speeds
static const uint32_t I2C_STANDARD_SPEED_HZ      = 100000U;     // 100 kHz standard mode
static const uint32_t I2C_FAST_SPEED_HZ          = 400000U;     // 400 kHz fast mode

// Duty cycle divisors
static const uint32_t I2C_CCR_DIV_STANDARD       = 2U;          // CCR divisor for standard mode
static const uint32_t I2C_CCR_DIV_FAST_DUTY_2    = 3U;          // CCR divisor for fast mode, duty = 2
static const uint32_t I2C_CCR_DIV_FAST_DUTY_16_9 = 25U;         // CCR divisor for fast mode, duty = 16/9

// Timing constants
static const uint32_t I2C_TRISE_CONV_MHZ         = 1000000U;    // Hz-to-MHz conversion factor
static const uint32_t I2C_TRISE_FAST_NS          = 300U;        // Max rise time in Fast mode (ns)
static const uint32_t I2C_NS_PER_SEC             = 1000000000U; // Nanoseconds per second

// Prototypes for helper functions
I2C_Status I2C_WaitForFlag(I2C_TypeDef *i2c, uint32_t flag, uint32_t timeout);

void I2C_EnableClock(I2C_TypeDef *i2c) {
    if (i2c == I2C1) {
        RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
    } else if (i2c == I2C2) {
        RCC->APB1ENR |= RCC_APB1ENR_I2C2EN;
    }
}

void I2C_DisableClock(I2C_TypeDef *i2c) {
    if (i2c == I2C1) {
        RCC->APB1ENR &= ~RCC_APB1ENR_I2C1EN;
    } else if (i2c == I2C2) {
        RCC->APB1ENR &= ~RCC_APB1ENR_I2C2EN;
    }
}

I2C_Status I2C_Configure(I2C_TypeDef *i2c, const I2C_Config *config) {
    // Check for invalid inputs
    if (i2c == NULL || config == NULL) {
        return I2C_ERROR_NULLPTR;
    }
    if (config->clockFrequency == 0) {
        return I2C_ERROR_INVALID_FREQ;
    }

    // Set the frequency
    uint32_t freqMHz = config->clockFrequency / 1000000;
    if (freqMHz < 2 || freqMHz > 42) {
        return I2C_ERROR_INVALID_FREQ;
    }
    i2c->CR2 &= ~I2C_CR2_FREQ_Msk;
    i2c->CR2 |= (freqMHz << I2C_CR2_FREQ_Pos);

    // Set the mode and duty cycle
    i2c->CCR &= ~(I2C_CCR_FS | I2C_CCR_DUTY);
    i2c->CCR |= (config->mode << I2C_CCR_FS_Pos);
    i2c->CCR |= (config->duty << I2C_CCR_DUTY_Pos);

    // Calculate CCR and TRISE
    int ccr = 0;
    int trise = 0;

    uint32_t freq = config->clockFrequency;
    if (config->mode == I2C_MODE_STANDARD) { // Standard mode (100 kHz)
        ccr = freq / (I2C_CCR_DIV_STANDARD * I2C_STANDARD_SPEED_HZ);
        trise = (freq / I2C_TRISE_CONV_MHZ) + 1;
    } else if (config->mode == I2C_MODE_FAST) {
        if (config->duty == I2C_DUTY_2) { // Fast mode with duty cycle = 2
            ccr = freq / (I2C_CCR_DIV_FAST_DUTY_2 * I2C_FAST_SPEED_HZ);
        }
        else if (config->duty == I2C_DUTY_16_9) { // Fast mode with duty cycle = 16/9
            ccr = freq / (I2C_CCR_DIV_FAST_DUTY_16_9 * I2C_FAST_SPEED_HZ);
        }
        trise = ((freq * I2C_TRISE_FAST_NS) / I2C_NS_PER_SEC) + 1;
    }

    if (ccr == 0 || trise == 0) {
        return I2C_ERROR_TIMING; // Invalid timing
    }

    // Set CCR and TRISE
    i2c->CCR &= ~I2C_CCR_CCR;
    i2c->CCR |= (ccr << I2C_CCR_CCR_Pos);
    i2c->TRISE = trise;

    // Configure ACK
    if (config->enableAck) {
        i2c->CR1 |= I2C_CR1_ACK;
    } else {
        i2c->CR1 &= ~I2C_CR1_ACK;
    }

    // Enable peripheral
    i2c->CR1 |= I2C_CR1_PE;

    return I2C_OK;
}

I2C_Status I2C_Start(I2C_TypeDef *i2c) {
    // Check if bus is busy
    if (i2c->SR2 & I2C_SR2_BUSY) {
        return I2C_ERROR_BUSY;
    }

    // Request START condition
    i2c->CR1 |= I2C_CR1_START;

    // Wait until SB flag is set
    uint32_t timeout = 100000;
    while (!(i2c->SR1 & I2C_SR1_SB)) {
        if (--timeout == 0) {
            return I2C_ERROR_TIMEOUT;
        }
    }

    // Check for arbitration loss
    if (i2c->SR1 & I2C_SR1_ARLO) {
        return I2C_ERROR_ARBITRATION;
    }

    return I2C_OK;
}

I2C_Status I2C_Stop(I2C_TypeDef *i2c) {
    // No need to stop if bus is idle
    if (!(i2c->SR2 & I2C_SR2_BUSY)) {
        return I2C_OK;
    }

    // Request STOP condition
    i2c->CR1 |= I2C_CR1_STOP;

    // Wait until BUSY clears
    uint32_t timeout = 100000;
    while (i2c->SR2 & I2C_SR2_BUSY) {
        if (--timeout == 0) {
            return I2C_ERROR_TIMEOUT;
        }
    }

    return I2C_OK;
}

I2C_Status I2C_SendAddress(I2C_TypeDef *i2c, uint8_t address, I2C_Direction dir) {
    I2C_Status status;

    // Wait for start bit
    status = I2C_WaitForFlag(i2c, I2C_SR1_SB, 100000);
    if (status != I2C_OK) return status;

    // Send address and direction
    i2c->DR = ((address << 1) | (uint8_t)dir);

    status = I2C_WaitForFlag(i2c, I2C_SR1_ADDR, 100000);
    if (status != I2C_OK) return status;

    // Required read to SR1 and SR2 as per the reference manual
    (void)i2c->SR1;
    (void)i2c->SR2;

    return I2C_OK;
}

I2C_Status I2C_WriteByte(I2C_TypeDef *i2c, uint8_t data) {
    I2C_Status status;

    // Wait until TXE
    status = I2C_WaitForFlag(i2c, I2C_SR1_TXE, 100000);
    if (status != I2C_OK) return status;
    // Write data
    i2c->DR = data;

    // Wait until BTF
    status = I2C_WaitForFlag(i2c, I2C_SR1_BTF, 100000);
    if (status != I2C_OK) return status;

    return I2C_OK;
}

I2C_Status I2C_Write(I2C_TypeDef *i2c, const uint8_t *data, uint32_t size) {
    if (data == NULL || size == 0) {
        return I2C_ERROR_INIT; // invalid parameters
    }

    for (uint32_t i = 0; i < size; ++i) {
        I2C_Status status = I2C_WriteByte(i2c, data[i]);
        if (status != I2C_OK) {
            return status;
        }
    }

    return I2C_OK;
}

I2C_Status I2C_ReadByte(I2C_TypeDef *i2c, uint8_t *data) {
    if (data == NULL) {
        return I2C_ERROR_INIT; // invalid pointer
    }

    I2C_Status status;

    status = I2C_WaitForFlag(i2c, I2C_SR1_RXNE, 100000);
    if (status != I2C_OK) return status;

    *data = (uint8_t)i2c->DR;
    return I2C_OK;
}

I2C_Status I2C_Read(I2C_TypeDef *i2c, uint8_t *buffer, uint32_t size) {
    if (buffer == NULL || size == 0) {
        return I2C_ERROR_INIT; // invalid parameters
    }

    for (uint32_t i = 0; i < size; ++i) {
        I2C_Status status = I2C_ReadByte(i2c, &buffer[i]);
        if (status != I2C_OK) {
            return status; // propagate error immediately
        }
    }

    return I2C_OK;
}

I2C_Status I2C_MasterTransmit(I2C_TypeDef *i2c, uint8_t address, uint8_t *data, uint32_t size) {
    I2C_Status status;

    // Generate START
    status = I2C_Start(i2c);
    if (status != I2C_OK) return status;

    // Send address (write)
    status = I2C_SendAddress(i2c, address, I2C_WRITE);
    if (status != I2C_OK) return status;

    // Send data
    status = I2C_Write(i2c, data, size);
    if (status != I2C_OK) return status;

    // Generate STOP
    I2C_Stop(i2c);

    return I2C_OK;
}

I2C_Status I2C_MasterReceive(I2C_TypeDef *i2c, uint8_t address, uint8_t *buffer, uint32_t size) {
    I2C_Status status;

    if (buffer == NULL || size == 0) return I2C_ERROR_INIT;

    // Generate START
    status = I2C_Start(i2c);
    if (status != I2C_OK) return status;

    // Send address (read)
    status = I2C_SendAddress(i2c, address, I2C_READ);
    if (status != I2C_OK) return status;

    // Enable ACK for multi-byte
    if (size > 1) i2c->CR1 |= I2C_CR1_ACK;

    for (uint32_t i = 0; i < size; ++i) {
        // Disable ACK before last byte
        if (i == size - 1) {
            i2c->CR1 &= ~I2C_CR1_ACK;
        }

        status = I2C_ReadByte(i2c, &buffer[i]);
        if (status != I2C_OK) return status;
    }

    // Generate STOP
    I2C_Stop(i2c);

    return I2C_OK;
}

I2C_Status I2C_Scan(I2C_TypeDef *i2c, I2C_ScanResult *result) {
    if (result == NULL) return I2C_ERROR_INIT;

    I2C_Status status;

    result->count = 0;

    for (uint8_t addr = 0; addr < 128; addr++) {
        // Send START condition
        i2c->CR1 |= I2C_CR1_START;

        // Wait for SB
        status = I2C_WaitForFlag(i2c, I2C_SR1_SB, 100000);
        if (status != I2C_OK) return status;

        // Send address with write bit
        i2c->DR = (addr << 1);

        status = I2C_WaitForFlag(i2c, (I2C_SR1_ADDR | I2C_SR1_AF), 100000);
        if (status != I2C_OK) return status;

        if (i2c->SR1 & I2C_SR1_ADDR) {
            // ACK received
            (void)i2c->SR1;
            (void)i2c->SR2;

            // Store address
            if (result->count < I2C_MAX_DEVICES) {
                result->addresses[result->count++] = addr;
            }
        } else {
            // NACK received
            i2c->SR1 &= ~I2C_SR1_AF;
        }

        // Send STOP condition
        i2c->CR1 |= I2C_CR1_STOP;

        // Small delay between attempts
        for (volatile int i = 0; i < 1000; i++);
    }

    return (result->count > 0) ? I2C_OK : I2C_ERROR_ACKFAIL;
}

I2C_Status I2C_WaitForFlag(I2C_TypeDef *i2c, uint32_t flag, uint32_t timeout) {
    while (!(i2c->SR1 & flag)) {
        if (i2c->SR1 & I2C_SR1_ARLO) { // Arbitration loss
            return I2C_ERROR_ARBITRATION;
        }
        if (i2c->SR1 & I2C_SR1_BERR) { // Bus error
            I2C_Stop(i2c);
            i2c->SR1 &= ~I2C_SR1_BERR;
            return I2C_ERROR_BUS;
        }
        if (i2c->SR1 & I2C_SR1_AF) { // Acknowledge failure
            I2C_Stop(i2c);
            i2c->SR1 &= ~I2C_SR1_AF;
            return I2C_ERROR_ACKFAIL;
        }
        if (--timeout == 0) { // Timeout
            I2C_Stop(i2c);
            return I2C_ERROR_TIMEOUT;
        }
    }

    return I2C_OK;
}
