#include <my_config.h>
#include <my_project_config.h>
#include <my_project_variables.h>
#include <uart_functions.h>
#include <adc_functions.h>
#include <logging.h>


unsigned int get_next_pointed_temperature_byte(void){
    unsigned int pointer = (unsigned int) &(LOG_MEMORY[LOG_MEMORY_SIZE - 2]);
    pointer -= 2 * temperature_index;
    return pointer;
}
unsigned int get_next_pointed_wet_and_dry_byte(void){
    unsigned int pointer = (unsigned int) &(LOG_MEMORY[0]);
    pointer += (wet_and_dry_index >> 3);
    return pointer;
}
unsigned int get_next_pointed_wet_and_dry_bit(void){
    return wet_and_dry_index & 0x7;
}

int check_memory_full(void){
    if(wet_and_dry_is_active && temperature_is_active)
        return (get_next_pointed_wet_and_dry_byte() >= get_next_pointed_temperature_byte());
    else if (wet_and_dry_is_active && !temperature_is_active)
        return (get_next_pointed_wet_and_dry_byte() >= ((unsigned int)(&LOG_MEMORY))+LOG_MEMORY_SIZE);
    else if (!wet_and_dry_is_active && temperature_is_active)
        return (get_next_pointed_temperature_byte() < (unsigned int)(&LOG_MEMORY));
    else
        return 0;
}
//int check_index_max(void){
//    if(wet_and_dry_is_active && temperature_is_active)
//        return (wet_and_dry_index >= MAX_INDEX_SIZE) || (temperature_index >= MAX_INDEX_SIZE);
//    else if (wet_and_dry_is_active && !temperature_is_active)
//        return (wet_and_dry_index >= MAX_INDEX_SIZE);
//    else if (!wet_and_dry_is_active && temperature_is_active)
//        return (temperature_index >= MAX_INDEX_SIZE);
//    else
//        return 0;
//}



void wet_and_dry_function(void){
    int base = 0, answer = 0;
    unsigned char temp;
    unsigned int i,b;
    if(!wet_and_dry_is_active)
        return;
    if(wet_and_dry_index >= MAX_INDEX_SIZE)
        return;
    if(check_memory_full())
        memory_full = 1;
    if(memory_full)
        return;
    PIN_OUT_SET(green_pin);

    if(PIN_IFLAG_GET(base_detect_pin))
        base = 1;

    PIN_DIR_IN(rx_pin);
    PIN_REN_DIS(base_detect_pin);
    PIN_OUT_SET(tx_pin);

    if(PIN_IN_GET(rx_pin))
        answer = 1;

    PIN_OUT_CLR(tx_pin);
    PIN_REN_EN(base_detect_pin);
    PIN_DIR_OUT(rx_pin);

    if(!base)
        PIN_IFLAG_CLR(base_detect_pin);

    i = get_next_pointed_wet_and_dry_byte();
    b = get_next_pointed_wet_and_dry_bit();

    temp = (*(unsigned char*)i);
    temp &= ~(1<<b);
    temp |= (answer? (1<<b) : (0<<b));
    UPDATE_PERSISTENT_VAR((*(unsigned char*)i),temp);
    UPDATE_PERSISTENT_VAR(wet_and_dry_index,wet_and_dry_index+1);

    if(check_memory_full())
        memory_full = 1;
    PIN_OUT_CLR(green_pin);
    yellow_state ^= 1;
}
void temperature_function(void){
    unsigned int t,i;
    if(!temperature_is_active)
        return;
    if(temperature_index >= MAX_INDEX_SIZE)
        return;
    if(check_memory_full())
        memory_full = 1;
    if(memory_full)
        return;
    PIN_OUT_SET(green_pin);

    t = take_temperature();
    i = get_next_pointed_temperature_byte();
    UPDATE_PERSISTENT_VAR((*(unsigned int*)i),t);
    UPDATE_PERSISTENT_VAR(temperature_index,temperature_index+1);

    if(check_memory_full())
        memory_full = 1;
    PIN_OUT_CLR(green_pin);
    red_state ^= 1;
}
