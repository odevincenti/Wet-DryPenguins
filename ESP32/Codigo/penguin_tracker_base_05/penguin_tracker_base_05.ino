#include "commands_from_pc.h"
#include "commands_for_msp.h"
#include "led_functions.h"
//#include "test.h"
#include "pinout.h"

#define TERMINATOR '\n'
//TODO  reemplazar OK por ACK '\x06' cuando termine de verlo por Arduino IDE
#define OK_INDICATOR "OK"
#define ERROR_INDICATOR '\x15'
#define SETTING_INPUT_FINISHED ']'


#define ENABLE_5V     digitalWrite(PIN_5V_ENABLE,LOW)
#define DISABLE_5V    digitalWrite(PIN_5V_ENABLE,HIGH)

#define ENABLE_TX     digitalWrite(PIN_TX_ENABLE,LOW)
#define DISABLE_TX    digitalWrite(PIN_TX_ENABLE,HIGH)

bool master_control = false;
bool operating_mode_set = false;
char new_operating_mode = 0; 





bool setting = false;
bool setting_operating_mode = false;
bool setting_activation_time = false;

char setting_array[256] = "";
char setting_command = 0;
int setting_index = 0;
int setting_menu_selected = SET_MENU;

char activation_time_array[256] = "";
int activation_time_index = 0;
bool activation_time_set = false;

// TODO!
char starting_date_input[256] = "";
int operating_mode_to_set = OPERATING_MODE__INACTIVE;

typedef int PCinputHandler(char input);

int H_get_fast_data(char input){
    return 0;
}

int H_get_last_events(char input){
    bool answer = get_last_events();
    if(answer){
        PC.print(OK_INDICATOR);
        PC.write(TERMINATOR);
        return 0;
    }else{
        PC.write(ERROR_INDICATOR);
        PC.write(TERMINATOR);
        return -1;
    }     
}


int H_unknown_input(char input){
    //PC.print("UNKNOWN PC INPUT = ");
    //PC.write(input);
    //PC.print("(");
    //PC.print((int)input);
    //PC.print(")\n");
    PC.write(ERROR_INDICATOR);
    PC.write(TERMINATOR);
    return -1;
}

int H_toggle_led(char input){
    const char* answer = toggle_led();
    if(answer != NULL){
        PC.print(answer);
        PC.write(TERMINATOR);
        return 0;
    }else{
        PC.write(ERROR_INDICATOR);
        PC.write(TERMINATOR);
        return -1;
    }   
}

int H_get_FSM_state(char input){
    int answer = get_current_menu();
    PC.print(answer);
    PC.write(TERMINATOR);
    return answer == UNKNOWN_MENU? -1: 0;
}

int H_activate_master(char input){
    master_control = true;
    PC.print("ACTIVATED MASTER CONTROL\n");
    return 0;
}

bool compare_strings(const char* substring, const char* main_string){
    bool answer = true;
    const char* substring_pointer = substring;
    const char* main_string_pointer = main_string;
    while(*main_string_pointer != '\0' && *substring_pointer != '\0' && answer == true){
        if (*main_string_pointer != *substring_pointer){
            answer = false;
        }
        substring_pointer++;
        main_string_pointer++;
    }
    return answer;
}

int H_set_operating_mode1(char input){
    setting_operating_mode = true;
    PC.print("SET OPERATING MODE PART 1\n");
    return 0;
}

int H_set_operating_mode2(char input){
    setting_operating_mode = false;
    operating_mode_set = true;
    if(input < '0'){
        new_operating_mode ='0';
    }else if (input > '3'){
        new_operating_mode ='3';
    }else{
        new_operating_mode = input;
    }
    
    PC.print("SET OPERATING MODE PART 2\n");
    PC.print(OK_INDICATOR);
    PC.write(TERMINATOR);

    return 0;
}

int H_get_operating_mode(char input){
    if (!operating_mode_set){
        const char* answer = get_from_user_menu_helper(1);
        if(answer != NULL){
            if(compare_strings("INACTIVE",answer)){
                PC.write(OPERATING_MODE__INACTIVE);
            }else if (compare_strings("WET AND DRY ONLY",answer)){
                PC.write(OPERATING_MODE__WET_AND_DRY);
            }else if (compare_strings("TEMPERATURE ONLY",answer)){
                PC.write(OPERATING_MODE__TEMPERATURE);
            }else if (compare_strings("WET AND DRY & TEMPERATURE",answer)){
                PC.write(OPERATING_MODE__BOTH);
            }else{
                PC.write(ERROR_INDICATOR);
            }

            PC.write(TERMINATOR);
            return 0;
        }else{
            PC.write(new_operating_mode);
            PC.write(TERMINATOR);
            return -1;
        }     
    }else{
        PC.write(new_operating_mode);
        PC.write(TERMINATOR);
        return 0;
    }

}

