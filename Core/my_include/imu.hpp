#pragma once

#ifdef __cplusplus

#include "main.h"
#include "i2c.h"

class imu {
public:
    static constexpr uint8_t DEV_ADDR   = 0x68 << 1;
    static constexpr uint8_t REG_PWR    = 0x6B;
    static constexpr uint8_t REG_ACCEL  = 0x3B;
    static constexpr uint16_t DATA_LEN  = 14;

    explicit imu(I2C_HandleTypeDef* hi2c);
    ~imu();

    bool init();
    bool read_async(uint8_t* buf);
    bool is_busy() const;

    void on_rx_complete(I2C_HandleTypeDef* hi2c);
    void on_error(I2C_HandleTypeDef* hi2c);

private:
    I2C_HandleTypeDef* hi2c_;
    volatile bool busy_ = false;
};

#else
#error "imu.hpp requires C++"
#endif
