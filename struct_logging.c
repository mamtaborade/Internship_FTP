#include "stm32f4xx_hal.h"
#include "fatfs.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>  // For abs()

// Assuming these are defined elsewhere (from previous code context)
I2C_HandleTypeDef hi2c1;
uint8_t accel_data[6];
FIL myFile;
RTC_HandleTypeDef hrtc;
#define THRESHOLD 2000  // Crash threshold (adjust as needed)

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_I2C1_Init();
    MX_FATFS_Init();
    MX_RTC_Init();  // Initialize RTC

    // Open file for appending logs
    if (f_open(&myFile, "blackbox_log.txt", FA_WRITE | FA_OPEN_ALWAYS) != FR_OK) {
        // Handle error (e.g., no SD card); for now, idle
        while (1) { HAL_Delay(1000); }
    }
    f_lseek(&myFile, f_size(&myFile));  // Seek to end for appending

    UINT bw;
    RTC_TimeTypeDef sTime = {0};
    int16_t x, y, z;

    while (1) {
        // Read accelerometer data
        HAL_I2C_Mem_Read(&hi2c1, 0x32, 0x28 | 0x80, 1, accel_data, 6, HAL_MAX_DELAY);
        x = (accel_data[1] << 8) | accel_data[0];
        y = (accel_data[3] << 8) | accel_data[2];
        z = (accel_data[5] << 8) | accel_data[4];

        // Read RTC time
        HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);

        // Compute crash flag (1 if |X| or |Y| > THRESHOLD, else 0)
        int crash_flag = (abs(x) > THRESHOLD || abs(y) > THRESHOLD) ? 1 : 0;

        // Format log entry with time, accel values, and crash flag
        char buffer[64];
        snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d,%d,%d,%d,%d\r\n",
                 sTime.Hours, sTime.Minutes, sTime.Seconds, x, y, z, crash_flag);

        // Write to file
        f_write(&myFile, buffer, strlen(buffer), &bw);

        // Optional: Sync periodically (e.g., every 10 iterations) for data safety
        // static uint8_t sync_counter = 0;
        // if (++sync_counter % 10 == 0) f_sync(&myFile);

        // Optional: Print to console for debugging
        printf("Logged: %s", buffer);

        // Optional: Handle crash (e.g., LED or buzzer as in previous code)
        if (crash_flag) {
            printf("CRASH DETECTED!\r\n");
            // Add LED/buzzer code here if needed
        }

        HAL_Delay(500);  // Log every 500ms
    }

    f_close(&myFile);  // Unreachable; add proper shutdown if needed
}

