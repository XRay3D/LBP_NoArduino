#include "bp.h"

#include <TextLCD.h>

extern TextLCD lcd;

void tone(PwmOut& buzzer, int freq_alarm, int time_alarm)
{
    buzzer.period(1.0f / freq_alarm);
    buzzer.pulsewidth(0.5f / freq_alarm);
    static Timeout t;
    t.attach([&buzzer] { buzzer.pulsewidth(0); }, time_alarm);
}

static class EEPROM_t {
public:
    EEPROM_t() {}
    void put(int /*addressU*/, int /*VOLTAGE_DAC*/) {}
    void get(int /*addressU*/, int /*VOLTAGE_DAC*/) {}
} EEPROM;

Bp::Bp()
    : button1(PC_15) // увеличиваем напряжение, ток
    , button2(PC_14) // уменьшаем напряжение, ток
    , button3(PC_13) // выбор значения, меню
    , button4(PC_12) // отключаем выход БП
    , Udac(PA_4)
    , Idac(PA_5)
    , ds(PB_14)
    , RelayOut(PA_15) // реле подаем напряжение на выход
    , Relay1(PA_15) // переключение обмоток 1-е реле
    , Relay2(PA_15) // переключение обмоток 2-е реле
    , Relay3(PA_15) // переключение обмоток 3-е реле
    , fan(PA_15) // управление вентилятором
    , buzzer(PA_15) // сигнал спикера
    , RelayOff(PA_15) // сигнал автоотключенияds
    , VOLTAGE(A0)
    , CURRENT(A5)
{
    setup();
    Thread* thread = new Thread;
    thread->start(callback(this, &Bp::detectTemperature));
}

void Bp::setup()
{
    //    lcd.init();
    //    lcd.backlight(); // Включаем подсветку дисплея
    //    lcd.createChar(1, gradus); // Создаем символ под номером 1
    //    lcd.setCursor(6, 0);
    //    lcd.print(NAME);
    //    lcd.setCursor(1, 1);
    //    lcd.print(DEVICE);
    wait_ms(1500);
    //    lcd.clear();
    //    lcd.setCursor(4, 0);
    //    lcd.print("Version ");
    //    lcd.setCursor(4, 1);
    //    lcd.print(VERSION);
    wait_ms(1500);
    //    lcd.clear();
    //Serial.begin(9600);
    //    analogReference(EXTERNAL); // опорное напряжение
    // Чтение данных из EEPROM
    //    EEPROM.get(addressU, VOLTAGE_DAC);
    //    EEPROM.get(addressI, CURRENT_DAC);

    //    pinMode(RelayOut, OUTPUT);
    //    pinMode(Relay1, OUTPUT);
    //    pinMode(Relay2, OUTPUT);
    //    pinMode(Relay3, OUTPUT);
    //    pinMode(fan, OUTPUT);
    //    pinMode(RelayOff, OUTPUT);
    //    Udac.begin(0x60);
    //    Idac.begin(0x61);
    Udac.write(VOLTAGE_DAC); // даем команду цап, устанавливаем напряжение
    Idac.write(CURRENT_DAC); // даем команду цап, устанавливаем ток

    button1.rise([=] { click1(); timeout1.attach(callback(this, &Bp::longPress1), 1.0); });
    button1.fall([=] { timeout1.detach(); });

    button2.rise([=] { click2(); timeout2.attach(callback(this, &Bp::longPress2), 1.0); });
    button2.fall([=] { timeout2.detach(); });

    button3.rise([=] { click3(); timeout3.attach(callback(this, &Bp::longPress3), 1.0); });
    button3.fall([=] { timeout3.detach(); });

    button4.rise([=] { click4(); timeout1.attach(callback(this, &Bp::longPress4), 1.0); });
    button4.fall([=] { timeout4.detach(); });

    timer_off = timer_off_set; // обратный отсчет устанавливаем в начало
    RelayOff = 1;
}

