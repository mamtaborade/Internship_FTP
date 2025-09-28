#include "stm32f4xx_hal.h"
#include <stdio.h>
#include <string.h>

RTC_HandleTypeDef hrtc;
UART_HandleTypeDef huart1;  // Assuming UART1 is used for communication

// Receive buffer for UART commands
char rx_buffer[100];
uint8_t rx_index = 0;
uint8_t new_command = 0;

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_UART1_UART_Init();  // Add this if not already called elsewhere (CubeMX generated)
    MX_RTC_Init();

    // Initialize receive buffer
    rx_buffer[0] = '\0';
    rx_index = 0;
    new_command = 0;

    RTC_TimeTypeDef sTime;
    RTC_DateTypeDef sDate;

    while (1) {
        // Get and print current time
        HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
        HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
        printf("%02d:%02d:%02d\r\n", sTime.Hours, sTime.Minutes, sTime.Seconds);

        // Poll for UART input (non-blocking, single byte check for simplicity)
        uint8_t rx_byte;
        if (HAL_UART_Receive(&huart1, &rx_byte, 1, 0) == HAL_OK) {
            if (rx_byte == '\r' || rx_byte == '\n') {
                rx_buffer[rx_index] = '\0';
                new_command = 1;
                rx_index = 0;
            } else if (rx_index < 99) {
                rx_buffer[rx_index++] = (char)rx_byte;
            } else {
                rx_index = 0;  // Buffer overflow reset
            }
        }

        // Process command if a full line was received
        if (new_command) {
            // Check for "SET TIME HH:MM:SS" command
            if (strncmp(rx_buffer, "SET TIME ", 9) == 0) {
                int hh, mm, ss;
                if (sscanf(rx_buffer + 9, "%d:%d:%d", &hh, &mm, &ss) == 3) {
                    // Validate time range
                    if (hh >= 0 && hh < 24 && mm >= 0 && mm < 60 && ss >= 0 && ss < 60) {
                        // Set the time
                        sTime.Hours = (uint8_t)hh;
                        sTime.Minutes = (uint8_t)mm;
                        sTime.Seconds = (uint8_t)ss;
                        sTime.TimeFormat = RTC_HOURFORMAT24;
                        sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
                        sTime.StoreOperation = RTC_STOREOPERATION_RESET;

                        if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) == HAL_OK) {
                            printf("Time set to %02d:%02d:%02d\r\n", hh, mm, ss);
                        } else {
                            printf("Failed to set time\r\n");
                        }
                    } else {
                        printf("Invalid time values (HH:00-23, MM/SS:00-59)\r\n");
                    }
                } else {
                    printf("Invalid format. Use: SET TIME HH:MM:SS\r\n");
                }
            }
            // Reset for next command
            new_command = 0;
        }

        HAL_Delay(1000);
    }
}