int H_get_indexes(char input){
    const char* answer = get_from_user_menu_helper(6,5);
    if(answer != NULL){
        PC.print(answer);
        PC.write(TERMINATOR);
        return 0;
    }else{
        PC.write(ERROR_INDICATOR);
        PC.write(TERMINATOR);
        return -1;
    }     
}

int H_get_handler(char option, int variable = 1){
    const char* answer = get_helper(option,variable);
    if(answer != NULL){
        PC.print(answer);
        PC.write(TERMINATOR);
        return 0;
    }else{
        PC.write(ERROR_INDICATOR);
        PC.write(TERMINATOR);
        return -1;
    }   
}



int H_get_wd_freq(char input){
    return H_get_handler('4');
}
int H_get_temp_freq(char input){
    return H_get_handler('5');
}
int H_get_calibrated_min(char input){
    return H_get_handler('6');
}
int H_get_penguin_name(char input){
    return H_get_handler('7');
}
int H_get_logger_ID(char input){
    return H_get_handler('8');
}
int H_get_activ_time(char input){
    if (!activation_time_set){
        return H_get_handler('9');
    }else{
        PC.print(activation_time_array);
        PC.write(TERMINATOR);
        return 0;
    }
}
int H_get_at30(char input){
    return H_get_handler('0',1);
}
int H_get_at105(char input){
    return H_get_handler('0',2);
}

int H_atime_finished_handler(void){
    PC.print(OK_INDICATOR);
    PC.write(TERMINATOR);
    activation_time_set = true;
    return 0;    
}

// TODO: que si no recibe pronto ], considera que fue error!
int H_set_finished_handler(void){
    bool answer = set_helper(setting_menu_selected, setting_command,setting_array);
    if(answer){
        //PC.print("RECIVED TRUE\n");
        PC.print(OK_INDICATOR);
        PC.write(TERMINATOR);
        return 0;
    }else{
        //PC.print("RECIVED FALSE\n");
        PC.write(ERROR_INDICATOR);
        PC.write(TERMINATOR);
        return -1;
    }   
}

int H_atime_init_handler(char input){
    setting_activation_time = true;
    activation_time_index = 0;
    activation_time_array[activation_time_index] = 0;
    return 0;
}

int H_set_init_handler(char command, int setting_menu){
    setting = true;
    setting_index = 0;
    setting_array[setting_index] = 0;
    setting_command = command;
    setting_menu_selected = setting_menu;
    return 0;
}

int H_set_wd_freq(char input){
    return H_set_init_handler('4',SET_MENU);
}
int H_set_temp_freq(char input){
    return H_set_init_handler('5',SET_MENU);
}
int H_set_calibrated_min(char input){
    return H_set_init_handler('6',SET_MENU);
}
int H_set_penguin_name(char input){
    return H_set_init_handler('N',DEBUG_MENU);
}


PCinputHandler* input_handlers[128] = { // RETURN 0 = OK; -1 = NOT OK!
//  [NUL]                   [STX]                   [SOT]                   [ETX]                   [EOT]                   [ENQ]                   [ACK]                   [BEL] 
    &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,
//  [BS]                    [HT]                    [LF]                    [VT]                    [FF]                    [CR]                    [SO]                    [SI]  
    &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,
//  [DLE]                   [DC1]                   [DC2]                   [DC3]                   [DC4]                   [NAK]                   [SYN]                   [ETB]  
    &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,
//  [CAN]                   [EM]                    [SUB]                   [ESC]                   [FS]                    [GS]                    [RS]                    [US] 
    &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,
//  [SP]                    !                       "                       #                       $                       %                       &                       '
    &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,
//  (                       )                       *                       +                       ,                       -                       .                       /
    &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,
//  0                       1                       2                       3                       4                       5                       6                       7 
    &H_unknown_input,       &H_get_at105,           &H_unknown_input,       &H_get_at30,            &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,
//  8                       9                       :                       ;                       <                       =                       >                       ?
    &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,
//  @                       A                       B                       C                       D                       E                       F                       G
    &H_unknown_input,       &H_get_activ_time,      &H_unknown_input,       &H_get_calibrated_min,  &H_unknown_input,       &H_get_last_events,     &H_get_FSM_state,       &H_unknown_input,
//  H                       I                       J                       K                       L                       M                       N                       O
    &H_unknown_input,       &H_get_logger_ID,       &H_unknown_input,       &H_unknown_input,       &H_toggle_led,          &H_get_operating_mode,  &H_get_penguin_name,    &H_unknown_input,
//  P                       Q                       R                       S                       T                       U                       V                       W
    &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_get_temp_freq,       &H_unknown_input,       &H_unknown_input,       &H_get_wd_freq,
//  X                       Y                       Z                       [                       \                       ]                       ^                       _
    &H_get_indexes,         &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,
//  `                       a                       b                       c                       d                       e                       f                       g
    &H_unknown_input,       &H_atime_init_handler,  &H_unknown_input,       &H_set_calibrated_min,  &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,
//  h                       i                       j                       k                       l                       m                       n                       o
    &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_set_operating_mode1, &H_set_penguin_name,    &H_unknown_input,
//  p                       q                       r                       s                       t                       u                       v                       w
    &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_set_temp_freq,       &H_unknown_input,       &H_unknown_input,       &H_set_wd_freq,
//  x                       y                       z                       {                       |                       }                       ~                       [DEL]
    &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_activate_master,     &H_unknown_input,
};


