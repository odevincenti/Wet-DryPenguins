#ifndef MY_PROJECT_CONFIG_H_
#define MY_PROJECT_CONFIG_H_

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define base_detect_pin     PORT_AND_PIN(4,2)
#define tx_pin              PORT_AND_PIN(1,4)
#define rx_pin              PORT_AND_PIN(1,5)
#define red_pin             PORT_AND_PIN(2,4)
//#define yellow_pin          PORT_AND_PIN(2,5)
//#define xtl_in              PORT_AND_PIN(2,1)
//#define xtl_out             PORT_AND_PIN(2,0)
//#define green_pin           PORT_AND_PIN(3,1)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define MAX_MINUTES         15
#define DEBOUNCING_T        1
#define DEBOUNCING_AMMOUNT  5
// 3 secs aprox
#define PASSWORD_TIME       100
#define INACTIVITY_TIME     300
#define PASSWORD            "1234"


/////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif /* MY_PROJECT_CONFIG_H_ */
