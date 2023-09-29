#include "StaticDisplay.h"
bool StaticDisplay::isDisplayVisible = true;
char StaticDisplay::oldText[MaxLineCount][MaxCharCount] = {0};
char StaticDisplay::currentText[MaxLineCount][MaxCharCount] = {0};
uint8_t StaticDisplay::cursor = 0;
uint16_t StaticDisplay::Display_Backround_Color = Display_Color_White;
uint16_t StaticDisplay::Display_Text_Color = Display_Color_Black;
Adafruit_ST7735 StaticDisplay::tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

StaticDisplay::StaticDisplay(){
  
}
void StaticDisplay::setupDisplay(void){
    
    tft.initR(INITR_BLACKTAB); // Init ST7735R chip, green tab
    tft.setRotation(1);
    tft.setFont();
    tft.fillScreen(Display_Backround_Color);
    tft.setTextColor(Display_Text_Color);
    tft.setTextSize(1);
    isDisplayVisible = true;
    tft.enableDisplay(isDisplayVisible);
};
void StaticDisplay::println(char msg[]){
    if(cursor >= MaxLineCount){//Shift up
  for(int i = 1; i < MaxLineCount; i++){
    strcpy(currentText[i-1],currentText[i]);
  }
    sprintf(
        currentText[MaxLineCount-1],
        "%s",
        msg);
  }else{
    sprintf(
        currentText[cursor],
        "%s",
        msg);
        cursor++;
  }

  for(int i = 0; i < MaxLineCount; i++){
      if (strcmp(currentText[i],oldText[i]) != 0) {

        // yes! home the cursor
        tft.setCursor(0,8*i);

        // change the text color to the background color
        tft.setTextColor(Display_Backround_Color);

        // redraw the old value to erase
        tft.print(oldText[i]);

        // home the cursor
        tft.setCursor(0,8*i);
        
        // change the text color to foreground color
        tft.setTextColor(Display_Text_Color);
    
        // draw the new time value
        tft.print(currentText[i]);
    
        // and remember the new value
        strcpy(oldText[i],currentText[i]);
      }
    }
};
void StaticDisplay::println(String msg){
    char buffer[MaxCharCount];
    sprintf(
        buffer,
        "%s",
        msg);
    println(buffer);
};
void StaticDisplay::println(char msg[], float value){
    char buffer[MaxCharCount];
    char floatValue[6];
    dtostrf(value, 4, 2, floatValue);

    sprintf(
        buffer,
        "%s%s",
        msg, floatValue);
    println(buffer);
};
void StaticDisplay::println(char msg[], String msg2){
    char buffer[MaxCharCount];
    sprintf(
        buffer,
        "%s%s",
        msg, msg2);
    println(buffer);
};