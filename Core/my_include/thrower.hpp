#pragma once

#ifdef __cplusplus

#include "main.h"
#include "tim.h"

class thrower {
public:
    static constexpr uint16_t PULSE_CLOSE = 500;
    static constexpr uint16_t PULSE_OPEN  = 2000;

    thrower(TIM_HandleTypeDef* htim, uint32_t channel);

    void open();
    void close();

private:
    TIM_HandleTypeDef* htim_;
    uint32_t channel_;
};

#else
#error "thrower.hpp requires C++"
#endif
