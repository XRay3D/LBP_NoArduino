#include "Wire.h"
#include "OneWire.h" //DS18B20
#include "Adafruit_MCP4725.h"
#include "LiquidCrystal_I2C.h"
#include "EEPROM.h"
#include "OneButton.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);

OneButton button1(8, true); // увеличиваем напряжение, ток
OneButton button2(6, true); // уменьшаем напряжение, ток
OneButton button3(7, true); // выбор значения, меню
OneButton button4(9, true); // отключаем выход БП

Adafruit_MCP4725 Udac;
Adafruit_MCP4725 Idac;

OneWire ds(2); // Объект OneWire

byte gradus[8] = // кодируем символ градуса
{
  B01110,
  B01010,
  B01110,
  B00000,
  B00000,
  B00000,
  B00000,
};

#define NAME "LBP"
#define DEVICE "by IVAN-KLUCH"
#define VERSION "1.1 DAC"
#define RelayOut 4                   // реле подаем напряжение на выход
#define Relay1 10                    // переключение обмоток 1-е реле
#define Relay2 11                    // переключение обмоток 2-е реле
#define Relay3 12                    // переключение обмоток 3-е реле
#define fan 5                        // управление вентилятором
#define buzzer 3                     // сигнал спикера
#define RelayOff A3                  // сигнал автоотключения 

int addressU = 0;                    // Начальный адресс ячейки памяти для напряжения
int addressI = 2;                    // Начальный адресс ячейки памяти для тока

int adr_A_Uout = 4;                  // Начальный адресс ячейки памяти для цифр напряжения (переменная float) 1-й блок
int adr_A_Udac = 8;                  // Начальный адресс ячейки памяти для значения ЦАП напряжения (переменная int) 1-й блок
int adr_A_Iout = 10;                 // Начальный адресс ячейки памяти для цифр тока (переменная float) 1-й блок
int adr_A_Idac = 14;                 // Начальный адресс ячейки памяти для значения ЦАП тока (переменная int) 1-й блок

int adr_B_Uout = 16;                 // Начальный адресс ячейки памяти для цифр напряжения (переменная float) 2-й блок
int adr_B_Udac = 20;                 // Начальный адресс ячейки памяти для значения ЦАП напряжения (переменная int) 2-й блок
int adr_B_Iout = 22;                 // Начальный адресс ячейки памяти для цифр тока (переменная float) 2-й блок
int adr_B_Idac = 26;                 // Начальный адресс ячейки памяти для значения ЦАП тока (переменная int) 2-й блок

int adr_C_Uout = 28;                 // Начальный адресс ячейки памяти для цифр напряжения (переменная float) 3-й блок
int adr_C_Udac = 32;                 // Начальный адресс ячейки памяти для значения ЦАП напряжения (переменная int) 3-й блок
int adr_C_Iout = 34;                 // Начальный адресс ячейки памяти для цифр тока (переменная float) 3-й блок
int adr_C_Idac = 38;                 // Начальный адресс ячейки памяти для значения ЦАП тока (переменная int) 3-й блок

int adr_D_Uout = 40;                 // Начальный адресс ячейки памяти для цифр напряжения (переменная float) 4-й блок
int adr_D_Udac = 44;                 // Начальный адресс ячейки памяти для значения ЦАП напряжения (переменная int) 4-й блок
int adr_D_Iout = 46;                 // Начальный адресс ячейки памяти для цифр тока (переменная float) 4-й блок
int adr_D_Idac = 50;                 // Начальный адресс ячейки памяти для значения ЦАП тока (переменная int) 4-й блок

int adr_E_Uout = 52;                 // Начальный адресс ячейки памяти для цифр напряжения (переменная float) 5-й блок
int adr_E_Udac = 56;                 // Начальный адресс ячейки памяти для значения ЦАП напряжения (переменная int) 5-й блок
int adr_E_Iout = 58;                 // Начальный адресс ячейки памяти для цифр тока (переменная float) 5-й блок
int adr_E_Idac = 62;                 // Начальный адресс ячейки памяти для значения ЦАП тока (переменная int) 5-й блок

float Ufix_A = 0.0;                  // переменная для цифр фиксированого напряжения 1-й блок
int Ufix_dac_A = 0;                  // переменая значения ЦАП для напряжения 1-й блок
float Ifix_A = 0.0;                  // переменная для фиксированого тока 1-й блок
int Ifix_dac_A = 0;                  // переменая значения ЦАП для тока 1-й блок