void Bp::loop()
{

    if (millis() - store_countdown_timer > countdown_timer * 60000 && Iout == 0) { // условия автоотключения
        avto_off();
    }
    if (Iout > 0) { // если ток более нуля сбрасываем таймер автооткл.
        cancel_auto_off();
    }

    // функция опроса кнопок
    button1.tick();
    button2.tick();
    button3.tick();
    button4.tick();

    detectTemperature(); // Определяем температуру от датчика DS18b20

    //------------------- запись в память напряжения ------------------------
    if (millis() - store_exit_time > exit_time && set == 1 && set != 4) {
        set = 0;
        lcd.clear();
        EEPROM.put(addressU, VOLTAGE_DAC); // Запись напряжения в память
    }
    //------------------- запись в помять напряжения и тока -----------------
    if (millis() - store_exit_time > exit_time && set == 2 && set != 4) {
        set = 0;
        lcd.clear();
        EEPROM.put(addressU, VOLTAGE_DAC); // Запись напряжения в память
        EEPROM.put(addressI, CURRENT_DAC); // Запись тока в память
    }
    //--------------------считаем напряжение и ток--------------------------
    Ucalc = analogRead(VOLTAGE) * (Ref_vol / 1023.0) * CoefU; //узнаем напряжение на выходе
    Ucorr = (40.0 - Ucalc) * 0.0026; //коррекция напряжения, при желании можно подстроить
    Uout = Ucalc + Ucorr;
    Iout = analogRead(CURRENT) * (Ref_vol / 1023.0) * CoefI; // узнаем ток в нагрузке

    CURRENT_SET = Ref_vol / value_max_I * CoefI * CURRENT_DAC; // в этой строке считаем какой ток установлен на выходе

    //--------------действия при высокой температуре радиатора---------
    if (temperature >= 35)
        fan = 1;
    if (temperature <= 31)
        fan = 0;
    if (temperature >= 38) {
        RelayOut = 1;
        output_control = 1;
    }

    //---------------переключение обмотк трансфотматора----------------
    if (Uout >= U_relay_on)
        Relay1 = 1;
    if (Uout <= U_relay_off)
        Relay1 = 0;

    if (VOLTAGE_DAC >= value_max)
        VOLTAGE_DAC = value_max; //не выходим за приделы максимума
    if (VOLTAGE_DAC <= value_min)
        VOLTAGE_DAC = value_min; //не выходим за приделы минимума

    if (CURRENT_DAC >= value_max_I)
        CURRENT_DAC = value_max_I; //не выходим за приделы максимума
    if (CURRENT_DAC <= value_min_I)
        CURRENT_DAC = value_min_I; //не выходим за приделы минимума
    //----------------выводим информацию на дисплей--------------------
    if (menu == 0) {
        lcd.setCursor(0, 0);
        if (Uout < 10)
            lcd.print(" ");
        lcd.print(Uout, 2);
        lcd.print(" V");

        lcd.setCursor(9, 0);
        lcd.print(Iout, 2);
        lcd.print(" A");
    }
    if (menu == 1 && set_fix_val == 1 || set_fix_val == 2) {
        lcd.setCursor(0, 0);
        if (Uout < 10)
            lcd.print(" ");
        lcd.print(Uout, 2);
        lcd.print(" V");
        lcd.setCursor(9, 0);
        lcd.print(CURRENT_SET, 2);
        lcd.print(" A ");
    }
    if (menu == 1 && set_fix_val == 1) {
        lcd.setCursor(0, 1);
        lcd.print("voltage record");
    }
    if (menu == 1 && set_fix_val == 2) {
        lcd.setCursor(0, 1);
        lcd.print("current record");
    }
    if (menu == 1) {
        lcd.setCursor(15, 1);
        lcd.print(preset_select);
    }
    if (set == 0 && timer_off == timer_off_set && menu == 0) {
        lcd.setCursor(9, 1);
        lcd.print("t ");
        lcd.print(temperature);
        lcd.print("\1C ");
    }
    if (set == 0 && output_control == 0 && menu == 0) {
        lcd.setCursor(0, 1);
        if (Uout * Iout < 10)
            lcd.print(" ");
        lcd.print(Uout * Iout, 2);
        lcd.print(" W ");
    }
    if (menu == 0 && set == 0 && output_control == 1) {
        lcd.setCursor(0, 1);
        lcd.print("Out OFF");
    }
    if (set == 1 && menu == 0) {
        lcd.setCursor(0, 1);
        lcd.print("VOLTAGE");
    }
    if (set == 2 && menu == 0) {
        lcd.setCursor(0, 1);
        lcd.print("CURRENT");
        lcd.setCursor(9, 1);
        lcd.print(CURRENT_SET, 2);
        lcd.print(" A ");
    }
    if (menu == 1 && set_fix_val == 0) {
        functions_for_fixed_values();
    }
    if (menu == 1 && set_fix_val == 3) {
        for (int i = 0; i <= 9; i++) {
            Serial.println(i);
            tone(buzzer, freq_alarm, time_alarm);
            record_fixed_values();
        }
        set_fix_val = 0;
    }
    if (menu == 1 && set_fix_val == 4) {
        select_fixed_value();
        set_fix_val = 0;
        menu = 0;
        lcd.clear();
    }
    if (menu == 1) {
        RelayOut = 1;
        output_control = 1;
    }
}

