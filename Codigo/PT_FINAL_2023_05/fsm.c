#include <my_config.h>
#include <my_project_config.h>
#include <my_project_variables.h>
#include <uart_functions.h>
#include <adc_functions.h>
#include <logging.h>
#include <fsm.h>


const char bye_message[] = "\nBye bye!\n";

#define SETTING_NUMBER_ARRAY_SIZE 8
unsigned char setting_number_array[SETTING_NUMBER_ARRAY_SIZE] = {
      [0 ... SETTING_NUMBER_ARRAY_SIZE-1] = 0
};

void clear_setting_number_array(void){
    int i;
    for(i = 0; i <SETTING_NUMBER_ARRAY_SIZE; i++){
        setting_number_array[i] = 0;
    }
}
void push_setting_number_array(unsigned char n){
    int i;
    for(i = SETTING_NUMBER_ARRAY_SIZE - 1; i>0; i--){
        setting_number_array[i] = setting_number_array[i-1];
    }
    setting_number_array[0] = n <= 9? n: 0;
}
void rx_setting_number_array(void){
    int i;
    tx_string("\nInput = ");
    for(i = SETTING_NUMBER_ARRAY_SIZE - 1; i >= 0; i--){
        tx_int(setting_number_array[i]);
    }
    tx_string("\n\n");
}

unsigned int setting_number_array_to_int(void){
    unsigned int answer = 0;
    int i;
    for(i = SETTING_NUMBER_ARRAY_SIZE-1; i>=0; i--){
        answer *= 10;
        answer += setting_number_array[i];
    }
    return answer;
}

char setting_name[PENGUIN_NAME_SIZE+1] = {[0 ... PENGUIN_NAME_SIZE] = '\0'};
unsigned int setting_name_index = 0;
void clear_setting_name(void){
    int i;
    for (i = 0; i < PENGUIN_NAME_SIZE+1; i++){
        setting_name[i] = '\0';
    }
    setting_name_index = 0;
}
void input_to_setting_name(unsigned char input){
    if(setting_name_index < PENGUIN_NAME_SIZE){
        setting_name[setting_name_index] = input;
        setting_name_index++;
    }
}
void backspace_setting_name(void){
    if(setting_name_index > 0){
        setting_name_index--;
        setting_name[setting_name_index] = '\0';
    }
}
void rx_setting_name(void){
    tx_string("\nInput = ");
    tx_string(setting_name);
    tx_string("\n\n");
}

void setting_name_to_starting_date(void){
    unsigned int i;
    WRITE_FRAM_ENABLE;
    for(i = 0; i < PENGUIN_NAME_SIZE; i++){
        starting_date[i] = setting_name[i];
    }
    starting_date[PENGUIN_NAME_SIZE] = '\0';
    WRITE_FRAM_DISABLE;
}

void setting_name_to_penguin_name(void){
    unsigned int i;
    WRITE_FRAM_ENABLE;
    for(i = 0; i < PENGUIN_NAME_SIZE; i++){
        penguin_name[i] = setting_name[i];
    }
    penguin_name[PENGUIN_NAME_SIZE] = '\0';
    WRITE_FRAM_DISABLE;
}

void rx_user_menu_data(void){
    //unsigned int i;
    tx_string("\nPROGRAMMED STATE = ");
    if(!device_is_active && !wet_and_dry_is_active && !temperature_is_active){
        tx_string("INACTIVE\n");
    }else if(device_is_active && wet_and_dry_is_active && !temperature_is_active){
        tx_string("WET AND DRY ONLY\n");
    }else if(device_is_active && !wet_and_dry_is_active && temperature_is_active){
        tx_string("TEMPERATURE ONLY\n");
    }else if(device_is_active && wet_and_dry_is_active && temperature_is_active){
        tx_string("WET AND DRY & TEMPERATURE\n");
    }else{
        tx_string("INVALID STATE\n");
    }

    SEND_BOOL(memory_full);
    SEND_BOOL(svsh_fault);
    SEND_INT(resets_since_last_communication);
    SEND_INT(temperature_index);
    SEND_INT(wet_and_dry_index);
    SEND_BOOL(temperature_index == MAX_INDEX_SIZE);
    SEND_BOOL(wet_and_dry_index == MAX_INDEX_SIZE);
    SEND_INT(get_next_pointed_temperature_byte());
    SEND_INT(get_next_pointed_wet_and_dry_byte());
    SEND_INT(get_next_pointed_wet_and_dry_bit());

}

