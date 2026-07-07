#include "led.hpp"

static led led1;

extern "C" void app_init()
{
    led1.SetMode(led::Mode::BLINK_1S);
}

extern "C" void app_loop()
{
    led1.Update();
}
