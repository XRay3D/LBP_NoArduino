/* Copyright (c) 2012, Ivan Kravets <me@ikravets.com>, www.ikravets.com. MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "ds18b20.h"

class Locker {
    PlatformMutex& _mutex;

public:
    Locker(PlatformMutex& mutex)
        : _mutex(mutex)
    {
        _mutex.lock();
    }
    ~Locker()
    {
        _mutex.unlock();
    }
};
DS18B20::DS18B20(PinName pin)
    : _dio(pin)
{
    _dio.mode(PullUp);
    //DS18B20_SetDigitalMode();
    ++init;
}

void DS18B20::writeBit(uint8_t Bit)
{
    _dio.output();
    _dio = 0;
    wait_us(5);
    if (Bit) {
        _dio = 1;
    }
    wait_us(80);
    _dio = 1;
}

uint8_t DS18B20::readBit()
{
    uint8_t Presence = 0;
    _dio.output();
    _dio = 0;
    wait_us(2);
    _dio = 1;
    wait_us(15);
    _dio.input();
    if (_dio == 1) {
        Presence = 1;
    } else {
        Presence = 0;
    }
    return (Presence);
}

void DS18B20::write(uint8_t data)
{
    uint8_t i;
    uint8_t x;
    for (i = 0; i < 8; i++) {
        x = data >> i;
        x &= 0x01;
        writeBit(x);
    }
    wait_us(100);
}

uint8_t DS18B20::read()
{
    uint8_t i;
    uint8_t data = 0;
    for (i = 0; i < 8; i++) {
        if (readBit()) {
            data |= 0x01 << i;
        }
        wait_us(15);
    }
    return (data);
}

uint8_t DS18B20::reset()
{
    uint8_t Presence = 0;
    _dio.output();
    _dio = 0;
    wait_us(500);
    _dio = 1;
    _dio.input();
    wait_us(70);
    if (_dio == 0) {
        ++Presence;
    }
    wait_us(430);
    if (_dio == 1) {
        Presence &= 1;
    }
    return Presence;
}

float DS18B20::getTemp()
{
    Locker locker(_mutex);
    if (init) {
        if (reset()) {
            write(0xCC);
            write(0x44);
            wait_us(470);
            reset();
            write(0xCC);
            write(0xBE);
            uint16_t temp = read();
            temp += read() << 8;
            reset();
            return temp / 16.0;
        } else {
            return -1.0;
        }
    } else {
        return -2.0;
    }
}
