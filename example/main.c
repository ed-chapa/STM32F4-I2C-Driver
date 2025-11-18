#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "system_stm32f4xx.h"
#include "stm32f4xx.h"
#include "i2c.h"

static const uint8_t OLED_ADDRESS = 0x3C; // Display address

// Required commands to initialize the display
uint8_t ssd1306_init_sequence[] = {
  0xAE,             // Display OFF
  0xD5, 0x80,       // Set Display Clock Divide Ratio/Oscillator Frequency
  0xA8, 0x3F,       // Set Multiplex Ratio (1 to 64)
  0xD3, 0x00,       // Set Display Offset
  0x40,             // Set Display Start Line
  0x8D, 0x14,       // Enable charge pump regulator
  0x20, 0x00,       // Set Memory Addressing Mode to Horizontal
  0xA1,             // Set Segment Re-map (column address 127 mapped to SEG0)
  0xC8,             // Set COM Output Scan Direction (remapped mode)
  0xDA, 0x12,       // Set COM Pins Hardware Configuration
  0x81, 0xCF,       // Set Contrast Control
  0xD9, 0xF1,       // Set Pre-charge Period
  0xDB, 0x40,       // Set VCOMH Deselect Level
  0xA4,             // Entire Display ON (resume to RAM content)
  0xA6,             // Set Normal Display (not inverted)
  0xAF              // Display ON
};

// Array containing the font
const uint8_t font5x8[][5] = {
    // Digits
    {0x3E,0x51,0x49,0x45,0x3E}, // 0
    {0x00,0x42,0x7F,0x40,0x00}, // 1
    {0x42,0x61,0x51,0x49,0x46}, // 2
    {0x21,0x41,0x45,0x4B,0x31}, // 3
    {0x18,0x14,0x12,0x7F,0x10}, // 4
    {0x27,0x45,0x45,0x45,0x39}, // 5
    {0x3C,0x4A,0x49,0x49,0x30}, // 6
    {0x01,0x71,0x09,0x05,0x03}, // 7
    {0x36,0x49,0x49,0x49,0x36}, // 8
    {0x06,0x49,0x49,0x29,0x1E}, // 9

    // Letters
    {0x7E,0x11,0x11,0x11,0x7E}, // A
    {0x7F,0x49,0x49,0x49,0x36}, // B
    {0x3E,0x41,0x41,0x41,0x22}, // C
    {0x7F,0x41,0x41,0x22,0x1C}, // D
    {0x7F,0x49,0x49,0x49,0x41}, // E
    {0x7F,0x09,0x09,0x09,0x01}, // F
    {0x3E,0x41,0x49,0x49,0x7A}, // G
    {0x7F,0x08,0x08,0x08,0x7F}, // H
    {0x00,0x41,0x7F,0x41,0x00}, // I
    {0x20,0x40,0x41,0x3F,0x01}, // J
    {0x7F,0x08,0x14,0x22,0x41}, // K
    {0x7F,0x40,0x40,0x40,0x40}, // L
    {0x7F,0x02,0x0C,0x02,0x7F}, // M
    {0x7F,0x04,0x08,0x10,0x7F}, // N
    {0x3E,0x41,0x41,0x41,0x3E}, // O
    {0x7F,0x09,0x09,0x09,0x06}, // P
    {0x3E,0x41,0x51,0x21,0x5E}, // Q
    {0x7F,0x09,0x19,0x29,0x46}, // R
    {0x46,0x49,0x49,0x49,0x31}, // S
    {0x01,0x01,0x7F,0x01,0x01}, // T
    {0x3F,0x40,0x40,0x40,0x3F}, // U
    {0x1F,0x20,0x40,0x20,0x1F}, // V
    {0x7F,0x20,0x18,0x20,0x7F}, // W
    {0x63,0x14,0x08,0x14,0x63}, // X
    {0x07,0x08,0x70,0x08,0x07}, // Y
    {0x61,0x51,0x49,0x45,0x43}, // Z

    // Puntuations
    {0x00, 0x60, 0x60, 0x00, 0x00}, // .
    {0x00, 0x80, 0x60, 0x00, 0x00}, // ,
    {0x00, 0x36, 0x36, 0x00, 0x00}, // :
    {0x62, 0x64, 0x08, 0x13, 0x23}, // %
};

I2C_Config I2C1_Config = {
    .instance = I2C1,
    .clockFrequency = 42000000,
    .mode = I2C_MODE_STANDARD,
    .enableAck = false
};

