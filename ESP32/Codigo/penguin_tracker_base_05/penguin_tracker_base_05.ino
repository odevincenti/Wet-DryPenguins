#include "commands_from_pc.h"
#include "commands_for_msp.h"
#include "led_functions.h"
//#include "test.h"
#include "pinout.h"

#define TERMINATOR '\n'
#define ERROR_INDICATOR '\x15'


#define ENABLE_5V     digitalWrite(PIN_5V_ENABLE,LOW)
#define DISABLE_5V    digitalWrite(PIN_5V_ENABLE,HIGH)

#define ENABLE_TX     digitalWrite(PIN_TX_ENABLE,LOW)
#define DISABLE_TX    digitalWrite(PIN_TX_ENABLE,HIGH)

bool master_control = false;

typedef int PCinputHandler(char input);

int H_get_fast_data(char input){
    return 0;
}
int H_regular_input(char input){
    return 0;
}

int H_unknown_input(char input){
    PC.print("UNKNOWN PC INPUT = ");
    PC.write(input);
    PC.print("(");
    PC.print((int)input);
    PC.print(")\n");
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
    return H_get_handler('9');
}
int H_get_at30(char input){
    return H_get_handler('0',1);
}
int H_get_at105(char input){
    return H_get_handler('0',2);
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
    &H_unknown_input,       &H_get_activ_time,      &H_unknown_input,       &H_get_calibrated_min,  &H_unknown_input,       &H_unknown_input,       &H_get_FSM_state,       &H_unknown_input,
//  H                       I                       J                       K                       L                       M                       N                       O
    &H_unknown_input,       &H_get_logger_ID,       &H_unknown_input,       &H_unknown_input,       &H_toggle_led,          &H_unknown_input,       &H_get_penguin_name,    &H_unknown_input,
//  P                       Q                       R                       S                       T                       U                       V                       W
    &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_get_temp_freq,       &H_unknown_input,       &H_unknown_input,       &H_get_wd_freq,
//  X                       Y                       Z                       [                       \                       ]                       ^                       _
    &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,
//  `                       a                       b                       c                       d                       e                       f                       g
    &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,
//  h                       i                       j                       k                       l                       m                       n                       o
    &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,
//  p                       q                       r                       s                       t                       u                       v                       w
    &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,       &H_unknown_input,
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
                connected = true;
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
            }
        }else{
            PC.print("FAILED TO CONNECT: \"OK\" MESSAGE NOT RECEIVED\n");
            change_led_color(FAILED_TO_CONNECT);
        }
        /*
        change_led_color(CONNECTING);
        ENABLE_5V;
        //discard_data_from_msp();
        delay(1000);
        MSP.write("1234");
        delay(500);
        //discard_data_from_msp();
        change_led_color(CONNECTED); // TODO: CHECKEAR QUE ESTÉ CONECTADO!!! HACERLO BIEN!!! CHANGOS!!
        */
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
                        if(byte_from_pc >= 0 && byte_from_pc < 128){
                            (*input_handlers[byte_from_pc])(byte_from_pc);
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