const char new_state_password[] = "Noot noot!";
unsigned int new_state_password_index = 0;
int incorrect_password = 0;

void init_entering_password(void){
    incorrect_password = 0;
    new_state_password_index = 0;
}

void rx_entering_password(void){
    int i;
    tx_string("\nINPUT = ");
    for(i = 0; i<new_state_password_index; i++){
        tx_string("*");
    }
    tx_string("\n");
}

enum FSM_STATE {
    INIT_STATE,
    GET_STATE,
    SET_STATE,
    SET_NEXT_STATE,
    NAME_STATE,
    USER_STATE,
    ENTERING_PASSWORD_STATE,
    SELECT_NEW_STATE,
    DATE_STATE,

    INVALID_STATE
};

void change_state(unsigned int new_state, int repeating);

int init_state_function(unsigned char input,    int repeating);
int get_state_function(unsigned char input,     int repeating);
int set_state_function(unsigned char input,     int repeating);
int set_next_state_function(unsigned char input,     int repeating);
int name_state_function(unsigned char input,     int repeating);
int user_state_function(unsigned char input,     int repeating);
int entering_password_function(unsigned char input,     int repeating);
int select_new_function(unsigned char input,     int repeating);
int date_state_function(unsigned char input,     int repeating);


int invalid_state_function(unsigned char input, int repeating);

int (*const(FSM_ARRAY[]))(unsigned char, int) = {
    [INIT_STATE]     = init_state_function,
    [GET_STATE]      = get_state_function,
    [SET_STATE]      = set_state_function,
    [SET_NEXT_STATE] = set_next_state_function,
    [NAME_STATE]     = name_state_function,
    [USER_STATE]     = user_state_function,
    [ENTERING_PASSWORD_STATE] = entering_password_function,
    [SELECT_NEW_STATE] = select_new_function,
    [DATE_STATE]     = date_state_function,

    [INVALID_STATE]  = invalid_state_function
};

const char * const CHANGE_STATE_MESSAGES[] = {
    [INIT_STATE]     = "\nDEBUG MENU\n\n"\
                        "G = Get\nS = Set\n"\
                        "N = Change Penguin Name\n"\
                        "L = Toggle LED\n"\
                        "> = USER MENU\n"\
                        "? = Repeat\nQ = Quit",// (continue as if no communication occurred)",

    [GET_STATE]      = "\nGETTING...\n\n"\
                        "1 = Device active\n"\
                        "2 = Wet and Dry active\n"\
                        "3 = Temperature active\n"\
                        "4 = Wet and Dry Period\n"\
                        "5 = Temperature Period\n"\
                        "6 = Calibrated Minute Value\n"\
                        "7 = Penguin Name\n"\
                        "8 = Penguin ID\n"\
                        "9 = Starting Date\n"\
                        "0 = Calibration values\n" \
                        "? = Repeat\n< = Return",
    [SET_STATE]      = "\nSETTING...\n\n"\
                        "0 - 9 = Input number\n" \
                        "> = Next; choose variable to set\n"\
                        "? = Repeat\n< = Return",
    [SET_NEXT_STATE] = "\nSETTING...\n\n"\
                        "1 = Device active           - Do not modify\n"\
                        "2 = Wet and Dry active      - Do not modify\n"\
                        "3 = Temperature active      - Do not modify\n"\
                        "4 = Wet and Dry Period      - Configure before changing state\n"\
                        "5 = Temperature Period      - Configure before changing state\n"\
                        "6 = Calibrated Minute Value - Configure once before release\n"\
                        "? = Repeat\n< = Return",

                        // - Configure before changing state
    [NAME_STATE]     = "\nCHANGING NAME...\n\n"\
                        "Any letter or number = Write\n"\
                        "- = Delete\n"\
                        "> = Confirm, change name\n"\
                        "? = Repeat\n< = Return",
    [USER_STATE]     = "\nUSER MENU\n\n"\
                        "Attention! Before starting new state, change periods in DEBUG MENU\n"\
                        "G = Get tracking data\n"\
                        "F = Get tracking data fast\n"\
                        "E = Events\n"\
                        "< = Go back to DEBUG MENU\n"\
                        "> = Set new state (either start tracking or turn off)\n"\
                        "? = Repeat\nQ = Quit",// (continue as if no communication occurred)",
    [ENTERING_PASSWORD_STATE] = "\nENTER PASSWORD\n\n"\
                                "WARNING! Changing state starts new tracking logs, deleting all previous data\n"\
                                "< = Abort password input",
    [SELECT_NEW_STATE] = "\nSELECT NEW STATE AND QUIT\n\n"\
                         "REMEMBER! After choosing an option, all previous data is lost\n"\
                         "1 = INACTIVE\n"\
                         "2 = WET AND DRY ONLY\n"\
                         "3 = TEMPERATURE ONLY\n"\
                         "4 = WET AND DRY & TEMPERATURE\n"\
                         "? = Repeat\n< = Return",

    [DATE_STATE]     = "\nINSERT DATE\n\n"\
                    "Any letter or number = Write\n"\
                    "- = Delete\n"\
                    "> = Confirm starting date\n"\
                    "? = Repeat\n< = Return",

    [INVALID_STATE]  = "\nWarning! Entering invalid state"
};

