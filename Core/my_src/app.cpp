#include "led.hpp"
#include "serial.hpp"

#define RASPBERRY_UART &huart3
#define SERVO_UART &huart1

 static led led;
 static serial serial_raspberry(RASPBERRY_UART);
 static serial serial_servo(SERVO_UART);

extern "C" void app_init()
{
    serial_raspberry.init();
    serial_servo.init();

    led.SetMode(led::Mode::BLINK_1S);
}

extern "C" void app_loop()
{
    led.Update();
    while(serial_raspberry.available()) {
        int byte = serial_raspberry.read();
        serial_servo.write(static_cast<uint8_t>(byte));
    }
}
