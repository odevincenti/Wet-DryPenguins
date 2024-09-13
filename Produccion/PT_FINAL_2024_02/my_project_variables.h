#ifndef MY_PROJECT_VARIABLES_H_
#define MY_PROJECT_VARIABLES_H_

#include <msp430.h>
#include <my_config.h>
#include <my_project_config.h>

extern volatile int sensing;
extern volatile int uart;

extern volatile unsigned int device_is_active;      //that is, user set it to take measurements
extern volatile unsigned int wet_and_dry_is_active;
extern volatile unsigned int temperature_is_active;

extern volatile unsigned int wet_and_dry_period;
extern volatile unsigned int temperature_period;

extern volatile unsigned int calibrated_minute_value;

extern volatile unsigned int remaining_mins_until_wet_and_dry;
extern volatile unsigned int remaining_mins_until_temperature;

#define PENGUIN_NAME_SIZE 17
                        // 37
                        // 31
extern char penguin_name[PENGUIN_NAME_SIZE+1]; // volatile
extern const unsigned int PENGUIN_ID;
extern char starting_date[PENGUIN_NAME_SIZE+1];

#define MAX_INDEX_SIZE  65534//4 //65535 // por la dudas
#define LOG_MEMORY_SIZE 28000//28000//28672//10//28672
extern  unsigned char LOG_MEMORY[LOG_MEMORY_SIZE];

extern unsigned int wet_and_dry_index;
extern unsigned int temperature_index;

//extern volatile unsigned int saved_rtc_remaining;

extern volatile unsigned int memory_full;
extern volatile unsigned int svsh_fault;
extern volatile unsigned int resets_since_last_communication;

enum EVENT_TYPE {RESET, RESET_WHILE_SENSING, RESET_WHILE_UART, FALSE_BASE_DETECTION, FAILED_STARTING_PASSWORD, INACTIVITY, UART_COMMUNICATION, NO_EVENT};
#define MAX_EVENT_LIST  100
extern volatile int looping_events;
extern volatile unsigned int last_events_index;
extern volatile unsigned int last_events[MAX_EVENT_LIST][3];

void new_event(unsigned int event_type);
void clear_events(void);


#define saved_rtc_remaining         BKUP0
#define measurements_pending        BKUP1
#define yellow_state                BKUP2
#define red_state                   BKUP3
#define detecting_base_debouncing   BKUP4
#define base_detected               BKUP5
#define communicating_with_base     BKUP6
#define expecting_starting_pasword  BKUP7
#define password_timeout            BKUP8
#define fsm_state                   BKUP9
#define previous_fsm_state          BKUPa
#define previous_input              BKUPb
#define new_state_just_configured   BKUPc

#define is_calibrating_minute       BKUPd
#define dummy_save_RTCIV_here       BKUPe
#define save_RTCCNT_here            BKUPf


#define CLEAR_RTCIFG                  dummy_save_RTCIV_here = RTCIV

#define SET_RTC_TO(t)   \
    DISABLE_RTC; \
    RTCMOD = (t)-1; \
    RTCCTL = RTCSS__VLOCLK | RTCSR | RTCPS__256 | RTCIE

// Nota: para pensar, el hecho de que el DISABLE_RTC solo deshabilite interrupciones, puede generar problemas?
//       Capaz deberia hacer que RTCSS sea "No clock"
//       OK capaz no, por el RTCSR
// Nota: igual, capaz si, deberia limpiar el RTCIFG en caso de que no este...

#define SET_RTC_TO_MINUTES(m)   SET_RTC_TO(((m)*calibrated_minute_value))

#define SAVE_RTCCNT_AND_DISABLE_RTC \
        saved_rtc_remaining = RTCCNT; \
        DISABLE_RTC; \
        saved_rtc_remaining = RTCMOD + 1 - saved_rtc_remaining;

// in RTCCLK
#define SUBSTRACT_SAVED_RTCCNT(t)           \
        if(saved_rtc_remaining >= (t)){       \
            saved_rtc_remaining-= (t);        \
        }else{                              \
            saved_rtc_remaining = 0;}

#define RETURN_TO_SAVED_RTCCNT \
    RTCMOD = saved_rtc_remaining; \
    RTCCTL = RTCSS__VLOCLK | RTCSR | RTCPS__256 | RTCIE



void set_rtc_to_next_active(void);
void start_rtc_counters(void);

#endif /* MY_PROJECT_VARIABLES_H_ */
