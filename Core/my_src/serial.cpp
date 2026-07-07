#include "serial.hpp"

static constexpr int MAX_INSTANCES = 4;
static serial* instances[MAX_INSTANCES] = {};
static int instance_count = 0;

extern "C" void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart)
{
    for (int i = 0; i < instance_count; i++) {
        if (instances[i]) {
            instances[i]->on_rx_complete(huart);
        }
    }
}

serial::serial(UART_HandleTypeDef* huart)
    : huart_(huart)
{
    if (instance_count < MAX_INSTANCES) {
        instances[instance_count++] = this;
    }
}

void serial::init()
{
    start_rx();
}

serial::~serial()
{
    HAL_UART_AbortReceive_IT(huart_);

    for (int i = 0; i < instance_count; i++) {
        if (instances[i] == this) {
            instances[i] = instances[--instance_count];
            instances[instance_count] = nullptr;
            break;
        }
    }
}

void serial::start_rx()
{
    HAL_UART_Receive_IT(huart_, &rx_byte_, 1);
}

void serial::on_rx_complete(UART_HandleTypeDef* huart)
{
    if (huart != huart_) return;

    size_t next = (head_ + 1) & BUF_MASK;
    if (next != tail_) {
        rx_buf_[head_] = rx_byte_;
        head_ = next;
    }

    start_rx();
}

int serial::available() const
{
    return (head_ >= tail_) ? (head_ - tail_) : (BUF_SIZE - tail_ + head_);
}

int serial::read()
{
    if (tail_ == head_) return -1;

    uint8_t byte = rx_buf_[tail_];
    tail_ = (tail_ + 1) & BUF_MASK;
    return byte;
}

void serial::write(uint8_t byte)
{
    HAL_UART_Transmit(huart_, &byte, 1, HAL_MAX_DELAY);
}

void serial::write(const uint8_t* data, uint16_t len)
{
    HAL_UART_Transmit(huart_, const_cast<uint8_t*>(data), len, HAL_MAX_DELAY);
}
