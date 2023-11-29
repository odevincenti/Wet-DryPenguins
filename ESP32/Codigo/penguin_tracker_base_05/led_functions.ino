#include "led_functions.h"
#include "pinout.h"

void change_led_color(int color){
    digitalWrite(PIN_RED_LED,   color & 0x1);
    digitalWrite(PIN_GREEN_LED, (color & 0x2) );
    digitalWrite(PIN_BLUE_LED,  (color & 0x4) );
}
void setup_led(void){
    pinMode(PIN_RED_LED,    OUTPUT);
    pinMode(PIN_GREEN_LED,  OUTPUT);
    pinMode(PIN_BLUE_LED,   OUTPUT);
    digitalWrite(PIN_RED_LED,   LOW);
    digitalWrite(PIN_GREEN_LED, LOW);
    digitalWrite(PIN_BLUE_LED,  LOW);
}