const char * const EVENT_MESSAGES[]={
 [RESET] = "Reset",
 [RESET_WHILE_SENSING] = "Reset while sensing",
 [RESET_WHILE_UART] = "Reset while in UART",
 [FALSE_BASE_DETECTION] = "False base detect",
 [FAILED_STARTING_PASSWORD] = "Timeout while Password",
 [INACTIVITY]   = "Disconnected for Inactivity",
 [UART_COMMUNICATION] = "UART communication"
};


void change_state(unsigned int new_state, int repeating){
    unsigned int i;
    if(!repeating){
        previous_fsm_state = fsm_state;
        fsm_state = new_state;
    }
    tx_string(CHANGE_STATE_MESSAGES[new_state]);
    tx_string("\n\n");
    for(i = 0; i < 25; i++){
        tx_string("*");
    }
    tx_string("\n");
}

void fsm_init(void){
    fsm_state = INIT_STATE;
    previous_input = '\0';
    change_state(INIT_STATE, 0);
}

int fsm_function(unsigned char input){
    int quit = 0;
    if(input != '?'){

        if (fsm_state < LENGTH(FSM_ARRAY)){
            quit = FSM_ARRAY[fsm_state](input, 0);
        }else{
            quit = FSM_ARRAY[INVALID_STATE](input, 0);
        }
        previous_input = input;
    }else{
        //SEND_ACK;
        tx_string("\nRepeating ...");
        if (previous_fsm_state < LENGTH(FSM_ARRAY)){
            quit = FSM_ARRAY[previous_fsm_state](previous_input, 1);
        }else{
            quit = FSM_ARRAY[INVALID_STATE](previous_input, 1);
        }
    }
    return quit;
}

int init_state_function(unsigned char input, int repeating){
    int quit = 0;
    switch(input){
    case 'Q': case 'q':
        SEND_ACK;
        tx_string(bye_message);
        SEND_NUL;
        quit = 1;
        break;
    case 'G': case 'g':
        SEND_ACK;
        change_state(GET_STATE, repeating);
        SEND_NUL;
        break;
    case 'S': case 's':
        SEND_ACK;
        if(!repeating){
            clear_setting_number_array();
        }
        rx_setting_number_array();
        change_state(SET_STATE, repeating);
        SEND_NUL;
        break;
    case 'N': case 'n':
        SEND_ACK;
        if(!repeating){
            clear_setting_name();
        }
        rx_setting_name();
        change_state(NAME_STATE, repeating);
        SEND_NUL;
        break;
    case '>':
        SEND_ACK;
        rx_user_menu_data();
        change_state(USER_STATE, repeating);
        SEND_NUL;
        break;

    case 'L': case 'l':
        SEND_ACK;
        if(!repeating){
            PIN_OUT_TOGGLE(red_pin);
        }
        if(PIN_IN_GET(red_pin)){
            tx_string("\nLED ON\n");
        }else{
            tx_string("\nLED OFF\n");
        }
        change_state(INIT_STATE, repeating);
        SEND_NUL;
        break;
    default:
        SEND_NAK;
        tx_string("\nUnknown Command");
        change_state(INIT_STATE, repeating);
        SEND_NUL;
        break;
    }
    return quit;
}

#define GET_BOOL_AND_GO_BACK(var) \
        SEND_ACK;\
        SEND_BOOL(var);\
        change_state(INIT_STATE, repeating);\
        SEND_NUL

