#pragma once

#ifdef __cplusplus

#include "main.h"

class led {
public:
    enum Mode {
        OFF = 0,
        ON,
        BLINK_2S,
        BLINK_1S
    };

    void SetMode(Mode mode);
    void Update();

private:
    static constexpr uint32_t BLINK_2S_PERIOD = 1000U;
    static constexpr uint32_t BLINK_1S_PERIOD = 500U;

    Mode mode_ = OFF;
    uint32_t last_tick_ = 0;
    bool state_ = false;
};

#else
#error "led.hpp requires C++"
#endif

