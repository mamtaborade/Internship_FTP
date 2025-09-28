#include "stm32f4xx_hal.h"
#include "fatfs.h"
#include <stdlib.h>  // For abs()

I2C_HandleTypeDef hi2c1;
uint8_t accel_data[6];
FIL myFile;

// Threshold for crash detection (adjust based on sensor range)
#define THRESHOLD 2000
#define BUFFER_SIZE 10

// Structure for a single accelerometer reading
typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
} AccelReading_t;

// Circular buffer to store last 10 readings
AccelReading_t accel_buffer[BUFFER_SIZE];
uint8_t buffer_index = 0;

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_I2C1_Init();
    MX_FATFS_Init();  // Initialize FatFs for logging

    // Initialize file for appending crash logs
    if (f_open(&myFile, "crash_log.txt", FA_WRITE | FA_OPEN_ALWAYS) != FR_OK) {
        // Handle error (e.g., no SD card); for now, proceed without logging
        while (1) { HAL_Delay(1000); }
    }
    f_lseek(&myFile, f_size(&myFile));  // Seek to end for appending

    // LED GPIO initialization (assuming Red LED on GPIOD Pin 14; adjust as needed)
    __HAL_RCC_GPIOD_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_14;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);  // Ensure LED is off initially

    // Initialize circular buffer (optional: set all to 0)
    for (uint8_t i = 0; i < BUFFER_SIZE; i++) {
        accel_buffer[i].x = 0;
        accel_buffer[i].y = 0;
        accel_buffer[i].z = 0;
    }

    while (1) {
        HAL_I2C_Mem_Read(&hi2c1, 0x32, 0x28 | 0x80, 1, accel_data, 6, HAL_MAX_DELAY);
        int16_t x = (accel_data[1] << 8) | accel_data[0];
        int16_t y = (accel_data[3] << 8) | accel_data[2];
        int16_t z = (accel_data[5] << 8) | accel_data[4];

        printf("X=%d, Y=%d, Z=%d\r\n", x, y, z);

        // Update circular buffer with current reading
        accel_buffer[buffer_index].x = x;
        accel_buffer[buffer_index].y = y;
        accel_buffer[buffer_index].z = z;
        buffer_index = (buffer_index + 1) % BUFFER_SIZE;

        // Crash detection: Check if |X| or |Y| exceeds threshold
        if (abs(x) > THRESHOLD || abs(y) > THRESHOLD) {
            UINT bw;
            f_puts("CRASH EVENT\r\n", &myFile);

            // Log the last 10 readings from the circular buffer
            f_puts("Last 10 Accelerometer Readings:\r\n", &myFile);
            uint8_t start_idx = buffer_index;  // Start from the oldest (circular)
            for (uint8_t i = 0; i < BUFFER_SIZE; i++) {
                uint8_t idx = (start_idx + i) % BUFFER_SIZE;
                char log_str[64];
                sprintf(log_str, "Reading %d: X=%d, Y=%d, Z=%d\r\n", i + 1,
                        accel_buffer[idx].x, accel_buffer[idx].y, accel_buffer[idx].z);
                f_puts(log_str, &myFile);
            }
            f_puts("\r\n", &myFile);  // Separator for next event
            f_sync(&myFile);  // Flush to ensure data is written to SD card

            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);  // Turn on Red LED
            printf("CRASH DETECTED! Logged to file.\r\n");
        } else {
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);  // Turn off Red LED
        }

        HAL_Delay(500);
    }
}

