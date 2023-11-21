#include "commands_for_msp.h"

#define INPUT_BUFFER_SIZE 5000
#define TRACKING_BUFFER_SIZE 28000
//50000
#define ASTERISK_STRING_SIZE 25

static char input_buffer[INPUT_BUFFER_SIZE] = "";
static int input_buffer_index = 0;
static int input_buffer_time = 0;

static unsigned char tracking_buffer[TRACKING_BUFFER_SIZE] = "";
static bool tracking_buffer_valid = false;
static char tracking_single_buffer[4] = "";

static char const start_of_tracking_data_string[3] = {0x06,0xA,0xA};

static void send_command_to_msp(char command);
static int receive_answer_from_msp(int time_out = 500, bool long_data = false);  // TODO: separar entre "tiempo hasta terminar mensaje" y "tiempo sin recivir nada". No olvida posibilidad de ruido!!!
static void print_input_buffer(bool long_data = false);

static bool convert_tracking_number(int tracking_buffer_index){
    bool valid_data = true;
    unsigned char number = 0; 
    char input_char;
    int i;
    int j;
    for (i=0;i<3 && valid_data;i++){
        input_char = tracking_single_buffer[i];  
        if (input_char =='\0'){
            break;
        }else if(input_char < '0' || input_char > '9'){
            valid_data = false;
        }else{
            number = (number*10)+(input_char - '0');
        }
        
    }
    if (tracking_single_buffer[i] != '\0' || tracking_single_buffer[0] == '\0'){
        valid_data= false;
    }

    if(valid_data){
        tracking_buffer[tracking_buffer_index] = number;
    }
    if(!valid_data){
        PC.print("TRACKING SINGLE BUFFER = ");
        PC.print("\n");
        for (j=0;j<4;j++){
            PC.write((int) tracking_single_buffer[j]);
            PC.print(" = ");
            PC.print((int) tracking_single_buffer[j]);
            PC.print("\n");
        }
    }
    return valid_data;
}

void keep_awake_msp(void){
    MSP.write("\n");
}

void send_and_print_received(char sending, bool long_data, int time_out){
    bool received = false;
    PC.print("\n\tSENDING: ");
    PC.print(sending);
    PC.print("\n");
    send_command_to_msp(sending);
    received = receive_answer_from_msp(time_out,long_data);//(4000000);
    if (received){
        PC.print("\tMESSAGE RECEIVED!\n");
    }else{
        PC.print("\t WARNING! MESSAGED NOT RECEIVED PROPERLY!!!\n");
    }
    PC.print("\tINPUT BUFFER INDEX = ");
    PC.print(input_buffer_index);
    PC.print(" / ");
    PC.print(INPUT_BUFFER_SIZE);
    PC.print("\n\tINPUT BUFFER WRITING TIME = ");
    PC.print(input_buffer_time);
    PC.print("\n");
    print_input_buffer(long_data);

}

/////////////////////////////////////////////////////

static void send_command_to_msp(char command){
    MSP.print(command);
}

