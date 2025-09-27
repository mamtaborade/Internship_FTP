#include "stm32f4xx_hal.h"

void SystemClock_Config(void);
void MX_GPIO_Init(void);

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();

    // Variables for timing the LED blinks (non-blocking using HAL_GetTick())
    uint32_t last_green = 0;   // Green LED (PD12) toggle interval: 500 ms (period 1 s)
    uint32_t last_orange = 0;  // Orange LED (PD13) toggle interval: 1000 ms (period 2 s)
    uint32_t last_blue = 0;    // Blue LED (PD15) toggle interval: 250 ms (period 500 ms)
    // Note: Assuming initial LED states are off (as configured in MX_GPIO_Init).

    // For buzzer: Assume buzzer is connected to GPIOC, GPIO_PIN_5 (configured as output in MX_GPIO_Init).
    // Button on PA0 (input, already configured) controls buzzer: ON when pressed, OFF when released.

    while (1) {
        uint32_t now = HAL_GetTick();

        // Blink Green LED (PD12) every 500 ms
        if (now - last_green >= 500) {
            HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12);
            last_green = now;
        }

        // Blink Orange LED (PD13) every 1000 ms
        if (now - last_orange >= 1000) {
            HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_13);
            last_orange = now;
        }

        // Blink Blue LED (PD15) every 250 ms (faster rate)
        if (now - last_blue >= 250) {
            HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_15);  // Assuming Blue LED on PD15 (adjust if different)
            last_blue = now;
        }

        // Button-controlled buzzer (GPIOC, GPIO_PIN_5)
        // Buzzer ON if button (PA0) is pressed (SET), OFF otherwise
        if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_SET) {
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_SET);  // Buzzer ON
        } else {
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_RESET);  // Buzzer OFF
        }

        // Small delay to prevent excessive CPU usage (optional, but good practice)
        HAL_Delay(10);
    }
}

