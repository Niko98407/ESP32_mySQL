#pragma once
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
const size_t MaxLineCount = 16;
const size_t MaxCharCount = 30;
const uint16_t  Display_Color_Black        = 0x0000;
const uint16_t  Display_Color_Blue         = 0x001F;
const uint16_t  Display_Color_Red          = 0xF800;
const uint16_t  Display_Color_Green        = 0x07E0;
const uint16_t  Display_Color_Cyan         = 0x07FF;
const uint16_t  Display_Color_Magenta      = 0xF81F;
const uint16_t  Display_Color_Yellow       = 0xFFE0;
const uint16_t  Display_Color_White        = 0xFFFF;



#define SerialDebugging true
#define TFT_CS 14       // Chip Select
#define TFT_RST 26      // Reset
#define TFT_DC 27      // data/ctrl
#define TFT_MOSI 25     // Data out
#define TFT_SCLK 33     // Clock out



class StaticDisplay{
    public:
        StaticDisplay();
        void setupDisplay(void);
        void println(char msg[]);
        void println(String msg);
        void println(char msg[], float value);
        void println(char msg[], String msg2);
        static bool isDisplayVisible;
    protected:
    private:
        static char oldText[MaxLineCount][MaxCharCount];
        static char currentText[MaxLineCount][MaxCharCount];
        static uint8_t cursor;
        static Adafruit_ST7735 tft; 
        static uint16_t Display_Text_Color;
        static uint16_t Display_Backround_Color;
};