void Bp::click1()
{
    if ((menu == 0 && set == 0) || set == 1) {
        set = 1;
        if (VOLTAGE_DAC < value_max)
            VOLTAGE_DAC = VOLTAGE_DAC + 2; //добавляем
        Udac.write(VOLTAGE_DAC); // даем команду цап, устанавливаем напряжение
    }
    if (set == 2 && menu == 0) {
        if (CURRENT_DAC < value_max)
            CURRENT_DAC = CURRENT_DAC + 10;
        Idac.write(CURRENT_DAC);
    }
    if (menu == 1 && set_fix_val == 0) { // переключаем блоки фиксированных значений +
        preset_select = preset_select + 1;
        if (preset_select == 6)
            preset_select = 1;
    }
    if (menu == 1 && set_fix_val == 1) {
        if (VOLTAGE_DAC < value_max)
            VOLTAGE_DAC = VOLTAGE_DAC + 2; //добавляем
        Udac.write(VOLTAGE_DAC); // даем команду цап, устанавливаем напряжение
    }
    if (menu == 1 && set_fix_val == 2) {
        if (CURRENT_DAC < value_max)
            CURRENT_DAC = CURRENT_DAC + 10;
        Idac.write(CURRENT_DAC);
    }
    tone(buzzer, freq_sw, time_sw); // сигнал спикера
    store_exit_time = millis();
    cancel_auto_off();
}

void Bp::longPress1()
{
    if (menu == 0 && set == 0 || set == 1) {
        set = 1;
        if (VOLTAGE_DAC > 200) {
            VOLTAGE_DAC = VOLTAGE_DAC + 10; //убавляем
        }
        if (VOLTAGE_DAC <= 200) {
            VOLTAGE_DAC = VOLTAGE_DAC + 4; //убавляем
        }
        Udac.write(VOLTAGE_DAC); // даем команду цап, устанавливаем напряжение
    }
    if (menu == 0 && set == 2) {
        if (CURRENT_DAC < value_max)
            CURRENT_DAC = CURRENT_DAC + 30;
        Idac.write(CURRENT_DAC);
    }
    if (menu == 1 && set_fix_val == 1 && VOLTAGE_DAC > 200) {
        VOLTAGE_DAC = VOLTAGE_DAC + 10; //добавляем
        Udac.write(VOLTAGE_DAC); // даем команду цап, устанавливаем напряжение
    }
    if (menu == 1 && set_fix_val == 1 && VOLTAGE_DAC <= 200) {
        VOLTAGE_DAC = VOLTAGE_DAC + 4; //добавляем
        Udac.write(VOLTAGE_DAC); // даем команду цап, устанавливаем напряжение
    }
    if (menu == 1 && set_fix_val == 2) {
        if (CURRENT_DAC < value_max)
            CURRENT_DAC = CURRENT_DAC + 30;
        Idac.write(CURRENT_DAC);
    }
    store_exit_time = millis();
    cancel_auto_off();
}

