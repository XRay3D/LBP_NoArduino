#include "TextLCD.h"
#include "encoder.h"
#include "mbed.h"

#include "bp.h"
#include "ds18b20.h"

static PwmOut lcdContrPwm(PWM_OUT);

static DigitalOut led(PD_2);

static AnalogIn adc1(PC_0);
static AnalogIn adc2(PC_1);

static AnalogOut dac1(PA_4);
static AnalogOut dac2(PA_5);

static Encoder encDac(PC_2, PC_4);
static Encoder encLcd(PC_5, PC_7);
static BusOut encGnd(PC_3, PC_6, PB_15, PB_13);

static TextLCD lcd(PB_4, PB_5, PB_6, PB_7, PB_8, PB_9, TextLCD::LCD20x2);

DS18B20 ds18b20(PB_14);

static Bp bp;

int main()
{
    printf("\n\n*** RTOS basic example ***\n");

    lcdContrPwm.period_us(100);
    lcdContrPwm.pulsewidth_us(10);

    encGnd = 0b1011;
    encDac.setRange(0x0, 0xFF);
    encLcd.setRange(20, 1000);
    encLcd.setPosition(100);

    float dac = 0.0;

    Thread encThread;
    encThread.start([&] {
        while (true) {
            wait_ms(10);
            dac = (1.0f / 0xFF) * encDac.position();
            lcdContrPwm.period_us(encLcd.position());
        }
    });

    uint16_t d = 0;

    while (true) {
        //float f = cos(++d * (M_PI / 180)) * 0.5 + 0.5;
        dac1.write(dac);
        dac2.write(1.0f - dac);
        led = !led;
        printf("HalaBuda%d\n", d);
        //lcd.cls();
        lcd.locate(0, 0);
        lcd.printf("D1=%1.4f D2=%1.4f\nA1=%1.4f T=%2.3f", dac1.read(), dac2.read(), adc1.read(), ds18b20.getTemp() /*adc2.read()*/);
        wait(0.25);
    }
}