void setup() {
    pinMode(PIN_5V_ENABLE,OUTPUT);
    DISABLE_5V;
    pinMode(PIN_TX_ENABLE,OUTPUT);
    DISABLE_TX;

    setup_led();

    PC.begin(115200);
    MSP.begin(115200, SERIAL_8O2);

    change_led_color(BASE_POWER_ON);

    master_control = false;
    setting = false;
    setting_operating_mode = false;
    setting_activation_time = false;
    operating_mode_set = false;
    activation_time_set = false;

    //test_millis();
}

//      FUNCTION:
//  Enables 5V to base_detect.
//  If receives "OK\n" from MSP, outputs 1
//  Otherwise, outputs 0 and disables 5V 
//      OUTPUT:
//  1 = Connected, received "OK\n" message from MSP
//  0 = Failed to connect
int enable_base_detect(void){
    const char starting_input[] = "OK\n";
    bool connected = false;
    int timeout = millis()+1000;
    int index = 0;
    char byte_from_msp = 0;
    //change_led_color(CONNECTING);
    ENABLE_5V;
    while(!connected && millis()<timeout){
        if(MSP.available()){
            byte_from_msp = MSP.read();
            if(byte_from_msp == starting_input[index]){
                index++;
                if(index == 3){
                    connected = true;
                    //change_led_color(CONNECTED);
                }
            }else{
                index = 0;
            }
        }
    }
    if(!connected){
        DISABLE_5V;
        //change_led_color(FAILED_TO_CONNECT);
    }
    return connected;
}

