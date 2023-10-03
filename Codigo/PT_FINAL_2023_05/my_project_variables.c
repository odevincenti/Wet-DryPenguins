#include <my_project_variables.h>

//#define PERSISTENT_UINT(var,starting_value)     _Pragma("PERSISTENT("#var")"); volatile unsigned int var = starting_value
//
//PERSTINTENT_UINT(calibrated_minute_value,2344);
//PERSTINTENT_UINT(remaining_mins_until_wet_and_dry,0);
//PERSTINTENT_UINT(remaining_mins_until_temperature,0);

#pragma PERSISTENT(sensing)
volatile int sensing = 0;
#pragma PERSISTENT(uart)
volatile int uart = 0;

#pragma PERSISTENT(device_is_active);
#pragma PERSISTENT(wet_and_dry_is_active);
#pragma PERSISTENT(temperature_is_active);
#pragma PERSISTENT(wet_and_dry_period);
#pragma PERSISTENT(temperature_period);
#pragma PERSISTENT(calibrated_minute_value);////_Pragma("PERSISTENT(calibrated_minute_value)"); volatile unsigned int calibrated_minute_value = 2344;
#pragma PERSISTENT(remaining_mins_until_wet_and_dry)
#pragma PERSISTENT(remaining_mins_until_temperature)

volatile unsigned int device_is_active = 0;
volatile unsigned int wet_and_dry_is_active = 0;
volatile unsigned int temperature_is_active = 0;
volatile unsigned int wet_and_dry_period = 2; // dont make this 0
volatile unsigned int temperature_period = 3; // dont make this 0

volatile unsigned int calibrated_minute_value = 30;//30;//2344;
volatile unsigned int remaining_mins_until_wet_and_dry = 2;
volatile unsigned int remaining_mins_until_temperature = 3;

#pragma PERSISTENT(memory_full)
#pragma PERSISTENT(svsh_fault)
#pragma PERSISTENT(resets_since_last_communication)
volatile unsigned int memory_full = 0;
volatile unsigned int svsh_fault = 0;
volatile unsigned int resets_since_last_communication = 0;

#pragma PERSISTENT(looping_events)
volatile int looping_events = 0;
#pragma PERSISTENT(last_events_index)
volatile unsigned int last_events_index = 0;
#pragma PERSISTENT(last_events)
extern volatile unsigned int last_events [MAX_EVENT_LIST][3] = {[0 ... MAX_EVENT_LIST-1] = {0,0,NO_EVENT}};

void clear_events(void){
    UPDATE_PERSISTENT_VAR(last_events_index,0);
    UPDATE_PERSISTENT_VAR(looping_events,0);
}

void new_event(unsigned int event_type){
    UPDATE_PERSISTENT_VAR(last_events[last_events_index][0],wet_and_dry_index);
    UPDATE_PERSISTENT_VAR(last_events[last_events_index][1],temperature_index);
    UPDATE_PERSISTENT_VAR(last_events[last_events_index][2],event_type);

    if(last_events_index >= MAX_EVENT_LIST-1){
        UPDATE_PERSISTENT_VAR(last_events_index,0);
        UPDATE_PERSISTENT_VAR(looping_events,1);
    }else{
        UPDATE_PERSISTENT_VAR(last_events_index,last_events_index+1);
    }
}

#pragma PERSISTENT(penguin_name)
char penguin_name[PENGUIN_NAME_SIZE+1] = "Pingu"; //volatile
const unsigned int PENGUIN_ID = 1234;
char starting_date[PENGUIN_NAME_SIZE+1] = "23-jun-1991 12:00:00";

#pragma PERSISTENT(LOG_MEMORY)
unsigned char LOG_MEMORY[LOG_MEMORY_SIZE] = {[0 ... LOG_MEMORY_SIZE-1] = 0};

#pragma PERSISTENT(temperature_index)
#pragma PERSISTENT(wet_and_dry_index)
unsigned int temperature_index = 0;
unsigned int wet_and_dry_index = 0;

void start_rtc_counters(void){
    new_state_just_configured = 0;

    UPDATE_PERSISTENT_VAR(remaining_mins_until_wet_and_dry, wet_and_dry_period);
    UPDATE_PERSISTENT_VAR(remaining_mins_until_temperature, temperature_period);
    UPDATE_PERSISTENT_VAR(temperature_index, 0);
    UPDATE_PERSISTENT_VAR(wet_and_dry_index, 0);

    UPDATE_PERSISTENT_VAR(memory_full, 0);
    UPDATE_PERSISTENT_VAR(svsh_fault, 0);
    UPDATE_PERSISTENT_VAR(resets_since_last_communication, 0);

    UPDATE_PERSISTENT_VAR(last_events_index, 0);

    set_rtc_to_next_active();
}

void set_rtc_to_next_active(void){
    //int m = 15;
    unsigned int min = MAX_MINUTES;
    DISABLE_RTC;
    if(device_is_active && (wet_and_dry_is_active || temperature_is_active)){
        if(remaining_mins_until_wet_and_dry == 0){
            UPDATE_PERSISTENT_VAR(remaining_mins_until_wet_and_dry, wet_and_dry_period);
        }
        if(remaining_mins_until_temperature == 0){
            UPDATE_PERSISTENT_VAR(remaining_mins_until_temperature, temperature_period);
        }

        if (wet_and_dry_is_active && temperature_is_active){
            min = remaining_mins_until_wet_and_dry < remaining_mins_until_temperature ? remaining_mins_until_wet_and_dry : remaining_mins_until_temperature;
            if (min > MAX_MINUTES) min = MAX_MINUTES;
            UPDATE_PERSISTENT_VAR(remaining_mins_until_wet_and_dry, remaining_mins_until_wet_and_dry - min);
            UPDATE_PERSISTENT_VAR(remaining_mins_until_temperature, remaining_mins_until_temperature - min);
        }else if(wet_and_dry_is_active){
            min = remaining_mins_until_wet_and_dry;
            if (min > MAX_MINUTES) min = MAX_MINUTES;
            UPDATE_PERSISTENT_VAR(remaining_mins_until_wet_and_dry, remaining_mins_until_wet_and_dry - min);
        }else if (temperature_is_active){
            min = remaining_mins_until_temperature;
            if (min > MAX_MINUTES) min = MAX_MINUTES;
            UPDATE_PERSISTENT_VAR(remaining_mins_until_temperature, remaining_mins_until_temperature - min);
        }

        SET_RTC_TO_MINUTES(min);
    }

}
