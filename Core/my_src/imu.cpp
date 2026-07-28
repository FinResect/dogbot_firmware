#include "imu.hpp"

static constexpr int MAX_INSTANCES = 2;
static imu* instances[MAX_INSTANCES] = {};
static int instance_count = 0;

extern "C" void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef* hi2c)
{
    for (int i = 0; i < instance_count; i++) {
        if (instances[i]) {
            instances[i]->on_rx_complete(hi2c);
        }
    }
}

extern "C" void HAL_I2C_ErrorCallback(I2C_HandleTypeDef* hi2c)
{
    for (int i = 0; i < instance_count; i++) {
        if (instances[i]) {
            instances[i]->on_error(hi2c);
        }
    }
}

imu::imu(I2C_HandleTypeDef* hi2c)
    : hi2c_(hi2c)
{
    if (instance_count < MAX_INSTANCES) {
        instances[instance_count++] = this;
    }
}

imu::~imu()
{
    for (int i = 0; i < instance_count; i++) {
        if (instances[i] == this) {
            instances[i] = instances[--instance_count];
            instances[instance_count] = nullptr;
            break;
        }
    }
}

bool imu::init()
{
    uint8_t cmd[2] = {REG_PWR, 0x00};
    return HAL_I2C_Master_Transmit(hi2c_, DEV_ADDR, cmd, 2, HAL_MAX_DELAY) == HAL_OK;
}

bool imu::read_async(uint8_t* buf)
{
    if (busy_) return false;
    busy_ = true;
    return HAL_I2C_Mem_Read_DMA(hi2c_, DEV_ADDR, REG_ACCEL,
                                I2C_MEMADD_SIZE_8BIT, buf, DATA_LEN) == HAL_OK;
}

bool imu::is_busy() const
{
    return busy_;
}

void imu::on_rx_complete(I2C_HandleTypeDef* hi2c)
{
    if (hi2c != hi2c_) return;
    busy_ = false;
}

void imu::on_error(I2C_HandleTypeDef* hi2c)
{
    if (hi2c != hi2c_) return;
    busy_ = false;
}