#define GET_INT_AND_GO_BACK(var) \
        SEND_ACK;\
        SEND_INT(var);\
        change_state(INIT_STATE, repeating);\
        SEND_NUL

#define GET_STRING_AND_GO_BACK(var)\
        SEND_ACK;\
        SEND_STRING(var);\
        change_state(INIT_STATE, repeating);\
        SEND_NUL

int get_state_function(unsigned char input, int repeating){
    int quit = 0;
    switch(input){
    case '<':
        SEND_ACK;
        change_state(INIT_STATE, repeating);
        SEND_NUL;
        break;
    case '1':
        GET_BOOL_AND_GO_BACK(device_is_active);
        break;
    case '2':
        GET_BOOL_AND_GO_BACK(wet_and_dry_is_active);
        break;
    case '3':
        GET_BOOL_AND_GO_BACK(temperature_is_active);
        break;
    case '4':
        GET_INT_AND_GO_BACK(wet_and_dry_period);
        break;
    case '5':
        GET_INT_AND_GO_BACK(temperature_period);
        break;
    case '6':
        GET_INT_AND_GO_BACK(calibrated_minute_value);
        break;
    case '7':
        GET_STRING_AND_GO_BACK(penguin_name);
        break;
    case '8':
        GET_INT_AND_GO_BACK(PENGUIN_ID);
        break;
    case '9':
        GET_STRING_AND_GO_BACK(starting_date);
        break;
    case '0':
        SEND_ACK;
        SEND_INT(CALADC_15V_30C);
        SEND_INT(CALADC_15V_105C);
        SEND_INT(ADC_GAIN);
        SEND_INT(ADC_OFFSET);
        change_state(INIT_STATE, repeating);
        SEND_NUL;
        break;
    default:
        SEND_NAK;
        tx_string("\nUnknown Command");
        change_state(GET_STATE, repeating);
        SEND_NUL;
        break;
    }
    return quit;
}
int set_state_function(unsigned char input, int repeating){
    int quit = 0;
    switch(input){
    case '<':
        SEND_ACK;
        change_state(INIT_STATE, repeating);
        SEND_NUL;
        break;
    case '>':
        SEND_ACK;
        change_state(SET_NEXT_STATE, repeating);
        SEND_NUL;
        break;

    default:
        if(input <= '9' && input >= '0'){
            SEND_NAK;
            if(!repeating){
                push_setting_number_array(input - '0');
            }
            rx_setting_number_array();
            change_state(SET_STATE, repeating);
            SEND_NUL;
            break;
        }

        SEND_NAK;
        tx_string("\nUnknown Command");
        change_state(SET_STATE, repeating);
        SEND_NUL;
        break;
    }
    return quit;
}

void set_int_and_go_back(volatile unsigned int *var, const unsigned int min, const unsigned int max, int repeating){
    unsigned int value = 0;
    SEND_ACK;
    if(!repeating){
        value = setting_number_array_to_int();
        value = (value > max) ? max : value;
        value = (value < min) ? min : value;
        WRITE_FRAM_ENABLE;
        *var = value;
        WRITE_FRAM_DISABLE;
    }
    change_state(INIT_STATE, repeating);
    SEND_NUL;
}

//#define SET_INT_AND_GO_BACK(var, max) \
//        SEND_ACK;\
//        value = setting_number_array_to_int();\
//        value = (value > max) ? max : value;\
//        if(!repeating){\
//            WRITE_FRAM_ENABLE;\
//            var = value;\
//            WRITE_FRAM_DISABLE;\
//        }\
//        change_state(INIT_STATE, repeating);\
//        SEND_NUL

int set_next_state_function(unsigned char input,  int repeating){
    int quit = 0;
    //unsigned int value = 0;
    switch(input){
    case '<':
        SEND_ACK;
        rx_setting_number_array();
        change_state(SET_STATE, repeating);
        SEND_NUL;
        break;
    case '1':
        set_int_and_go_back(&device_is_active,0,1,repeating);
        break;
    case '2':
        set_int_and_go_back(&wet_and_dry_is_active,0,1,repeating);
        break;
    case '3':
        set_int_and_go_back(&temperature_is_active,0,1,repeating);
        break;
    case '4':
        set_int_and_go_back(&wet_and_dry_period,1,60,repeating);
        break;
    case '5':
        set_int_and_go_back(&temperature_period,1,60,repeating);
        break;
    case '6':
        set_int_and_go_back(&calibrated_minute_value,1,4000,repeating);
        break;
    default:
        SEND_NAK;
        tx_string("\nUnknown Command");
        change_state(SET_NEXT_STATE, repeating);
        SEND_NUL;
        break;
    }
    return quit;
}