void Bp::click2()
{
    tone(buzzer, freq_sw, time_sw);
    if (menu == 0 && set == 0 || set == 1) {
        set = 1;
        if (VOLTAGE_DAC > value_min)
            VOLTAGE_DAC = VOLTAGE_DAC - 2; //убавляем
        store_exit_time = millis();
        Udac.write(VOLTAGE_DAC); // даем команду цап, устанавливаем напряжение
    }
    if (set == 2 && menu == 0) {
        if (CURRENT_DAC > value_min)
            CURRENT_DAC = CURRENT_DAC - 10; //убавляем
        Idac.write(CURRENT_DAC);
    }
    if (menu == 1 && set_fix_val == 0) { // переключаем блоки фиксированных значений -
        preset_select = preset_select - 1;
        if (preset_select == 0)
            preset_select = 5;
    }

    if (menu == 1 && set_fix_val == 1) {
        if (VOLTAGE_DAC > value_min)
            VOLTAGE_DAC = VOLTAGE_DAC - 2; //убавляем
        Udac.write(VOLTAGE_DAC); // даем команду цап, устанавливаем напряжение
        //Ufix_dac_A = VOLTAGE_DAC;
    }
    if (menu == 1 && set_fix_val == 2) {
        if (CURRENT_DAC > value_min)
            CURRENT_DAC = CURRENT_DAC - 10; //убавляем
        Idac.write(CURRENT_DAC);
    }
    store_exit_time = millis();
    cancel_auto_off();
}

void Bp::longPress2()
{
    if (menu == 0 && set == 0 || set == 1) {
        set = 1;
        if (VOLTAGE_DAC > 200) {
            VOLTAGE_DAC = VOLTAGE_DAC - 10; //убавляем
        }
        if (VOLTAGE_DAC <= 200) {
            VOLTAGE_DAC = VOLTAGE_DAC - 4; //убавляем
        }
        Udac.write(VOLTAGE_DAC); // даем команду цап, устанавливаем напряжение
    }
    if (menu == 0 && set == 2) {
        if (CURRENT_DAC > value_min)
            CURRENT_DAC = CURRENT_DAC - 30; //убавляем
        Idac.write(CURRENT_DAC);
    }
    if (menu == 1 && set_fix_val == 1 && VOLTAGE_DAC > 200) {
        VOLTAGE_DAC = VOLTAGE_DAC - 10; //убавляем
        Udac.write(VOLTAGE_DAC); // даем команду цап, устанавливаем напряжение
    }
    if (menu == 1 && set_fix_val == 1 && VOLTAGE_DAC <= 200) {
        VOLTAGE_DAC = VOLTAGE_DAC - 4; //убавляем
        Udac.write(VOLTAGE_DAC); // даем команду цап, устанавливаем напряжение
    }
    if (menu == 1 && set_fix_val == 2) {
        if (CURRENT_DAC > value_min)
            CURRENT_DAC = CURRENT_DAC - 30; //убавляем
        Idac.write(CURRENT_DAC);
    }
    store_exit_time = millis();
    cancel_auto_off();
}

void Bp::click3()
{
    tone(buzzer, freq_sw, time_sw); // сигнал спикера
    if (menu == 0) {
        set = set + 1; //поочередно переключаем режим отображения информации
        if (set == 3)
            set = 0; //дошли до конца, начинаем снова
    }
    if (menu == 1 && set_fix_val == 0) {
        set_fix_val = 4;
    }
    store_exit_time = millis();
    cancel_auto_off();
    if (set_fix_val == 1 || set_fix_val == 2) {
        set_fix_val++;
    }
}

void Bp::doubleclick3()
{
    menu = menu + 1;
    if (menu == 2)
        menu = 0; //дошли до конца, начинаем снова
    store_exit_time = millis();
    cancel_auto_off();
    tone(buzzer, freq_sw, time_sw); // сигнал спикера
    lcd.clear();
}

void Bp::longPress3()
{
    if (menu == 1) {
        set_fix_val = 1;
    }
}

