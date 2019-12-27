#ifndef DS18B20_H
#define DS18B20_H

#include "mbed.h"

class DS18B20 {
public:
    DS18B20(PinName pin);

    // Returns temperature from sensor
    float getTemp(void);

    // Sends one bit to bus
    void writeBit(uint8_t Bit);

    // Reads one bit from bus
    uint8_t readBit(void);

    // Sends one byte to bus
    void write(uint8_t data);

    // Reads one byte from bus
    uint8_t read(void);

    // Sends reset pulse
    uint8_t reset(void);

private:
    PlatformMutex _mutex;
    DigitalInOut _dio;
    uint8_t init = 0;
};

#endif