float Ufix_B = 0.0;                  // переменная для цифр фиксированого напряжения 2-й блок
int Ufix_dac_B = 0;                  // переменая значения ЦАП для напряжения 2-й блок
float Ifix_B = 0.0;                  // переменная для фиксированого тока 2-й блок
int Ifix_dac_B = 0;                  // переменая значения ЦАП для тока 2-й блок

float Ufix_C = 0.0;                  // переменная для цифр фиксированого напряжения 3-й блок
int Ufix_dac_C = 0;                  // переменая значения ЦАП для напряжения 3-й блок
float Ifix_C = 0.0;                  // переменная для фиксированого тока 3-й блок
int Ifix_dac_C = 0;                  // переменая значения ЦАП для тока 3-й блок

float Ufix_D = 0.0;                  // переменная для цифр фиксированого напряжения 4-й блок
int Ufix_dac_D = 0;                  // переменая значения ЦАП для напряжения 4-й блок
float Ifix_D = 0.0;                  // переменная для фиксированого тока 4-й блок
int Ifix_dac_D = 0;                  // переменая значения ЦАП для тока 4-й блок

float Ufix_E = 0.0;                  // переменная для цифр фиксированого напряжения 5-й блок
int Ufix_dac_E = 0;                  // переменая значения ЦАП для напряжения 5-й блок
float Ifix_E = 0.0;                  // переменная для фиксированого тока 5-й блок
int Ifix_dac_E = 0;                  // переменая значения ЦАП для тока 5-й блок

float U_Preset = 0;                  // переменная для вывода на дисплей напряжения (при выборе фиксированных значений)
float I_Preset = 0;                  // переменная для вывода на дисплей тока (при выборе фиксированных значений)

int temperature = 0;                 // переменная для хранения значение температуры с датчика DS18B20
long lastUpdateTime = 0;             // Переменная для хранения времени последнего считывания с датчика
const int TEMP_UPDATE_TIME = 1000;   // Определяем периодичность проверок
const int VOLTAGE = A0;
const int CURRENT = A6;
float CoefU = 7.921;                   // коэффициент делителя напряжения CoefU=Uout(max)/5v.опорное
float CoefI = 0.838;                  // коэффициент делителя тока CoefI=Iout/5v опорное
float Uout = 0;
float Iout = 0;
float Ucorr = 0;
float Ucalc = 0;
float U_relay_on = 20.0;             // напряжение включения реле (переключение обмоток)
float U_relay_off = 19.5;            // напряжение отключения реле (переключение обмоток)
float Ref_vol = 5.05;                 // опорное напряжение
//float set_Iout = 0;
int VOLTAGE_DAC = 0;                 // установка напряжения, отсчет в диапазоне 0-4095
int CURRENT_DAC = 0;                 //установка тока, отсчет в диапазоне 0-4095
float CURRENT_SET = 0;               // переменная установленного тока
int value_max = 4052;                // максимальное значение
int value_min = 10;                  // минимальное значение
int value_max_I = 4070;              // максимальное значение (ток)
int value_min_I = 10;                // минимальное значение  (ток)
int set = 0;                         // пункты меню
int menu = 0;                        // меню
int preset_select = 1;               // блоки фикированных напряжений
int set_fix_val = 0;                 // настройка фиксированных значений
int output_control = 0;              // управление выходом БП
unsigned long store_exit_time;       // храним врема выхода из меню
const int exit_time = 4000;          // устанавливаем время выхода из меню
word freq_sw = 2000;                 // часота тона при нажатии кнопок
word freq_alarm = 1500;              // частота тона, сигнал тревоги
word time_sw = 20;                   // длительность тона при нажатии кнопок
word time_alarm = 500;               // длительность тона, сигнал тревоги
byte timer_off;
byte timer_off_set = 30;
//byte var = 0;
unsigned long store_countdown_timer; // храним время автоотключения
const int countdown_timer = 10;       // устанавливаем время автоотключения в минутах
unsigned long store_shutdown_time;   // храним время обратного отсчета
const int shutdown_time = 1000;      // время обратного отсчета 1 сек.
//-------------------------------------------------------------------
void setup() {

  lcd.init();
  lcd.backlight(); // Включаем подсветку дисплея
  lcd.createChar(1, gradus); // Создаем символ под номером 1
  lcd.setCursor(6, 0);
  lcd.print(NAME);
  lcd.setCursor(1, 1);
  lcd.print(DEVICE);
  delay(1500);
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("Version ");
  lcd.setCursor(4, 1);
  lcd.print(VERSION);
  delay(1500);
  lcd.clear();
  //Serial.begin(9600);
  analogReference(EXTERNAL); // опорное напряжение
  // Чтение данных из EEPROM
  EEPROM.get(addressU, VOLTAGE_DAC);
  EEPROM.get(addressI, CURRENT_DAC);

  pinMode(RelayOut, OUTPUT);
  pinMode(Relay1, OUTPUT);
  pinMode(Relay2, OUTPUT);
  pinMode(Relay3, OUTPUT);
  pinMode(fan, OUTPUT);
  pinMode(RelayOff, OUTPUT);
  Udac.begin(0x60);
  Idac.begin(0x61);
  Udac.setVoltage(VOLTAGE_DAC, false); // даем команду цап, устанавливаем напряжение
  Idac.setVoltage(CURRENT_DAC, false); // даем команду цап, устанавливаем ток

  button1.attachClick(click1);
  button1.attachDuringLongPress(longPress1);

  button2.attachClick(click2);
  button2.attachDuringLongPress(longPress2);

  button3.attachClick(click3);
  button3.attachDoubleClick(doubleclick3);
  button3.attachDuringLongPress(longPress3);

  button4.attachClick(click4);
  button4.attachDuringLongPress(longPress4);

  timer_off = timer_off_set; // обратный отсчет устанавливаем в начало
  digitalWrite(RelayOff, 1);
}//конец setup

