#include "led.hpp"
#include "serial.hpp"
#include "imu.hpp"
#include "thrower.hpp"
#include <cstring>

#define RASPBERRY_UART &huart3
#define SERVO_UART &huart1

static led led;
static serial serial_raspberry(RASPBERRY_UART);
static serial serial_servo(SERVO_UART);
static imu imu_sensor(&hi2c1);
static thrower thrower1(&htim3, TIM_CHANNEL_1);
static thrower thrower2(&htim3, TIM_CHANNEL_2);

static led::Mode led_mode = led::Mode::BLINK_1S;

enum PacketState { WAIT_HEADER, IN_PACKET };
static PacketState pkt_state = WAIT_HEADER;
static uint8_t pkt_buf[32];
static uint8_t pkt_len = 0;

static uint8_t imu_buf[14];
static bool imu_pending = false;

static const uint8_t hex_chars[] = "0123456789ABCDEF";

static void hex_encode(const uint8_t* src, uint8_t* dst, size_t len)
{
    for (size_t i = 0; i < len; i++) {
        dst[i * 2]     = hex_chars[src[i] >> 4];
        dst[i * 2 + 1] = hex_chars[src[i] & 0x0F];
    }
}

static void process_raspberry_byte(uint8_t byte)
{
    switch (pkt_state) {
    case WAIT_HEADER:
        if (byte == '#') {
            pkt_state = IN_PACKET;
            pkt_len = 0;
        }
        break;
    case IN_PACKET:
        if (byte == '#') {
            pkt_len = 0;
        } else if (byte == '!') {
            if (pkt_len == 7 && std::memcmp(pkt_buf, "IMUREAD", 7) == 0) {
                if (!imu_sensor.is_busy()) {
                    if (imu_sensor.read_async(imu_buf)) {
                        imu_pending = true;
                    }
                }
            } else if (pkt_len == 12 && std::memcmp(pkt_buf, "THROW1_START", 12) == 0) {
                thrower1.open();
            } else if (pkt_len == 12 && std::memcmp(pkt_buf, "THROW1_CLOSE", 12) == 0) {
                thrower1.close();
            } else if (pkt_len == 12 && std::memcmp(pkt_buf, "THROW2_START", 12) == 0) {
                thrower2.open();
            } else if (pkt_len == 12 && std::memcmp(pkt_buf, "THROW2_CLOSE", 12) == 0) {
                thrower2.close();
            } else {
                serial_servo.write(pkt_buf, pkt_len);
            }
            pkt_state = WAIT_HEADER;
        } else {
            if (pkt_len < sizeof(pkt_buf)) {
                pkt_buf[pkt_len++] = byte;
            } else {
                pkt_state = WAIT_HEADER;
            }
        }
        break;
    }
}

extern "C" void app_init()
{
    serial_raspberry.init();
    serial_servo.init();
    imu_sensor.init();
    thrower1.close();
    thrower2.close();

    led_mode = led::Mode::BLINK_1S;
    led.SetMode(led::Mode::BLINK_1S);
}

extern "C" void app_loop()
{
    led.Update();

    bool busy = serial_raspberry.is_tx_busy() || serial_servo.is_tx_busy();
    led::Mode target = busy ? led::Mode::BLINK_500MS : led::Mode::BLINK_1S;
    if (target != led_mode) {
        led_mode = target;
        led.SetMode(target);
    }

    if (imu_pending && !imu_sensor.is_busy()) {
        imu_pending = false;
        uint8_t hex_buf[28];
        hex_encode(imu_buf, hex_buf, 14);
        serial_raspberry.write('#');
        serial_raspberry.write(hex_buf, 28);
        serial_raspberry.write('!');
    }

    while (serial_raspberry.available()) {
        process_raspberry_byte(static_cast<uint8_t>(serial_raspberry.read()));
    }

    while (serial_servo.available()) {
        uint8_t byte = static_cast<uint8_t>(serial_servo.read());
        serial_raspberry.write('#');
        serial_raspberry.write(byte);
        serial_raspberry.write('!');
    }
}
