#pragma once

#ifdef __cplusplus

#include "main.h"
#include "usart.h"

class serial {
public:
    explicit serial(UART_HandleTypeDef* huart);
    ~serial();

    void init();

    int available() const;
    int read();
    void write(uint8_t byte);
    void write(const uint8_t* data, uint16_t len);

    void on_rx_complete(UART_HandleTypeDef* huart);

private:
    void start_rx();

    UART_HandleTypeDef* huart_;

    static constexpr size_t BUF_SIZE = 256;
    static constexpr size_t BUF_MASK = BUF_SIZE - 1;

    uint8_t rx_buf_[BUF_SIZE];
    volatile size_t head_ = 0;
    size_t tail_ = 0;
    uint8_t rx_byte_ = 0;
};

#else
#error "serial.hpp requires C++"
#endif
