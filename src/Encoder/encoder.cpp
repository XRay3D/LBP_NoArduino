#include "encoder.h"

Encoder::Encoder(PinName int_a, PinName int_b, bool speed)
    : m_speed_enabled(speed)
    , pin_a(int_a, PullDown)
    , pin_b(int_b, PullDown)
{

    if (m_speed_enabled)
        EncoderTimer.start();
    pin_a.fall(callback(this, &Encoder::encoderFalling));
    pin_a.rise(callback(this, &Encoder::encoderRising));
    m_position = 0;
    m_speed = 0;
    zero_speed = false;
}

void Encoder::encoderFalling(void)
{
    //temporary speed storage, in case higher interrupt level does stuff
    float temp_speed = 0;
    int motortime_now;
    if (m_speed_enabled) {
        motortime_now = EncoderTimer.read_us();
        EncoderTimer.reset();
        EncoderTimeout.attach(callback(this, &Encoder::timeouthandler), 0.1f);
        /*calculate as ticks per second*/
        if (zero_speed)
            temp_speed = 0;
        else
            temp_speed = 1000000.f / motortime_now;
        zero_speed = false;
    }
    if (pin_b) {
        if (m_position < m_max)
            m_position++;
        m_speed = temp_speed;
    } else {
        if (m_position > m_min)
            m_position--;
        m_speed = -temp_speed; //negative speed
    }
}

void Encoder::encoderRising(void)
{
    //temporary speed storage, in case higher interrupt level does stuff

    float temp_speed = 0;
    int motortime_now;
    if (m_speed_enabled) {
        motortime_now = EncoderTimer.read_us();
        EncoderTimer.reset();
        EncoderTimeout.attach(callback(this, &Encoder::encoderFalling), 0.1f);
        /*calculate as ticks per second*/
        if (zero_speed)
            temp_speed = 0;
        else
            temp_speed = 1000000.f / motortime_now;
        zero_speed = false;
    }
    if (pin_b) {
        if (m_position > m_min)
            m_position--;
        m_speed = -temp_speed; //negative speed
    } else {
        if (m_position < m_max)
            m_position++;
        m_speed = temp_speed;
    }
}

void Encoder::timeouthandler(void)
{
    m_speed = 0;
    zero_speed = true;
}
