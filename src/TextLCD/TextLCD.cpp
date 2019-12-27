/* mbed TextLCD Library, for a 4-bit LCD based on HD44780
 * Copyright (c) 2007-2010, sford, http://mbed.org
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "TextLCD.h"
#include "mbed.h"

TextLCD::TextLCD(PinName rs, PinName e, PinName d4, PinName d5, PinName d6, PinName d7, LCDType type)
    : _rs(rs)
    , _e(e)
    , _d(d4, d5, d6, d7)
    , _type(type)
{
    _e = 1;
    _rs = 0; // command mode

    wait_ms(15); // Wait 15ms to ensure powered up

    // send "Display Settings" 3 times (Only top nibble of 0x30 as we've got 4-bit bus)
    for (int i = 0; i < 3; i++) {
        writeByte(0x3);
        wait_ms(2); // this command takes 1.64ms, so wait for it
    }
    writeByte(0x2); // 4-bit mode
    wait_us(40); // most instructions take 40us

    writeCommand(0x28); // Function set 001 BW N F - -
    writeCommand(0x0C);
    writeCommand(0x6); // Cursor Direction and Display Shift : 0000 01 CD S (CD 0-left, 1-right S(hift) 0-no, 1-yes
    cls();
    //lcd_init();
}

void TextLCD::character(int column, int row, int c)
{
    int a = address(column, row);
    writeCommand(a);
    writeData(c);
}

void TextLCD::cls()
{
    //lcd_clear();
    writeCommand(0x01); // cls, and set cursor to 0
    wait_ms(2); // This command takes 1.64 ms
    locate(0, 0);
}

void TextLCD::locate(int column, int row)
{
    _column = column;
    _row = row;
}

int TextLCD::_putc(int value)
{
    if (value == '\n') {
        _column = 0;
        _row++;
        if (_row >= rows()) {
            _row = 0;
        }
    } else {
        character(_column, _row, value);
        //lcd_goto(static_cast<uint8_t>(_row), static_cast<uint8_t>(_column));
        //lcd_dat(static_cast<uint8_t>(value));
        _column++;
        if (_column >= columns()) {
            _column = 0;
            _row++;
            if (_row >= rows()) {
                _row = 0;
            }
        }
    }
    return value;
}

int TextLCD::_getc()
{
    return -1;
}

void TextLCD::writeByte(int value)
{
    _d = value >> 4;
    wait_us(40); // most instructions take 40us
    _e = 0;
    wait_us(40);
    _e = 1;
    _d = value >> 0;
    wait_us(40);
    _e = 0;
    wait_us(40); // most instructions take 40us
    _e = 1;
}

void TextLCD::writeCommand(int command)
{
    _rs = 0;
    writeByte(command);
}

void TextLCD::writeData(int data)
{
    _rs = 1;
    writeByte(data);
}
//////////////////////////
//void TextLCD::lcd_init() //init display
//{

//    //RW_SetLow();
//    _rs = 0;
//    _e = 0;
//    _d = 0;

//    wait_ms(20); //delay on power up

//    lcd_bus(0b0011);
//    wait_ms(5);

//    lcd_bus(0b0011);
//    wait_ms(1);

//    lcd_bus(0b0011);
//    wait_ms(1);
//    lcd_bus(0b0010);

//    lcd_cmd5(0b00101100);

//    lcd_cmd5(0b00001100); //Display ON/OFF Control

//    lcd_cmd5(0b00000001); //Display Clear

//    lcd_cmd5(0b00000110); //Entry Mode Set

//    //    lcd_cmd5(0b00000010); //enable 4-bit mode
//    //    lcd_cmd5(0b00001000); //4-bit mode, 2-line, 5x8 font
//    //    lcd_cmd5(0b00001000); //display off
//    //
//    //    lcd_cmd5(DISPLAY_CLR); //lcd_clear();
//    //
//    //    lcd_cmd(0b00000110); //entry mode set
//    //
//    lcd_cmd(0b00001100 | CURSOR_OFF | BLINK_OFF);

//    lcd_custom(custchar);

//    lcd_goto(0, 0);
//}

//void TextLCD::lcd_bus(uint8_t data) //write four bit
//{
//    //if (data & 0b00000001) DB0_SetHigh(); else DB0_SetLow(); // DB4
//    //if (data & 0b00000010) DB1_SetHigh(); else DB1_SetLow(); // DB5
//    //if (data & 0b00000100) DB2_SetHigh(); else DB2_SetLow(); // DB6
//    //if (data & 0b00001000) DB3_SetHigh(); else DB3_SetLow(); // DB7
//    _d = data;
//    _e = 1;
//    wait_us(50);
//    _e = 0;
//}

//void TextLCD::lcd_wrt(uint8_t data) //write data
//{
//    lcd_bus(data >> 4);
//    lcd_bus(data);
//    wait_us(50);
//}

//void TextLCD::lcd_dat(uint8_t data) //write data to the display RAM
//{
//    _rs = 1;
//    lcd_wrt(data);
//}

//void TextLCD::lcd_cmd(uint8_t data) //write a command
//{
//    _rs = 0;
//    lcd_wrt(data);
//}

//void TextLCD::lcd_cmd5(uint8_t data) //wait 5 ms after write
//{
//    lcd_cmd(data);
//    wait_ms(5);
//}

//void TextLCD::lcd_clear() //clear the screen
//{
//    lcd_cmd5(DISPLAY_CLR);
//}

//void TextLCD::lcd_goto(uint8_t line, uint8_t column) //line 0..1, column 0..39
//{
//    lcd_cmd(DDRAM_WRITE | (line ? 0b01000000 : 0b00000000) | column);
//}

//void TextLCD::lcd_print(const char* str) //print a string
//{
//    while (*str)
//        lcd_dat(*str++);
//}

//void TextLCD::lcd_custom(const uint8_t* arr) //loading an array of custom characters
//{
//    lcd_cmd(CGRAM_WRITE);
//    for (uint8_t k = 0; k < 64; k++)
//        lcd_dat(arr[k]);
//}

//void TextLCD::lcd_right() //shift right
//{
//    lcd_cmd(SCR_RIGHT);
//}

//void TextLCD::lcd_left() //shift left
//{
//    lcd_cmd(SCR_LEFT);
//}

//void TextLCD::lcd_cursor_on() //cursor on, blink on
//{
//    lcd_cmd(0b00001100 | CURSOR_ON | BLINK_ON);
//}

//void TextLCD::lcd_cursor_off() //cursor off, blink off
//{
//    lcd_cmd(0b00001100 | CURSOR_OFF | BLINK_OFF);
//}

int TextLCD::address(int column, int row)
{
    switch (_type) {
    case LCD20x4:
        switch (row) {
        case 0:
            return 0x80 + column;
        case 1:
            return 0xc0 + column;
        case 2:
            return 0x94 + column;
        case 3:
            return 0xd4 + column;
        }
    case LCD16x2B:
        return 0x80 + (row * 40) + column;
    case LCD16x2:
    case LCD20x2:
    default:
        return 0x80 + (row * 0x40) + column;
    }
}

int TextLCD::columns()
{
    switch (_type) {
    case LCD20x4:
    case LCD20x2:
        return 20;
    case LCD16x2:
    case LCD16x2B:
    default:
        return 16;
    }
}

int TextLCD::rows()
{
    switch (_type) {
    case LCD20x4:
        return 4;
    case LCD16x2:
    case LCD16x2B:
    case LCD20x2:
    default:
        return 2;
    }
}
