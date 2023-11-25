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


#define IGNORE_FIRST_MOMENTS true

// TODO tracking_buffer_valid resetear y asi tambien los otros

bool master_control = false;
bool operating_mode_set = false;


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
char operating_mode_to_set = OPERATING_MODE__INACTIVE;

typedef int HandlerOutput_t;
typedef HandlerOutput_t PCinputHandler(char input);
#define NO_ACTION_NEEDED ((HandlerOutput_t)0) 
#define AN_ERROR_OCCURRED ((HandlerOutput_t)-1)
#define DISCONNECT_MSP   ((HandlerOutput_t)1)  


HandlerOutput_t H_get_fast_data(char input){
    bool answer = get_fast_data();
    if(answer){
        PC.print(OK_INDICATOR);
        PC.write(TERMINATOR);
        return NO_ACTION_NEEDED;
    }else{
        PC.write(ERROR_INDICATOR);
        PC.write(TERMINATOR);
        return AN_ERROR_OCCURRED;
    }
   
}

HandlerOutput_t H_get_last_events(char input){
    bool answer = get_last_events();
    if(answer){
        PC.print(OK_INDICATOR);
        PC.write(TERMINATOR);
        return NO_ACTION_NEEDED;
    }else{
        PC.write(ERROR_INDICATOR);
        PC.write(TERMINATOR);
        return AN_ERROR_OCCURRED;
    }     
}


HandlerOutput_t H_unknown_input(char input){
    PC.write(ERROR_INDICATOR);
    PC.write(TERMINATOR);
    return AN_ERROR_OCCURRED;
}

HandlerOutput_t H_toggle_led(char input){
    const char* answer = toggle_led();
    if(answer != NULL){
        PC.print(answer);
        PC.write(TERMINATOR);
        return NO_ACTION_NEEDED;
    }else{
        PC.write(ERROR_INDICATOR);
        PC.write(TERMINATOR);
        return AN_ERROR_OCCURRED;
    }   
}

HandlerOutput_t H_get_FSM_state(char input){
    int answer = get_current_menu();
    PC.print(answer);
    PC.write(TERMINATOR);
    return answer == UNKNOWN_MENU? AN_ERROR_OCCURRED: NO_ACTION_NEEDED;
}

HandlerOutput_t H_activate_master(char input){
    master_control = true;
    PC.print("ACTIVATED MASTER CONTROL\n");
    return NO_ACTION_NEEDED;
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

HandlerOutput_t H_set_operating_mode1(char input){
    setting_operating_mode = true;
    return NO_ACTION_NEEDED;
}

HandlerOutput_t H_set_operating_mode2(char input){
    setting_operating_mode = false;
    operating_mode_set = true;
    if(input < '0'){
        operating_mode_to_set ='0';
    }else if (input > '3'){
        operating_mode_to_set ='3';
    }else{
        operating_mode_to_set = input;
    }

    PC.print(OK_INDICATOR);
    PC.write(TERMINATOR);

    return NO_ACTION_NEEDED;
}

HandlerOutput_t H_get_operating_mode(char input){
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
            return NO_ACTION_NEEDED;
        }else{
            PC.write(operating_mode_to_set);
            PC.write(TERMINATOR);
            return AN_ERROR_OCCURRED;
        }     
    }else{
        PC.write(operating_mode_to_set);
        PC.write(TERMINATOR);
        return NO_ACTION_NEEDED;
    }

}

HandlerOutput_t H_get_indexes(char input){
    const char* answer = get_from_user_menu_helper(6,5);
    if(answer != NULL){
        PC.print(answer);
        PC.write(TERMINATOR);
        return NO_ACTION_NEEDED;
    }else{
        PC.write(ERROR_INDICATOR);
        PC.write(TERMINATOR);
        return AN_ERROR_OCCURRED;
    }     
}

HandlerOutput_t H_get_handler(char option, int variable = 1){
    const char* answer = get_helper(option,variable);
    if(answer != NULL){
        PC.print(answer);
        PC.write(TERMINATOR);
        return NO_ACTION_NEEDED;
    }else{
        PC.write(ERROR_INDICATOR);
        PC.write(TERMINATOR);
        return AN_ERROR_OCCURRED;
    }   
}



HandlerOutput_t H_get_wd_freq(char input){
    return H_get_handler('4');
}
HandlerOutput_t H_get_temp_freq(char input){
    return H_get_handler('5');
}
HandlerOutput_t H_get_calibrated_min(char input){
    return H_get_handler('6');
}
HandlerOutput_t H_get_penguin_name(char input){
    return H_get_handler('7');
}
HandlerOutput_t H_get_logger_ID(char input){
    return H_get_handler('8');
}
HandlerOutput_t H_get_activ_time(char input){
    if (!activation_time_set){
        return H_get_handler('9');
    }else{
        PC.print(activation_time_array);
        PC.write(TERMINATOR);
        return NO_ACTION_NEEDED;
    }
}
HandlerOutput_t H_get_at30(char input){
    return H_get_handler('0',1);
}
HandlerOutput_t H_get_at105(char input){
    return H_get_handler('0',2);
}

HandlerOutput_t H_atime_finished_handler(void){
    PC.print(OK_INDICATOR);
    PC.write(TERMINATOR);
    activation_time_set = true;
    return NO_ACTION_NEEDED;    
}

