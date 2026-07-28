#include "thrower.hpp"

thrower::thrower(TIM_HandleTypeDef* htim, uint32_t channel)
    : htim_(htim)
    , channel_(channel)
{
}

void thrower::open()
{
    __HAL_TIM_SET_COMPARE(htim_, channel_, PULSE_OPEN);
}

void thrower::close()
{
    __HAL_TIM_SET_COMPARE(htim_, channel_, PULSE_CLOSE);
}