static int receive_answer_from_msp(int time_out, bool long_data){
    int starting_time = millis();
    int time_out_time = starting_time + time_out;
    bool message_received = false;
    int input_from_MSP = 0;
    int remaining_asterisks = ASTERISK_STRING_SIZE;

    int tracking_time = 0;
    int tracking_index = 0;
    bool tracking_error = false;
    int single_char_index=0;
    int i;
    

    input_buffer_index = 0;

    if(long_data){
        tracking_buffer_valid = false;
        for(i=0;i<3;i++){
            tracking_time = millis();
            while(!tracking_error && millis() - tracking_time < 1000){
                if(MSP.sent_bytes()){
                    if(MSP.read() != start_of_tracking_data_string[i]){
                        PC.print("ERROR 1\n");
                        tracking_error=true;
                    }else{
                        break;
                    }
                    tracking_time = millis();
                }
            }
            if (millis()-tracking_time >= 1000){
                PC.print("ERROR 2\n");
                tracking_error = true;
            }
        }

        if(!tracking_error){
            PC.print("RECEIVED START OF DATA\n");
        }

        single_char_index=0;
        tracking_index = 0;
        tracking_time = millis();
        while(!tracking_error && tracking_index < TRACKING_BUFFER_SIZE && millis() - tracking_time < 1000){
            if(MSP.sent_bytes()){
                input_from_MSP  = MSP.read();
                if(input_from_MSP >= '0' && input_from_MSP <= '9'){
                    if(single_char_index < 3){
                        tracking_single_buffer[single_char_index++]=input_from_MSP;
                    }else{
                        PC.print("ERROR 6\n");
                        tracking_error= true; 
                    }
                }else if(input_from_MSP == '\t'){
                    tracking_single_buffer[single_char_index] = '\0';
                    if(convert_tracking_number(tracking_index)){
                        tracking_index++;
                        single_char_index=0;
                    }else{
                        PC.print("ERROR 3\n");
                        tracking_error= true;
                    }
                }else if (input_from_MSP == '\n'){
                    // DO NOTHING
                }else{
                    PC.print("ERROR 4\n");
                    tracking_error= true;
                }

                tracking_time = millis();
            }
        }
        if (millis()-tracking_time >= 1000){
            PC.print("ERROR 5\n");
            tracking_error = true;
        }
        if(!tracking_error && tracking_index == TRACKING_BUFFER_SIZE){
            tracking_buffer_valid = true;
        }


    }

    while(!message_received && millis() < time_out_time && input_buffer_index < INPUT_BUFFER_SIZE){
        if(MSP.sent_bytes()){
            input_from_MSP  = MSP.read();
            input_buffer[input_buffer_index++] = (char) input_from_MSP;
            if(input_from_MSP == '*'){
                --remaining_asterisks;
            }
            if(input_from_MSP == '\n'){
                if(remaining_asterisks){
                    remaining_asterisks = ASTERISK_STRING_SIZE;
                }else{
                    message_received = true;
                }
            }
        }
    }
    input_buffer_time = millis()-starting_time;
    return message_received && (tracking_buffer_valid || !long_data);
}

static void print_input_buffer(bool long_data){
    int printing_index = 0;
    int tracking_index = 0;
    int last_time = millis();
    if(long_data){
        if(tracking_buffer_valid){
            PC.print("\tTRACKING BUFFER VALID!\n");
        }else{
            PC.print("\tTRACKING BUFFER NOT VALID!!!\n");
        }
        for (tracking_index = 0; tracking_index<TRACKING_BUFFER_SIZE; tracking_index++){
            PC.print((int) tracking_buffer[tracking_index]);
            if((tracking_index & 0xF) == 0xF){
                PC.print("\n");
            }else{
                PC.print("\t");
            }

            if(millis() - last_time >= 1000){
                keep_awake_msp();
                last_time = millis();
            }

        }
    }

    while(printing_index < input_buffer_index){
        PC.print(input_buffer[printing_index++]);
    }
}


void wait_until_character_and_discard(char character, unsigned int times = 1){
    unsigned int i = 0;
    for(i = 0; i < times; i++){
        while((char)MSP.read() != character);
    }
}

void get_operating_mode(void){
    char aux[30] = "";
    unsigned int index = 0;

    char byte_from_msp;

    index = 0;
    change_led_color(PROCESSING);
    MSP.write(">");

    wait_until_character_and_discard('\n');
    wait_until_character_and_discard('=');
    MSP.read();
    

    
    do{
        byte_from_msp = (char) MSP.read();
        if(byte_from_msp >= 0x20 && byte_from_msp < 0x7F){
            aux[index] = byte_from_msp;
            index++;
        }
    }while(byte_from_msp != '\n');

    if(aux[0] == 'I'){
        PC.write(OPERATING_MODE__INACTIVE);
    }else if (aux[0] == 'T'){
        PC.write(OPERATING_MODE__TEMPERATURE);
    }else if (aux[12] == 'O'){
        PC.write(OPERATING_MODE__WET_AND_DRY);
    }else{
        PC.write(OPERATING_MODE__BOTH);
    }
    PC.write('\n');
    PC.flush();

    // TODO: INVALID STATE!!!
    // TODO: mandar info a PC DESPUES de dejar MSP en DEBUG MENU ???

    MSP.write("<");
    wait_until_character_and_discard('*',25);

    change_led_color(CONNECTED);
}