void loop() {

  if (millis() - store_countdown_timer > countdown_timer * 60000 && Iout == 0 ) { // условия автоотключения
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
  if (millis() - store_exit_time > exit_time && set == 1 && set != 4 ) {
    set = 0;
    lcd.clear();
    EEPROM.put(addressU, VOLTAGE_DAC);// Запись напряжения в память
  }
  //------------------- запись в помять напряжения и тока -----------------
  if (millis() - store_exit_time > exit_time && set == 2 && set != 4 ) {
    set = 0;
    lcd.clear();
    EEPROM.put(addressU, VOLTAGE_DAC);// Запись напряжения в память
    EEPROM.put(addressI, CURRENT_DAC);// Запись тока в память
  }
  //--------------------считаем напряжение и ток--------------------------
  Ucalc = analogRead(VOLTAGE) * (Ref_vol / 1023.0) * CoefU; //узнаем напряжение на выходе  
  Ucorr = (40.0 - Ucalc) * 0.0026; //коррекция напряжения, при желании можно подстроить
  Uout = Ucalc + Ucorr;
  Iout = analogRead(CURRENT) * (Ref_vol  / 1023.0) * CoefI; // узнаем ток в нагрузке  

  CURRENT_SET = Ref_vol / value_max_I * CoefI * CURRENT_DAC; // в этой строке считаем какой ток установлен на выходе 

  //--------------действия при высокой температуре радиатора---------
  if (temperature >= 35) digitalWrite(fan, 1);
  if (temperature <= 31) digitalWrite(fan, 0);
  if (temperature >= 38) {
    digitalWrite(RelayOut, 1);
    output_control = 1;
  }
  
  //---------------переключение обмотк трансфотматора----------------
  if (Uout >= U_relay_on) digitalWrite(Relay1, 1);
  if (Uout <= U_relay_off) digitalWrite(Relay1, 0);

  if (VOLTAGE_DAC >= value_max) VOLTAGE_DAC = value_max;           //не выходим за приделы максимума
  if (VOLTAGE_DAC <= value_min) VOLTAGE_DAC = value_min;           //не выходим за приделы минимума

  if (CURRENT_DAC >= value_max_I) CURRENT_DAC = value_max_I;       //не выходим за приделы максимума
  if (CURRENT_DAC <= value_min_I) CURRENT_DAC = value_min_I;       //не выходим за приделы минимума
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
    lcd.setCursor (0, 1);
    lcd.print ("voltage record");
  }
  if (menu == 1 && set_fix_val == 2) {
    lcd.setCursor (0, 1);
    lcd.print ("current record");
  }
  if (menu == 1) {
    lcd.setCursor (15, 1);
    lcd.print (preset_select);
  }
  if (set == 0 && timer_off == timer_off_set && menu == 0) {
    lcd.setCursor (9, 1);
    lcd.print ("t ");
    lcd.print (temperature);
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
    digitalWrite(RelayOut, 1);
    output_control = 1;
  }
} //конец loop
//-------------функции для кнопки плюс (один клик)--------------------------
void click1() {
  if (menu == 0 && set == 0 || set == 1) {
    set = 1;
    if (VOLTAGE_DAC < value_max) VOLTAGE_DAC = VOLTAGE_DAC + 2; //добавляем
    Udac.setVoltage(VOLTAGE_DAC, false); // даем команду цап, устанавливаем напряжение
  }
  if (set == 2 && menu == 0) {
    if (CURRENT_DAC < value_max) CURRENT_DAC = CURRENT_DAC + 10;
    Idac.setVoltage(CURRENT_DAC, false);
  }
  if (menu == 1 && set_fix_val == 0) { // переключаем блоки фиксированных значений +
    preset_select = preset_select + 1;
    if (preset_select == 6) preset_select = 1;
  }
  if (menu == 1 && set_fix_val == 1) {
    if (VOLTAGE_DAC < value_max) VOLTAGE_DAC = VOLTAGE_DAC + 2; //добавляем
    Udac.setVoltage(VOLTAGE_DAC, false); // даем команду цап, устанавливаем напряжение
  }
  if (menu == 1 && set_fix_val == 2) {
    if (CURRENT_DAC < value_max) CURRENT_DAC = CURRENT_DAC + 10;
    Idac.setVoltage(CURRENT_DAC, false);
  }
  tone(buzzer, freq_sw, time_sw); // сигнал спикера
  store_exit_time = millis();
  cancel_auto_off();
} //конец click1()
//---------------функции для кнопки плюс (непрерывно нажата)----------------
void longPress1() {
  if (menu == 0 && set == 0 || set == 1) {
    set = 1;
    if (VOLTAGE_DAC > 200) {
      VOLTAGE_DAC = VOLTAGE_DAC + 10; //убавляем
    }
    if (VOLTAGE_DAC <= 200) {
      VOLTAGE_DAC = VOLTAGE_DAC + 4; //убавляем
    }
    Udac.setVoltage(VOLTAGE_DAC, false); // даем команду цап, устанавливаем напряжение
  }
  if (menu == 0 && set == 2) {
    if (CURRENT_DAC < value_max) CURRENT_DAC = CURRENT_DAC + 30;
    Idac.setVoltage(CURRENT_DAC, false);
  }
  if (menu == 1 && set_fix_val == 1 && VOLTAGE_DAC > 200) {
    VOLTAGE_DAC = VOLTAGE_DAC + 10; //добавляем
    Udac.setVoltage(VOLTAGE_DAC, false); // даем команду цап, устанавливаем напряжение
  }
  if (menu == 1 && set_fix_val == 1 && VOLTAGE_DAC <= 200) {
    VOLTAGE_DAC = VOLTAGE_DAC + 4; //добавляем
    Udac.setVoltage(VOLTAGE_DAC, false); // даем команду цап, устанавливаем напряжение
  }
  if (menu == 1 && set_fix_val == 2) {
    if (CURRENT_DAC < value_max) CURRENT_DAC = CURRENT_DAC + 30;
    Idac.setVoltage(CURRENT_DAC, false);
  }
  store_exit_time = millis();
  cancel_auto_off();
}//конец longPress1
//---------------функции для кнопки минус (один клик)--------------------
void click2() {
  tone(buzzer, freq_sw, time_sw);
  if (menu == 0 && set == 0 || set == 1) {
    set = 1;
    if (VOLTAGE_DAC > value_min)VOLTAGE_DAC = VOLTAGE_DAC - 2; //убавляем
    store_exit_time = millis();
    Udac.setVoltage(VOLTAGE_DAC, false); // даем команду цап, устанавливаем напряжение
  }
  if (set == 2 && menu == 0) {
    if (CURRENT_DAC > value_min) CURRENT_DAC = CURRENT_DAC - 10; //убавляем
    Idac.setVoltage(CURRENT_DAC, false);
  }
  if (menu == 1 && set_fix_val == 0) { // переключаем блоки фиксированных значений -
    preset_select = preset_select - 1;
    if (preset_select == 0) preset_select = 5;
  }

  if (menu == 1 && set_fix_val == 1) {
    if (VOLTAGE_DAC > value_min) VOLTAGE_DAC = VOLTAGE_DAC - 2; //убавляем
    Udac.setVoltage(VOLTAGE_DAC, false); // даем команду цап, устанавливаем напряжение
    //Ufix_dac_A = VOLTAGE_DAC;
  }
  if (menu == 1 && set_fix_val == 2) {
    if (CURRENT_DAC > value_min) CURRENT_DAC = CURRENT_DAC - 10; //убавляем
    Idac.setVoltage(CURRENT_DAC, false);
  }
  store_exit_time = millis();
  cancel_auto_off();
}//конец click2
//----------------функции для кнопки минус (непрерывно нажата)-------------
void longPress2() {
  if (menu == 0 && set == 0 || set == 1) {
    set = 1;
    if (VOLTAGE_DAC > 200) {
      VOLTAGE_DAC = VOLTAGE_DAC - 10; //убавляем
    }
    if (VOLTAGE_DAC <= 200) {
      VOLTAGE_DAC = VOLTAGE_DAC - 4; //убавляем
    }
    Udac.setVoltage(VOLTAGE_DAC, false); // даем команду цап, устанавливаем напряжение
  }
  if (menu == 0 && set == 2) {
    if (CURRENT_DAC > value_min) CURRENT_DAC = CURRENT_DAC - 30; //убавляем
    Idac.setVoltage(CURRENT_DAC, false);
  }
  if (menu == 1 && set_fix_val == 1 && VOLTAGE_DAC > 200) {
    VOLTAGE_DAC = VOLTAGE_DAC - 10; //убавляем
    Udac.setVoltage(VOLTAGE_DAC, false); // даем команду цап, устанавливаем напряжение
  }
  if (menu == 1 && set_fix_val == 1 && VOLTAGE_DAC <= 200) {
    VOLTAGE_DAC = VOLTAGE_DAC - 4; //убавляем
    Udac.setVoltage(VOLTAGE_DAC, false); // даем команду цап, устанавливаем напряжение
  }
  if (menu == 1 && set_fix_val == 2) {
    if (CURRENT_DAC > value_min) CURRENT_DAC = CURRENT_DAC - 30; //убавляем
    Idac.setVoltage(CURRENT_DAC, false);
  }
  store_exit_time = millis();
  cancel_auto_off();
}// конец longPress2
//-------------------(действия для кнопки меню (один клик)---------------
void click3() {
  tone(buzzer, freq_sw, time_sw); // сигнал спикера
  if (menu == 0) {
    set = set + 1;       //поочередно переключаем режим отображения информации
    if (set == 3) set = 0; //дошли до конца, начинаем снова
  }
  if (menu == 1 && set_fix_val == 0) {
    set_fix_val = 4;
  }
  store_exit_time = millis();
  cancel_auto_off();
  if (set_fix_val == 1 || set_fix_val == 2) {
    set_fix_val ++;
  }
}//конец click3
//-------------------(действия для кнопки меню (двойной клик)-------------
void doubleclick3() {
  menu = menu + 1;
  if (menu == 2) menu = 0; //дошли до конца, начинаем снова
  store_exit_time = millis();
  cancel_auto_off();
  tone(buzzer, freq_sw, time_sw); // сигнал спикера
  lcd.clear();
} // конец doubleclick3
//------------------(действия для кнопки меню (длительно нажата)-----------
void longPress3() {
  if (menu == 1) {
    set_fix_val = 1;
  }
} // конец longPress3
//------------------(действия для кнопки отключение выхода (один клик)-------
void click4() {
  tone(buzzer, freq_sw, time_sw); // сигнал спикера
  output_control = output_control + 1;
  if (output_control == 2) output_control = 0;

  if (output_control == 0) {
    digitalWrite(RelayOut, 0);
  }

  if (output_control == 1) {
    digitalWrite(RelayOut, 1);
  }
  cancel_auto_off();
} // конец click4()
//----------------(действия для кнопки отключение выхода (длительно нажата)------
void longPress4() {
  lcd.clear();
  lcd.setCursor (3, 0);
  lcd.print ("Power OFF");
  delay(500);
  signal_attention();
  delay(1000);
  digitalWrite(RelayOff, 0);
}// конец longPress4
//---------------сигнал спикера-------------
void signal_attention() {
  tone(buzzer, freq_alarm, time_alarm);
}
//------------------функции с фиксированными значениями --------------------------
void functions_for_fixed_values() {

  lcd.setCursor (0, 1);
  lcd.print ("Preset select");
  lcd.setCursor (0, 0);
  if (U_Preset < 10)
    lcd.print(" ");
  lcd.print (U_Preset);
  lcd.print(" V ");
  lcd.setCursor (9, 0);
  lcd.print (I_Preset);
  lcd.print (" A");

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
} // конец functions_for_fixed_values
//------------------запись в память фиксированных значений-------------------------
void record_fixed_values() {
  if (preset_select == 1) {
    EEPROM.put(adr_A_Uout, Uout);            // Запись цифр напряжения в память 1-й блок
    EEPROM.put(adr_A_Udac, VOLTAGE_DAC);     // Запись значения ЦАП напряжения в память 1-й блок
    EEPROM.put(adr_A_Iout, CURRENT_SET);     // Запись цифр тока в память 1-й блок
    EEPROM.put(adr_A_Idac, CURRENT_DAC);     // Запись значения ЦАП тока в память 1-й блок
  }
  if (preset_select == 2) {
    EEPROM.put(adr_B_Uout, Uout);            // Запись цифр напряжения в память 2-й блок
    EEPROM.put(adr_B_Udac, VOLTAGE_DAC);     // Запись значения ЦАП напряжения в память 2-й блок
    EEPROM.put(adr_B_Iout, CURRENT_SET);     // Запись цифр тока в память 2-й блок
    EEPROM.put(adr_B_Idac, CURRENT_DAC);     // Запись значения ЦАП тока в память 2-й блок
  }
  if (preset_select == 3) {
    EEPROM.put(adr_C_Uout, Uout);            // Запись цифр напряжения в память 3-й блок
    EEPROM.put(adr_C_Udac, VOLTAGE_DAC);     // Запись значения ЦАП напряжения в память 3-й блок
    EEPROM.put(adr_C_Iout, CURRENT_SET);     // Запись цифр тока в память 3-й блок
    EEPROM.put(adr_C_Idac, CURRENT_DAC);     // Запись значения ЦАП тока в память 3-й блок
  }
  if (preset_select == 4) {
    EEPROM.put(adr_D_Uout, Uout);            // Запись цифр напряжения в память 4-й блок
    EEPROM.put(adr_D_Udac, VOLTAGE_DAC);     // Запись значения ЦАП напряжения в память 4-й блок
    EEPROM.put(adr_D_Iout, CURRENT_SET);     // Запись цифр тока в память 4-й блок
    EEPROM.put(adr_D_Idac, CURRENT_DAC);     // Запись значения ЦАП тока в память 4-й блок
  }
  if (preset_select == 5) {
    EEPROM.put(adr_E_Uout, Uout);            // Запись цифр напряжения в память 5-й блок
    EEPROM.put(adr_E_Udac, VOLTAGE_DAC);     // Запись значения ЦАП напряжения в память 5-й блок
    EEPROM.put(adr_E_Iout, CURRENT_SET);     // Запись цифр тока в память 5-й блок
    EEPROM.put(adr_E_Idac, CURRENT_DAC);     // Запись значения ЦАП тока в память 5-й блок
  }
} // конец record_fixed_values()
//---------------установка одного из фиксированных значений в активное------------
void select_fixed_value() {
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
  Udac.setVoltage(VOLTAGE_DAC, false); // даем команду цап, устанавливаем напряжение
  Idac.setVoltage(CURRENT_DAC, false); // даем команду цап, устанавливаем ток
  EEPROM.put(addressU, VOLTAGE_DAC);// Запись напряжения в память
  EEPROM.put(addressI, CURRENT_DAC);// Запись тока в память
} // конец select_fixed_value