int invalid_state_function(unsigned char input, int repeating){
    SEND_NAK;
    change_state(INIT_STATE, repeating);
    SEND_NUL;
    return 0;
}

int name_state_function(unsigned char input,     int repeating){
    int quit = 0;
    switch(input){
    case '<':
        SEND_ACK;
        change_state(INIT_STATE, repeating);
        SEND_NUL;
        break;
    case '>':
        SEND_ACK;
        if(!repeating){
            setting_name_to_penguin_name();
        }
        tx_string("\nName changed to ");
        tx_string(penguin_name);
        tx_string("\n");
        change_state(INIT_STATE, repeating);
        SEND_NUL;
        break;
    case '-':
        SEND_ACK;
        if(!repeating){
            backspace_setting_name();
        }
        rx_setting_name();
        change_state(NAME_STATE, repeating);
        SEND_NUL;
        break;
    default:
        if((input >= 'a' && input <= 'z') || (input >= 'A' && input <= 'Z') ||\
                input == ' ' || (input >= '0' && input <= '9')){
            SEND_ACK;
            if(!repeating){
                input_to_setting_name(input);
            }
            rx_setting_name();
            change_state(NAME_STATE, repeating);
            SEND_NUL;
            break;
        }

        SEND_NAK;
        tx_string("\nUnknown Command");
        change_state(NAME_STATE, repeating);
        SEND_NUL;
        break;
    }
    return quit;
}

int date_state_function(unsigned char input, int repeating){
    int quit = 0;
    switch(input){
    case '<':
        SEND_ACK;
        rx_user_menu_data();
        change_state(USER_STATE, repeating);
        SEND_NUL;
        break;
    case '>':
        SEND_ACK;
        if(!repeating){
            setting_name_to_starting_date();
        }
        tx_string("\nStarting date = ");
        tx_string(starting_date);
        tx_string("\n");
        change_state(SELECT_NEW_STATE, repeating);
        SEND_NUL;
        break;
    case '-':
        SEND_ACK;
        if(!repeating){
            backspace_setting_name();
        }
        rx_setting_name();
        change_state(DATE_STATE, repeating);
        SEND_NUL;
        break;
    default:
        if((input >= 'a' && input <= 'z') || (input >= 'A' && input <= 'Z') ||\
                input == ' ' || (input >= '0' && input <= '9')){
            SEND_ACK;
            if(!repeating){
                input_to_setting_name(input);
            }
            rx_setting_name();
            change_state(DATE_STATE, repeating);
            SEND_NUL;
            break;
        }

        SEND_NAK;
        tx_string("\nUnknown Command");
        change_state(DATE_STATE, repeating);
        SEND_NUL;
        break;
    }
    return quit;
}