void Bp::click4()
{
    tone(buzzer, freq_sw, time_sw); // сигнал спикера
    output_control = output_control + 1;
    if (output_control == 2)
        output_control = 0;

    if (output_control == 0) {
        RelayOut = 0;
    }

    if (output_control == 1) {
        RelayOut = 1;
    }
    cancel_auto_off();
}

void Bp::longPress4()
{
    lcd.clear();
    lcd.setCursor(3, 0);
    lcd.print("Power OFF");
    wait_ms(500);
    signal_attention();
    wait_ms(1000);
    RelayOff = 0;
}

void Bp::signal_attention()
{
    tone(buzzer, freq_alarm, time_alarm);
}

void Bp::functions_for_fixed_values()
{

    lcd.setCursor(0, 1);
    lcd.print("Preset select");
    lcd.setCursor(0, 0);
    if (U_Preset < 10)
        lcd.print(" ");
    lcd.print(U_Preset);
    lcd.print(" V ");
    lcd.setCursor(9, 0);
    lcd.print(I_Preset);
    lcd.print(" A");

    if (preset_select == 1) {
        EEPROM.get(adr_A_Uout, Ufix_A);
        EEPROM.get(adr_A_Udac, Ufix_dac_A);
        EEPROM.get(adr_A_Iout, Ifix_A);
        EEPROM.get(adr_A_Idac, Ifix_dac_A);
        U_Preset = Ufix_A;
        I_Preset = Ifix_A;
    }
    if (preset_select == 2) {
        EEPROM.get(adr_B_Uout, Ufix_B);
        EEPROM.get(adr_B_Udac, Ufix_dac_B);
        EEPROM.get(adr_B_Iout, Ifix_B);
        EEPROM.get(adr_B_Idac, Ifix_dac_B);
        U_Preset = Ufix_B;
        I_Preset = Ifix_B;
    }
    if (preset_select == 3) {
        EEPROM.get(adr_C_Uout, Ufix_C);
        EEPROM.get(adr_C_Udac, Ufix_dac_C);
        EEPROM.get(adr_C_Iout, Ifix_C);
        EEPROM.get(adr_C_Idac, Ifix_dac_C);
        U_Preset = Ufix_C;
        I_Preset = Ifix_C;
    }
    if (preset_select == 4) {
        EEPROM.get(adr_D_Uout, Ufix_D);
        EEPROM.get(adr_D_Udac, Ufix_dac_D);
        EEPROM.get(adr_D_Iout, Ifix_D);
        EEPROM.get(adr_D_Idac, Ifix_dac_D);
        U_Preset = Ufix_D;
        I_Preset = Ifix_D;
    }
    if (preset_select == 5) {
        EEPROM.get(adr_E_Uout, Ufix_E);
        EEPROM.get(adr_E_Udac, Ufix_dac_E);
        EEPROM.get(adr_E_Iout, Ifix_E);
        EEPROM.get(adr_E_Idac, Ifix_dac_E);
        U_Preset = Ufix_E;
        I_Preset = Ifix_E;
    }
}

