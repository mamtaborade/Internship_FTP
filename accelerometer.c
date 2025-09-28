#include "stm32f4xx_hal.h"
#include <stdlib.h>  // For abs()

I2C_HandleTypeDef hi2c1;
uint8_t accel_data[6];

// Threshold for crash detection (adjust based on sensor range, e.g., 2000 for ~2g if full scale is ±16g)
#define CRASH_THRESHOLD 2000

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_I2C1_Init();

    // Buzzer GPIO initialization (assuming buzzer connected to GPIOA Pin 5; adjust as needed)
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);  // Ensure buzzer is off initially

    while (1) {
        HAL_I2C_Mem_Read(&hi2c1, 0x32, 0x28 | 0x80, 1, accel_data, 6, HAL_MAX_DELAY);
        int16_t x = (accel_data[1] << 8) | accel_data[0];
        int16_t y = (accel_data[3] << 8) | accel_data[2];
        int16_t z = (accel_data[5] << 8) | accel_data[4];

        printf("X=%d, Y=%d, Z=%d\r\n", x, y, z);

        // Crash detection: Check if |X| or |Y| exceeds threshold
        if (abs(x) > CRASH_THRESHOLD || abs(y) > CRASH_THRESHOLD) {
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);  // Turn on buzzer
            printf("CRASH DETECTED!\r\n");
        } else {
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);  // Turn off buzzer
        }

        HAL_Delay(500);
    }
}