int user_state_function(unsigned char input, int repeating){
    int quit = 0;
    unsigned int i;
    switch(input){
    case '<':
        SEND_ACK;
        change_state(INIT_STATE, repeating);
        SEND_NUL;
        break;
    case 'Q': case 'q':
        SEND_ACK;
        tx_string(bye_message);
        SEND_NUL;
        quit = 1;
        break;
    case '>':
        SEND_ACK;
        if(!repeating){
            init_entering_password();
        }
        rx_entering_password();
        change_state(ENTERING_PASSWORD_STATE, repeating);
        SEND_NUL;
        break;
    case 'G': case 'g':
        SEND_ACK;
        tx_string("\n");
        for(i = 0; i<LOG_MEMORY_SIZE;i++){
            tx_string("@ ");
            tx_int(((unsigned int) (&LOG_MEMORY))+i);
            tx_string("\tIndex ");
            tx_int(i);
            tx_string("/");
            tx_int(LOG_MEMORY_SIZE-1);
            tx_string("\t=\t");
            tx_int((unsigned int) LOG_MEMORY[i]);
            tx_string("\n");
        }
        tx_string("\n");
        rx_user_menu_data();
        change_state(USER_STATE, repeating);
        SEND_NUL;
        break;
    case 'F': case 'f':
        SEND_ACK;
        tx_string("\n");
        for(i = 0; i<LOG_MEMORY_SIZE;i++){
            if((i & 0x7) == 0)
                tx_string("\n");
            tx_int((unsigned int) LOG_MEMORY[i]);
            tx_string("\t");

        }
        tx_string("\n");
        rx_user_menu_data();
        change_state(USER_STATE, repeating);
        SEND_NUL;
        break;

    case 'E': case 'e':
        SEND_ACK;
        SEND_INT(last_events_index);
        SEND_BOOL(looping_events);
        for(i = 0; i< MAX_EVENT_LIST; i++){
            tx_int(last_events[i][0]);
            tx_string("\t");
            tx_int(last_events[i][1]);
            tx_string("\t");
            tx_string(((last_events[i][2] < NO_EVENT) ? EVENT_MESSAGES[last_events[i][2]] : "Event not valid"));
            tx_string("\n");
        }
//        for(i = 0; i<LOG_MEMORY_SIZE;i++){
//            if((i & 0x7) == 0)
//                tx_string("\n");
//            tx_int((unsigned int) LOG_MEMORY[i]);
//            tx_string("\t");
//
//        }
        tx_string("\n");
        rx_user_menu_data();
        change_state(USER_STATE, repeating);
        SEND_NUL;
        break;
    default:
        SEND_NAK;
        tx_string("\nUnknown Command");
        rx_user_menu_data();
        change_state(USER_STATE, repeating);
        SEND_NUL;
        break;
    }

    return quit;
}

int entering_password_function(unsigned char input, int repeating){
    int quit = 0;
    switch(input){
    case '<':
        SEND_ACK;
        tx_string("\nAborted Password Input");
        rx_user_menu_data();
        change_state(USER_STATE, repeating);
        SEND_NUL;
        break;
    default:
        if(!repeating){
            if(new_state_password[new_state_password_index] != input){
                incorrect_password = 1;
            }
            new_state_password_index++;
        }
        SEND_ACK;
        if(new_state_password[new_state_password_index] == '\0'){
            SEND_BOOL(incorrect_password);
            if(incorrect_password){
                rx_user_menu_data();
                change_state(USER_STATE, repeating);
            }else{
                if(!repeating){//change_state(SELECT_NEW_STATE, repeating);
                    clear_setting_name();
                }
                rx_setting_name();
                change_state(DATE_STATE, repeating);
            }
        }else{
            rx_entering_password();
            change_state(ENTERING_PASSWORD_STATE, repeating);
        }
        SEND_NUL;
        break;
    }
    return quit;
}

int select_new_function(unsigned char input,     int repeating){
    int quit = 0;
    switch(input){
    case '1':
        WRITE_FRAM_ENABLE;
        device_is_active = 0;
        wet_and_dry_is_active = 0;
        temperature_is_active = 0;
        WRITE_FRAM_DISABLE;
        new_state_just_configured = 1;
        quit = 1;
        SEND_ACK;
        tx_string(bye_message);
        SEND_NUL;
        break;
    case '2':
        WRITE_FRAM_ENABLE;
        device_is_active = 1;
        wet_and_dry_is_active = 1;
        temperature_is_active = 0;
        WRITE_FRAM_DISABLE;
        new_state_just_configured = 1;
        quit = 1;
        SEND_ACK;
        tx_string(bye_message);
        SEND_NUL;
        break;
    case '3':
        WRITE_FRAM_ENABLE;
        device_is_active = 1;
        wet_and_dry_is_active = 0;
        temperature_is_active = 1;
        WRITE_FRAM_DISABLE;
        new_state_just_configured = 1;
        quit = 1;
        SEND_ACK;
        tx_string(bye_message);
        SEND_NUL;
        break;
    case '4':
        WRITE_FRAM_ENABLE;
        device_is_active = 1;
        wet_and_dry_is_active = 1;
        temperature_is_active = 1;
        WRITE_FRAM_DISABLE;
        new_state_just_configured = 1;
        quit = 1;
        SEND_ACK;
        tx_string(bye_message);
        SEND_NUL;
        break;
    case '<':
        SEND_ACK;
        rx_setting_name();
        change_state(DATE_STATE, repeating);
        SEND_NUL;
        break;
    default:
        SEND_NAK;
        tx_string("\nUnknown Command");
        change_state(SELECT_NEW_STATE, repeating);
        SEND_NUL;
        break;
    }
    return quit;
}