void SystemClock_Init(void) {
    RCC->CR |= RCC_CR_HSION;                                                // Enable HSI
    while (!(RCC->CR & RCC_CR_HSIRDY));                                     // Wait for HSI to be ready

    RCC->APB1ENR |= RCC_APB1ENR_PWREN;                                      // Enable power interface clock
    PWR->CR &= ~PWR_CR_VOS;                                                 // Clear bits
    PWR->CR |= (2 << PWR_CR_VOS_Pos);                                       // Set the voltage regulator to Scale 2

    FLASH->ACR |= FLASH_ACR_DCEN;                                           // Enable data cache
    FLASH->ACR |= FLASH_ACR_ICEN;                                           // Enable instruction cache
    FLASH->ACR |= FLASH_ACR_PRFTEN;                                         // Enable prefetch buffer
    FLASH->ACR &= ~FLASH_ACR_LATENCY;                                       // Clear bits
    FLASH->ACR |= FLASH_ACR_LATENCY_2WS;                                    // Set the flash latency to 2 wait states

    RCC->CFGR &= ~RCC_CFGR_HPRE;                                            // Clear bits
    RCC->CFGR |= RCC_CFGR_HPRE_DIV1;                                        // Don't divide AHB clock frequency
    RCC->CFGR &= ~RCC_CFGR_PPRE1;                                           // Clear bits
    RCC->CFGR |= RCC_CFGR_PPRE1_DIV2;                                       // Divide APB1 clock frequency by 2
    RCC->CFGR &= ~RCC_CFGR_PPRE2;                                           // Don't divide APB2 clock frequency

    RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLSRC;                                    // Set HSI as PLL clock source
    RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLM;                                      // Clear bits
    RCC->PLLCFGR |= (16 << RCC_PLLCFGR_PLLM_Pos);                           // Set main PLL division factor to 16
    RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLN;                                      // Clear bits
    RCC->PLLCFGR |= (336 << RCC_PLLCFGR_PLLN_Pos);                          // Set main PLL multiplication factor to 336
    RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLP;                                      // Clear bits
    RCC->PLLCFGR |= (1 << RCC_PLLCFGR_PLLP_Pos);                            // Set main PLL division factor to 4

    RCC->CR |= RCC_CR_PLLON;                                                // Enable PLL
    while(!(RCC->CR & RCC_CR_PLLRDY));                                      // Wait for PLL to be ready

    RCC->CFGR &= ~RCC_CFGR_SW;                                              // Clear bits
    RCC->CFGR |= RCC_CFGR_SW_PLL;                                           // Set PLL as system clock
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);                 // Wait for PLL to be set as system clock

    SystemCoreClockUpdate();                                                // Update SystemCoreClock variable
}

void GPIO_Init(void) {
    // 1. Enable GPIOB clock
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

    // 2. Configure PB8 and PB9 as Alternate Function
    GPIOB->MODER &= ~(GPIO_MODER_MODER8 | GPIO_MODER_MODER9);   // Clear mode
    GPIOB->MODER |= (GPIO_MODER_MODER8_1 | GPIO_MODER_MODER9_1); // Set to AF mode (10)

    // 3. Set output type to Open-Drain
    GPIOB->OTYPER |= (GPIO_OTYPER_OT8 | GPIO_OTYPER_OT9);

    // 4. Set speed to Very High (recommended for I2C)
    GPIOB->OSPEEDR |= (GPIO_OSPEEDER_OSPEEDR8 | GPIO_OSPEEDER_OSPEEDR9);

    // 5. Enable pull-up resistors (I2C requires pull-ups)
    GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPDR8 | GPIO_PUPDR_PUPDR9);
    GPIOB->PUPDR |= (GPIO_PUPDR_PUPDR8_0 | GPIO_PUPDR_PUPDR9_0); // Pull-up

    // 6. Select AF4 (I2C1) for PB8 and PB9
    GPIOB->AFR[1] &= ~((0xF << ((8-8)*4)) | (0xF << ((9-8)*4))); // Clear AFR bits
    GPIOB->AFR[1] |=  ((4 << ((8-8)*4)) | (4 << ((9-8)*4)));     // AF4 for I2C1
}

void I2C_Init(void) {
    I2C_EnableClock(I2C1);
    I2C_Configure(I2C1, &I2C1_Config);
}

void SSD1306_SendCommand(uint8_t cmd) {
    I2C_Start(I2C1);
    I2C_SendAddress(I2C1, OLED_ADDRESS, I2C_WRITE); // Write mode
    I2C_WriteByte(I2C1, 0x00);              // Control byte: command
    I2C_WriteByte(I2C1, cmd);               // Actual command
    I2C_Stop(I2C1);
}

