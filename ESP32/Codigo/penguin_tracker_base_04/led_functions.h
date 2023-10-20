#ifndef __led_functions_h__
#define __led_functions_h__

enum COLOR {BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE};
void setup_led(void);
void change_led_color(int color);

#define BASE_POWER_ON RED
#define CONNECTING    YELLOW
#define CONNECTED      GREEN
#define PROCESSING    BLUE

#endif