#pragma once
#include <Arduino.h>
class StaticDisplayDummy{
    public:
        StaticDisplayDummy();
        void setupDisplay(void);
        void println(char msg[]);
        void println(String msg);
        void println(char msg[], float value);
        void println(char msg[], String msg2);
    protected:
    private:
};
void StaticDisplayDummy(){

};
void StaticDisplayDummy::setupDisplay(void){

};
void StaticDisplayDummy::println(char msg[]){
    Serial.println(msg);
};
void StaticDisplayDummy::println(String msg){
   Serial.println(msg);
};
void StaticDisplayDummy::println(char msg[], float value){
   char buffer[100];
    char floatValue[6];
    dtostrf(value, 4, 2, floatValue);

    sprintf(
        buffer,
        "%s%s",
        msg, floatValue);
    println(buffer);
};
void StaticDisplayDummy::println(char msg[], String msg2){
    Serial.println(String(msg) + msg2);
};