void SSD1306_ClearDisplay(void) {
    for (uint8_t page = 0; page < 8; page++) {
        // Set column and page address
        I2C_Start(I2C1);
        I2C_SendAddress(I2C1, OLED_ADDRESS, I2C_WRITE); // Write mode
        I2C_WriteByte(I2C1, 0x00);              // Control byte: command
        I2C_WriteByte(I2C1, 0x21);              // Set column address
        I2C_WriteByte(I2C1, 0);                 // Start column
        I2C_WriteByte(I2C1, 127);               // End column
        I2C_WriteByte(I2C1, 0x22);              // Set page address
        I2C_WriteByte(I2C1, page);              // Start page
        I2C_WriteByte(I2C1, page);              // End page
        I2C_Stop(I2C1);

        // Write 128 zeros to clear the page
        I2C_Start(I2C1);
        I2C_SendAddress(I2C1, OLED_ADDRESS, I2C_WRITE); // Write mode
        I2C_WriteByte(I2C1, 0x40);             // Control byte: data
        for (uint8_t col = 0; col < 128; col++) {
            I2C_WriteByte(I2C1, 0x00);
        }
        I2C_Stop(I2C1);
    }
}

void SSD1306_Init(void) {
    for (int i = 0; i < 25; ++i)
    {
        SSD1306_SendCommand(ssd1306_init_sequence[i]);
    }

    SSD1306_ClearDisplay();
}

void SSD1306_DrawText(const char *text, uint8_t startX, uint8_t startPage) {
    uint8_t page = startPage;
    uint8_t x = startX;
    uint8_t buffer[128] = {0};

    while (*text && page < 8) {
        if (*text == '\n') {
            // Send current page
            I2C_Start(I2C1);
            I2C_SendAddress(I2C1, OLED_ADDRESS, I2C_WRITE);
            I2C_WriteByte(I2C1, 0x00);
            I2C_WriteByte(I2C1, 0x21);
            I2C_WriteByte(I2C1, 0);
            I2C_WriteByte(I2C1, 127);
            I2C_WriteByte(I2C1, 0x22);
            I2C_WriteByte(I2C1, page);
            I2C_WriteByte(I2C1, page);
            I2C_Stop(I2C1);

            I2C_Start(I2C1);
            I2C_SendAddress(I2C1, OLED_ADDRESS, I2C_WRITE);
            I2C_WriteByte(I2C1, 0x40);
            I2C_Write(I2C1, buffer, 128);
            I2C_Stop(I2C1);

            // Move to next line
            page++;
            x = startX;
            memset(buffer, 0, 128);
            text++;
            continue;
        }

        char c = *text++;
        uint8_t index;

        if (c >= '0' && c <= '9') {
            index = c - '0';
        } else if (c >= 'A' && c <= 'Z') {
            index = c - 'A' + 10;
        } else if (c == '.') {
            index = 36;
        } else if (c == ',') {
            index = 37;
        } else if (c == ':') {
            index = 38;
        } else if (c == '%') {
           index = 39;
        } else if (c == '!') {
           index = 40;
        } else {
            x += 6;
            continue;
        }

        if (x < 128 - 6) {
            for (int i = 0; i < 5; i++) {
                buffer[x + i] = font5x8[index][i];
            }
            x += 6;
        }
    }

    // Send final page
    I2C_Start(I2C1);
    I2C_SendAddress(I2C1, OLED_ADDRESS, I2C_WRITE);
    I2C_WriteByte(I2C1, 0x00);
    I2C_WriteByte(I2C1, 0x21);
    I2C_WriteByte(I2C1, 0);
    I2C_WriteByte(I2C1, 127);
    I2C_WriteByte(I2C1, 0x22);
    I2C_WriteByte(I2C1, page);
    I2C_WriteByte(I2C1, page);
    I2C_Stop(I2C1);

    I2C_Start(I2C1);
    I2C_SendAddress(I2C1, OLED_ADDRESS, I2C_WRITE);
    I2C_WriteByte(I2C1, 0x40);
    I2C_Write(I2C1, buffer, 128);
    I2C_Stop(I2C1);
}

int main(void) {
    SystemClock_Init();
    GPIO_Init();
    I2C_Init();

    SSD1306_Init();
    SSD1306_DrawText("HELLO, WORLD.", 0, 0); // Draw the text at the top left

    while(1) {
        // loop forever
    }
}