void get_indexes(void){
    char aux[10] = "";
    unsigned int index = 0;
    char byte_from_msp;
    unsigned int i;
    change_led_color(PROCESSING);
    MSP.write(">");

    wait_until_character_and_discard('\n',9);
    wait_until_character_and_discard('=');
    wait_until_character_and_discard(' ');

    index = 0;
    do{
        byte_from_msp = (char) MSP.read();
        if(byte_from_msp >= 0x20 && byte_from_msp < 0x7F){
            aux[index] = byte_from_msp;
            index++;
            //PC.print(byte_from_msp);
        }
    }while(byte_from_msp != '\n');
    aux[index] = '\0';
    wait_until_character_and_discard('\n');
    wait_until_character_and_discard('=');
    wait_until_character_and_discard(' ');
    
    do{
        byte_from_msp = (char) MSP.read();
        if(byte_from_msp >= 0x20 && byte_from_msp < 0x7F){
            PC.print(byte_from_msp);
        }
    }while(byte_from_msp != '\n');
    PC.print('\n');
    PC.print(aux);
    PC.print('\n');
    PC.flush();

    MSP.write("<");
    wait_until_character_and_discard('*',25);
    
    change_led_color(CONNECTED);
}

void get_calibration_C(bool atC){
    char byte_from_msp;

    change_led_color(PROCESSING);
    MSP.write("G");
    wait_until_character_and_discard('*',25);
    MSP.write("0");
    wait_until_character_and_discard('\n');
    if(atC){
        wait_until_character_and_discard('\n',2);
    }
    wait_until_character_and_discard('=');
    wait_until_character_and_discard(' ');
    do{
        byte_from_msp = (char) MSP.read();
        if(byte_from_msp >= 0x20 && byte_from_msp < 0x7F){
            PC.print(byte_from_msp);
        }
        
    }while(byte_from_msp != '\n');
    PC.print('\n');
    PC.flush();

    wait_until_character_and_discard('*',25);
    
    change_led_color(CONNECTED);
}

void toggle_led(){
    char byte_from_msp;

    change_led_color(PROCESSING);
    MSP.write("L");
    wait_until_character_and_discard('\n');
    wait_until_character_and_discard(' ');
    do{
        byte_from_msp = (char) MSP.read();
        if(byte_from_msp >= 0x20 && byte_from_msp < 0x7F){
            PC.print(byte_from_msp);
        }
        
    }while(byte_from_msp != '\n');
    PC.print('\n');
    PC.flush();

    wait_until_character_and_discard('*',25);
    
    change_led_color(CONNECTED);
}

void get_helper_function(char option){
    char byte_from_msp;

    change_led_color(PROCESSING);
    MSP.write("G");
    wait_until_character_and_discard('*',25);
    MSP.write(option);
    wait_until_character_and_discard('\n');
    wait_until_character_and_discard('=');
    wait_until_character_and_discard(' ');
    do{
        byte_from_msp = (char) MSP.read();
        if(byte_from_msp >= 0x20 && byte_from_msp < 0x7F){
            PC.print(byte_from_msp);
        }
        
    }while(byte_from_msp != '\n');
    PC.print('\n');
    PC.flush();

    wait_until_character_and_discard('*',25);

    change_led_color(CONNECTED);
}

void set_helper_function(char option){
    char input_from_base = '0';
    change_led_color(PROCESSING);
    MSP.write("S");
    wait_until_character_and_discard('*',25);
    do{
        MSP.write((char) input_from_base);
        input_from_base = PC.read();
    }while(input_from_base != '\n');

    while((char) MSP.read() != '*');
    MSP.write(">");
    delay(100);
    while((char) MSP.read() != '*');
    
    while(1){
        //WIP
    }

}
