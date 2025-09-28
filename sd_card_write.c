#include "fatfs.h"
#include "stm32f4xx_hal.h"
#include <stdio.h>
#include <string.h>

FIL myFile;
RTC_HandleTypeDef hrtc;

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_FATFS_Init();
    MX_RTC_Init();  // Initialize RTC (assumes CubeMX-generated function; ensure RTC calendar is started)

    if (f_open(&myFile, "log.txt", FA_WRITE | FA_OPEN_ALWAYS) == FR_OK) {
        f_lseek(&myFile, f_size(&myFile));  // Seek to end of file to append (creates if doesn't exist)

        UINT bw;
        // Write header only if file is new (size == 0)
        if (f_size(&myFile) == 0) {
            char text[] = "Hello STM32 BlackBox\r\n";
            f_write(&myFile, text, strlen(text), &bw);
        }

        // Infinite loop to append RTC time every second
        while (1) {
            RTC_TimeTypeDef sTime = {0};
            if (HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN) == HAL_OK) {
                char time_str[32];
                sprintf(time_str, "Time: %02d:%02d:%02d\r\n", sTime.Hours, sTime.Minutes, sTime.Seconds);
                f_write(&myFile, time_str, strlen(time_str), &bw);
                // Optional: Sync to storage periodically (e.g., every 10 writes) to ensure data persistence
                // f_sync(&myFile);
            } else {
                // Handle RTC read error if needed (e.g., log error message)
            }
            HAL_Delay(1000);
        }

        f_close(&myFile);  // Unreachable in infinite loop; add proper shutdown if needed
    } else {
        // Handle file open error (e.g., SD card not inserted); for now, idle
        while (1) {
            HAL_Delay(1000);
        }
    }
}

