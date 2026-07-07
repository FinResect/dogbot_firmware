#include "led.hpp"

void led::SetMode(Mode mode)
{
    mode_ = mode;
    state_ = false;
    last_tick_ = HAL_GetTick();

    if (mode == OFF) {
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
    } else if (mode == ON) {
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
    }
}

void led::Update()
{
    uint32_t period;

    switch (mode_) {
    case BLINK_2S:
        period = BLINK_2S_PERIOD;
        break;
    case BLINK_1S:
        period = BLINK_1S_PERIOD;
        break;
    default:
        return;
    }

    uint32_t now = HAL_GetTick();
    if (now - last_tick_ >= period) {
        last_tick_ = now;
        state_ = !state_;
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin,
            state_ ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }
}