// TODO: que si no recibe pronto ], considera que fue error!
HandlerOutput_t H_set_finished_handler(void){
    bool answer = set_helper(setting_menu_selected, setting_command,setting_array);
    if(answer){
        PC.print(OK_INDICATOR);
        PC.write(TERMINATOR);
        return NO_ACTION_NEEDED;
    }else{
        PC.write(ERROR_INDICATOR);
        PC.write(TERMINATOR);
        return AN_ERROR_OCCURRED;
    }   
}

HandlerOutput_t H_atime_init_handler(char input){
    setting_activation_time = true;
    activation_time_index = 0;
    activation_time_array[activation_time_index] = 0;
    return NO_ACTION_NEEDED;
}

HandlerOutput_t H_set_init_handler(char command, int setting_menu){
    setting = true;
    setting_index = 0;
    setting_array[setting_index] = 0;
    setting_command = command;
    setting_menu_selected = setting_menu;
    return NO_ACTION_NEEDED;
}

HandlerOutput_t H_set_wd_freq(char input){
    return H_set_init_handler('4',SET_MENU);
}
HandlerOutput_t H_set_temp_freq(char input){
    return H_set_init_handler('5',SET_MENU);
}
HandlerOutput_t H_set_calibrated_min(char input){
    return H_set_init_handler('6',SET_MENU);
}
HandlerOutput_t H_set_penguin_name(char input){
    return H_set_init_handler('N',DEBUG_MENU);
}

HandlerOutput_t H_quit_handler(char operating_mode){
    bool answer = false;
    if (activation_time_set){
        answer = quit_helper(operating_mode, activation_time_array);
    }
    else{
        answer = quit_helper(operating_mode, "");
    }
    if(answer){
        PC.print(OK_INDICATOR);
        PC.write(TERMINATOR);
        return DISCONNECT_MSP;
    }else{
        PC.write(ERROR_INDICATOR);
        PC.write(TERMINATOR);
        return AN_ERROR_OCCURRED;        
    }
}
   

HandlerOutput_t H_activate_logger(char input){
    return H_quit_handler(operating_mode_to_set);
}
HandlerOutput_t H_deactivate_logger(char input){
    return H_quit_handler(OPERATING_MODE__INACTIVE);
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
    &H_unknown_input,       &H_get_activ_time,      &H_unknown_input,       &H_get_calibrated_min,  &H_get_fast_data,       &H_get_last_events,     &H_get_FSM_state,       &H_unknown_input,
//  H                       I                       J                       K                       L                       M                       N                       O
    &H_unknown_input,       &H_get_logger_ID,       &H_unknown_input,       &H_unknown_input,       &H_toggle_led,          &H_get_operating_mode,  &H_get_penguin_name,    &H_unknown_input,
//  P                       Q                       R                       S                       T                       U                       V                       W
    &H_unknown_input,       &H_activate_logger,     &H_unknown_input,       &H_unknown_input,       &H_get_temp_freq,       &H_unknown_input,       &H_unknown_input,       &H_get_wd_freq,
//  X                       Y                       Z                       [                       \                       ]                       ^                       _
    &H_get_indexes,         &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,
//  `                       a                       b                       c                       d                       e                       f                       g
    &H_unknown_input,       &H_atime_init_handler,  &H_unknown_input,       &H_set_calibrated_min,  &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,
//  h                       i                       j                       k                       l                       m                       n                       o
    &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_set_operating_mode1, &H_set_penguin_name,    &H_unknown_input,
//  p                       q                       r                       s                       t                       u                       v                       w
    &H_unknown_input,       &H_deactivate_logger,   &H_unknown_input,       &H_unknown_input,       &H_set_temp_freq,       &H_unknown_input,       &H_unknown_input,       &H_set_wd_freq,
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
    if(IGNORE_FIRST_MOMENTS){
        return true;
    }

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
    int return_from_handler = 0;

    DISABLE_5V;
    DISABLE_TX;
    
    master_control = false;
    setting = false;
    setting_operating_mode = false;
    setting_activation_time = false;
    operating_mode_set = false;
    activation_time_set = false; 


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
                operating_mode_to_set = OPERATING_MODE__INACTIVE;
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
                                return_from_handler = (*input_handlers[byte_from_pc])(byte_from_pc); // DO SOMETHING WITH -1!!!
                            }
                        }else if(setting){
                            if(byte_from_pc == SETTING_INPUT_FINISHED){
                                return_from_handler = H_set_finished_handler(); // DO SOMETHING WITH -1!!!
                                setting = false;
                            }else{
                                if(setting_index < 255){
                                    setting_array[setting_index++] =  byte_from_pc;
                                    setting_array[setting_index] = 0;
                                }

                            }
                            
                        }else if (setting_activation_time){
                            if(byte_from_pc == SETTING_INPUT_FINISHED){
                                return_from_handler= H_atime_finished_handler(); // DO SOMETHING WITH -1!!!
                                setting_activation_time = false;
                            }else{
                                if(activation_time_index < 255){
                                    activation_time_array[activation_time_index++] =  byte_from_pc;
                                    activation_time_array[activation_time_index] = 0;
                                }

                            }

                        }
                        else{ //setting_operating_mode
                            return_from_handler = H_set_operating_mode2(byte_from_pc);
                        }

                        // TODO: return_from_handler do something!
                        switch(return_from_handler){
                            case DISCONNECT_MSP:
                            quit = true;
                            break;

                            default:
                            break;
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
            change_led_color(BASE_POWER_ON);
        }


    }
}