//-------------------отмена автоотключения---------------------------
void cancel_auto_off() {
  store_countdown_timer = millis(); // сброс таймера автоотключения
  timer_off = timer_off_set; // обратный отсчет устанавливаем в начало
}
//-------------------автоотключение, обратный отсчет-------------------
void avto_off() {
  if (millis() - store_shutdown_time > shutdown_time) {
    timer_off --;
    store_shutdown_time = millis();
    lcd.clear();
  }

  lcd.setCursor (0, 1);
  lcd.print ("Auto OFF");
  lcd.setCursor (9, 1);
  lcd.print (timer_off);
  lcd.setCursor (12, 1);
  lcd.print ("sec");

  if (timer_off == 1) {
    tone(buzzer, freq_alarm, time_alarm); // сигнал спикера
    delay(1000);
    digitalWrite(RelayOff, 0);
  }
}//конец avto_off
//--------------------мерим температуру-------------------------------
int detectTemperature() {

  byte data[2];
  ds.reset();
  ds.write(0xCC);
  ds.write(0x44);

  if (millis() - lastUpdateTime > TEMP_UPDATE_TIME)
  {
    lastUpdateTime = millis();
    ds.reset();
    ds.write(0xCC);
    ds.write(0xBE);
    data[0] = ds.read();
    data[1] = ds.read();

    temperature = (data[1] << 8) + data[0]; temperature = temperature >> 4;
  }
}
