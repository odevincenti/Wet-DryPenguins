#ifndef MY_PROJECT_CONFIG_H_
#define MY_PROJECT_CONFIG_H_

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define base_detect_pin     PORT_AND_PIN(4,2)
#define tx_pin              PORT_AND_PIN(1,4)
#define rx_pin              PORT_AND_PIN(1,5)
#define red_pin             PORT_AND_PIN(2,4)
#define yellow_pin          PORT_AND_PIN(2,5)
#define xtl_in              PORT_AND_PIN(2,1)
#define xtl_out             PORT_AND_PIN(2,0)
#define green_pin           PORT_AND_PIN(3,1)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define MAX_MINUTES         15
#define DEBOUNCING_T        1
#define DEBOUNCING_AMMOUNT  5
// 3 secs aprox
#define PASSWORD_TIME       100
#define INACTIVITY_TIME     300
#define PASSWORD            "1234"


#define EXTRA_SIGNALS_FOR_DEBUG 0

#if EXTRA_SIGNALS_FOR_DEBUG
#define pin0                PORT_AND_PIN(1,6)
#define pin1                PORT_AND_PIN(1,7)
#define pin2                PORT_AND_PIN(1,0)
#define pin3                PORT_AND_PIN(1,1)
#define pin4                PORT_AND_PIN(1,2)
#define pin5                PORT_AND_PIN(1,3)
#define pin6                PORT_AND_PIN(2,2)
#define pin7                PORT_AND_PIN(3,0)
#define pinAM               PORT_AND_PIN(3,3)
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif /* MY_PROJECT_CONFIG_H_ */