void Bp::record_fixed_values()
{
    if (preset_select == 1) {
        EEPROM.put(adr_A_Uout, Uout); // Запись цифр напряжения в память 1-й блок
        EEPROM.put(adr_A_Udac, VOLTAGE_DAC); // Запись значения ЦАП напряжения в память 1-й блок
        EEPROM.put(adr_A_Iout, CURRENT_SET); // Запись цифр тока в память 1-й блок
        EEPROM.put(adr_A_Idac, CURRENT_DAC); // Запись значения ЦАП тока в память 1-й блок
    }
    if (preset_select == 2) {
        EEPROM.put(adr_B_Uout, Uout); // Запись цифр напряжения в память 2-й блок
        EEPROM.put(adr_B_Udac, VOLTAGE_DAC); // Запись значения ЦАП напряжения в память 2-й блок
        EEPROM.put(adr_B_Iout, CURRENT_SET); // Запись цифр тока в память 2-й блок
        EEPROM.put(adr_B_Idac, CURRENT_DAC); // Запись значения ЦАП тока в память 2-й блок
    }
    if (preset_select == 3) {
        EEPROM.put(adr_C_Uout, Uout); // Запись цифр напряжения в память 3-й блок
        EEPROM.put(adr_C_Udac, VOLTAGE_DAC); // Запись значения ЦАП напряжения в память 3-й блок
        EEPROM.put(adr_C_Iout, CURRENT_SET); // Запись цифр тока в память 3-й блок
        EEPROM.put(adr_C_Idac, CURRENT_DAC); // Запись значения ЦАП тока в память 3-й блок
    }
    if (preset_select == 4) {
        EEPROM.put(adr_D_Uout, Uout); // Запись цифр напряжения в память 4-й блок
        EEPROM.put(adr_D_Udac, VOLTAGE_DAC); // Запись значения ЦАП напряжения в память 4-й блок
        EEPROM.put(adr_D_Iout, CURRENT_SET); // Запись цифр тока в память 4-й блок
        EEPROM.put(adr_D_Idac, CURRENT_DAC); // Запись значения ЦАП тока в память 4-й блок
    }
    if (preset_select == 5) {
        EEPROM.put(adr_E_Uout, Uout); // Запись цифр напряжения в память 5-й блок
        EEPROM.put(adr_E_Udac, VOLTAGE_DAC); // Запись значения ЦАП напряжения в память 5-й блок
        EEPROM.put(adr_E_Iout, CURRENT_SET); // Запись цифр тока в память 5-й блок
        EEPROM.put(adr_E_Idac, CURRENT_DAC); // Запись значения ЦАП тока в память 5-й блок
    }
}

void Bp::select_fixed_value()
{
    if (preset_select == 1) {
        VOLTAGE_DAC = Ufix_dac_A;
        CURRENT_DAC = Ifix_dac_A;
    }
    if (preset_select == 2) {
        VOLTAGE_DAC = Ufix_dac_B;
        CURRENT_DAC = Ifix_dac_B;
    }
    if (preset_select == 3) {
        VOLTAGE_DAC = Ufix_dac_C;
        CURRENT_DAC = Ifix_dac_C;
    }
    if (preset_select == 4) {
        VOLTAGE_DAC = Ufix_dac_D;
        CURRENT_DAC = Ifix_dac_D;
    }
    if (preset_select == 5) {
        VOLTAGE_DAC = Ufix_dac_E;
        CURRENT_DAC = Ifix_dac_E;
    }
    Udac.write(VOLTAGE_DAC); // даем команду цап, устанавливаем напряжение
    Idac.write(CURRENT_DAC); // даем команду цап, устанавливаем ток
    EEPROM.put(addressU, VOLTAGE_DAC); // Запись напряжения в память
    EEPROM.put(addressI, CURRENT_DAC); // Запись тока в память
}

void Bp::cancel_auto_off()
{
    store_countdown_timer = millis(); // сброс таймера автоотключения
    timer_off = timer_off_set; // обратный отсчет устанавливаем в начало
}

void Bp::avto_off()
{
    if (millis() - store_shutdown_time > shutdown_time) {
        timer_off--;
        store_shutdown_time = millis();
        lcd.clear();
    }

    lcd.setCursor(0, 1);
    lcd.print("Auto OFF");
    lcd.setCursor(9, 1);
    lcd.print(timer_off);
    lcd.setCursor(12, 1);
    lcd.print("sec");

    if (timer_off == 1) {
        tone(buzzer, freq_alarm, time_alarm); // сигнал спикера
        wait_ms(1000);
        RelayOff = 0;
    }
}

void Bp::detectTemperature()
{
    while (true) {
        uint8_t data[2];
        ds.reset();
        ds.write(0xCC);
        ds.write(0x44);

        //        if (millis() - lastUpdateTime > TEMP_UPDATE_TIME) {
        //            lastUpdateTime = millis();
        ds.reset();
        ds.write(0xCC);
        ds.write(0xBE);
        data[0] = ds.read();
        data[1] = ds.read();

        temperature = (data[1] << 8) + data[0];
        temperature = temperature >> 4;
        //        }

        wait(TEMP_UPDATE_TIME);
    }
}