void loop(){
    char byte_from_msp = '\0';
    char byte_from_pc = '\0';
    int last_time;
    bool quit = false;

    bool connected = false;

    bool answer_from_msp_received= false;



    while(PC.sent_bytes() <= 0){ // Se queda acá hasta que reciba datos. Se podría hacer algo en el medio? O hacer que este en "sleep"?
        if(MSP.available()){
            byte_from_msp = MSP.read();
            PC.print("WARNING! RECEIVED = ");
            PC.print(byte_from_msp);
            PC.print("(");
            PC.print((int)byte_from_msp);
            PC.print(")\n");
        }

    }
    if(PC.read() == CONNECT_TO_MSP){
        connected = false;
        change_led_color(CONNECTING);
        if(enable_base_detect()){
            ENABLE_TX;
            delay(500);
            MSP.write("1234");
            answer_from_msp_received = receive_answer_from_msp();
            if(answer_from_msp_received && get_current_menu() == DEBUG_MENU){
                change_led_color(CONNECTED);
                setting = false;
                setting_operating_mode = false;
                setting_activation_time = false;
                connected = true;
                operating_mode_set = false;
                activation_time_set = false;
                PC.print(OK_INDICATOR);
                PC.write(TERMINATOR);
            }else{
                DISABLE_TX;
                DISABLE_5V;
                // TODO: ENVIAR MENSAJE DE ERROR
                if(!answer_from_msp_received){
                    PC.print("FAILED TO CONNECT: MESSAGE FROM MSP NOT VALID\n");
                }else{
                    PC.print("FAILED TO CONNECT: MAIN MENU NOT REACHED. MENU ");
                    PC.print(get_current_menu());
                    PC.print("INSTEAD\n");
                }
                change_led_color(FAILED_TO_CONNECT);
                PC.write(ERROR_INDICATOR);
                PC.write(TERMINATOR);
            }
        }else{
            PC.print("FAILED TO CONNECT: \"OK\" MESSAGE NOT RECEIVED\n");
            change_led_color(FAILED_TO_CONNECT);
            PC.write(ERROR_INDICATOR);
            PC.write(TERMINATOR);
        }

        if (connected){

            
            last_time = millis();
            while(!quit){//TODO: quit
                if(millis() - last_time >= 1000){
                    keep_awake_msp();
                    last_time = millis();
                }

                if(PC.available()){
                    /*if(MSP.available()){
                        PC.print("WAIT! MSP HAVE NOT FINISHED SENDING MESSAGE!\n");
                    }else{
                        byte_from_pc = PC.read();
                        send_and_print_received((char)byte_from_pc);
                    }*/

                    byte_from_pc = PC.read();

                    if(!master_control){
                        if(!setting && !setting_operating_mode && !setting_activation_time){
                            if(byte_from_pc >= 0 && byte_from_pc < 128){
                                (*input_handlers[byte_from_pc])(byte_from_pc); // DO SOMETHING WITH -1!!!
                            }
                        }else if(setting){
                            if(byte_from_pc == SETTING_INPUT_FINISHED){
                                //PC.print("SETTING FINISHED, COMMAND = ");
                                //PC.write(setting_command);
                                //PC.print(" AT MENU ");
                                //PC.print(setting_menu_selected);
                                //PC.print("\nSETTING STRING = ");
                                //PC.print(setting_array);
                                //PC.print("\n");
                                H_set_finished_handler(); // DO SOMETHING WITH -1!!!
                                setting = false;
                            }else{
                                if(setting_index < 255){
                                    setting_array[setting_index++] =  byte_from_pc;
                                    setting_array[setting_index] = 0;
                                }

                                //PC.print("SETTING, RECEIVED = ");
                                //PC.write(byte_from_pc);
                                //PC.print("\n");
                            }
                            
                        }else if (setting_activation_time){
                            if(byte_from_pc == SETTING_INPUT_FINISHED){
                                H_atime_finished_handler(); // DO SOMETHING WITH -1!!!
                                setting_activation_time = false;
                            }else{
                                if(activation_time_index < 255){
                                    activation_time_array[activation_time_index++] =  byte_from_pc;
                                    activation_time_array[activation_time_index] = 0;
                                }

                            }

                        }
                        else{ //setting_operating_mode
                            H_set_operating_mode2(byte_from_pc);
                        }
                    }else{
                        if (byte_from_pc == MASTER_CONTROL_COMMAND){
                            master_control = false;
                        }else{
                            if (byte_from_pc == 'F'){
                                send_and_print_received((char)byte_from_pc, true, 3600000);
                            }else{
                                send_and_print_received((char)byte_from_pc);
                            }
                            last_time = millis();    
                        }                    
                    }
                    

                    
                }
                if (MSP.available()){
                    PC.print("REMAINING = ");
                    PC.print(MSP.available());
                    PC.write('\n');
                    byte_from_msp = MSP.read();
                }

            }
            DISABLE_TX;
            DISABLE_5V;
        }


    }
}

/*
                    switch((char) byte_from_pc){
                    case GET_FSM_STATE:                 // F
                        PC.print("Y\n"); //TODO: que funcione de verdad!!
                        break;
                    case GET_WET_AND_DRY_FREC:    // W
                        get_helper_function('4');
                        last_time = millis();
                        break;
                    case GET_TEMPERATURE_FREC:    // T
                        get_helper_function('5');
                        last_time = millis();
                        break;
                    case GET_CALIBRATED_MINUTE: // C
                        get_helper_function('6');
                        last_time = millis();
                        break;
                    case GET_NAME:                            // N
                        get_helper_function('7');
                        last_time = millis();
                        break;
                    case GET_LOGGER_ID:                 // I
                        get_helper_function('8');
                        last_time = millis();
                        break;
                    case TOGGLE_LED:                        // L
                        toggle_led();
                        last_time = millis();
                        break;
                    case GET_ACTIVATION_TIME:     // A
                        get_helper_function('9');
                        last_time = millis();
                        break;
                    case GET_AT_30C:                        // 3
                        get_calibration_C(at30C);
                        last_time = millis();
                        break;
                    case GET_AT_105C:                     // 1
                        get_calibration_C(at105C);
                        last_time = millis();
                        break;    
                    case GET_ALL_INDEX:                 // X
                        get_indexes();
                        last_time = millis();
                        break;
                    case GET_OPERATING_MODE:        //M
                        get_operating_mode();
                        last_time = millis();
                        break;

                    case '?':
                        test_buffer();
                        last_time = millis();
                        break; 

                    default:
                        break;
                    }

                    */


