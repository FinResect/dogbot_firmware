#include "serial.hpp"
#include <cstring>

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

extern "C" void HAL_UART_TxCpltCallback(UART_HandleTypeDef* huart)
{
    for (int i = 0; i < instance_count; i++) {
        if (instances[i]) {
            instances[i]->on_tx_complete(huart);
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
    HAL_UART_AbortTransmit(huart_);

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

bool serial::is_tx_busy() const
{
    return tx_busy_ || (tx_head_ != tx_tail_);
}

void serial::start_tx()
{
    if (tx_busy_) return;

    size_t available;
    if (tx_head_ >= tx_tail_) {
        available = tx_head_ - tx_tail_;
    } else {
        available = TX_BUF_SIZE - tx_tail_ + tx_head_;
    }
    if (available == 0) return;

    size_t first = TX_BUF_SIZE - tx_tail_;
    if (available <= first) {
        std::memcpy(tx_dma_buf_, tx_buf_ + tx_tail_, available);
    } else {
        std::memcpy(tx_dma_buf_, tx_buf_ + tx_tail_, first);
        std::memcpy(tx_dma_buf_ + first, tx_buf_, available - first);
    }

    tx_dma_len_ = available;
    tx_busy_ = true;
    HAL_UART_Transmit_DMA(huart_, tx_dma_buf_, tx_dma_len_);
}

void serial::on_tx_complete(UART_HandleTypeDef* huart)
{
    if (huart != huart_) return;

    tx_tail_ = (tx_tail_ + tx_dma_len_) & TX_BUF_MASK;
    tx_busy_ = false;

    start_tx();
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
    write(&byte, 1);
}

void serial::write(const uint8_t* data, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++) {
        size_t next = (tx_head_ + 1) & TX_BUF_MASK;
        while (next == tx_tail_) { }
        tx_buf_[tx_head_] = data[i];
        tx_head_ = next;
    }
    start_tx();
}
