#include "commands_for_msp.h"

#define INPUT_BUFFER_SIZE 5000
#define TRACKING_BUFFER_SIZE 28000
#define ASTERISK_STRING_SIZE 25

static char input_buffer[INPUT_BUFFER_SIZE] = "";
static int input_buffer_index = 0;
static int input_buffer_time = 0;

static unsigned char tracking_buffer[TRACKING_BUFFER_SIZE] = "";
static bool tracking_buffer_valid = false;
static char tracking_single_buffer[4] = "";

static char const start_of_tracking_data_string[3] = {0x06,0xA,0xA};
static char const ending_message[15] = "\x06\nBye bye!\nOK\n";

static void try_to_go_back(void){
    bool received = false;
    send_command_to_msp('<');
    received = receive_answer_from_msp();
    send_command_to_msp('<');
    received = receive_answer_from_msp();
    send_command_to_msp('<');
    received = receive_answer_from_msp();
}

const char password[] = "Noot noot!";

bool get_fast_data(void){
    bool received = false;

    
    if (get_current_menu() != DEBUG_MENU){
        PC.print("ERROR 1");
        try_to_go_back();
        return false;
    }
    send_command_to_msp('>');
    received = receive_answer_from_msp();      
    if (!received && get_current_menu() != USER_MENU){
        PC.print("ERROR 2");
        try_to_go_back();
        return false;
    }    
    send_command_to_msp('F');
    received = receive_answer_from_msp(3600000,true);
    if (!received && get_current_menu() != USER_MENU){
        PC.print("ERROR 3");
        try_to_go_back();
        return false;
    }

    if(!tracking_buffer_valid){
        PC.print("ERROR 4");
        try_to_go_back();
        return false;
    }
    received = print_fast_data();
    if(!received  && get_current_menu() != USER_MENU){
        PC.print("ERROR 5");
        try_to_go_back();
        return false;
    }    
    
    send_command_to_msp('<');
    received = receive_answer_from_msp();
    if (!received  &&  get_current_menu() != DEBUG_MENU){
        PC.print("ERROR 6");
        try_to_go_back();
        return false;
    }     
    return true;

}

bool quit_helper(char operating_mode, const char* activation_time){
    bool received = false;
    int i = 0;
    const char* atime_index = activation_time;

    //PC.print("QUIT HELPER WITH operating_mode = ");
    //PC.write(operating_mode);
    //PC.print(" AND ACTIVATION TIME = ");
    //PC.print(activation_time);
    //PC.print("\n");

    if (get_current_menu() != DEBUG_MENU){
        PC.print("ERROR 1");
        try_to_go_back();
        return false;
    }
    send_command_to_msp('>');
    received = receive_answer_from_msp();      
    if (!received && get_current_menu() != USER_MENU){
        PC.print("ERROR 2");
        try_to_go_back();
        return false;
    }
    send_command_to_msp('>');
    received = receive_answer_from_msp();
    if (!received && get_current_menu() != PASSWORD_SCREEN){
        PC.print("ERROR 3");
        try_to_go_back();
        return false;
    }

    while(password[i] != '!'){
        send_command_to_msp(password[i]);
        received = receive_answer_from_msp();
        if (!received && get_current_menu() != PASSWORD_SCREEN){
            PC.print("ERROR 4");
            try_to_go_back();
            return false;
        }   
        i++;
    }
    send_command_to_msp(password[i]);
    received = receive_answer_from_msp();
    if (!received && get_current_menu() != DATE_MENU){
        PC.print("ERROR 5");
        try_to_go_back();
        return false;
    } 

    while(*atime_index != '\0'){
        send_command_to_msp(*(atime_index++));
        received = receive_answer_from_msp();
        if (!received && get_current_menu() != DATE_MENU){
            PC.print("ERROR 6");
            try_to_go_back();
            return false;
        }
    }
    send_command_to_msp('>');
    received = receive_answer_from_msp();
    if (!received && get_current_menu() != NEW_STATE_MENU){
        PC.print("ERROR 7");
        try_to_go_back();
        return false;
    }


    send_command_to_msp((char)(operating_mode+1));
    received = receive_last_message();
    if(!received){
        PC.print("ERROR 8");
        try_to_go_back();
        return false;        
    }

    return true;


}



const char* toggle_led(void){
    static char answer[4] = "";
    const char* original_position = 0;
    bool received = false;
    int answer_index = 0;

    if (get_current_menu() != DEBUG_MENU){
        //PC.print("ERROR D\n");
        try_to_go_back();
        return NULL;
    }
    send_command_to_msp('L');
    received = receive_answer_from_msp();    
    if(! received || get_current_menu() != DEBUG_MENU){
        try_to_go_back();
        return NULL; 
    }
    original_position = input_buffer;
    original_position = strstr(original_position,"LED");
    if(original_position == NULL || original_position> ((const char*)input_buffer+INPUT_BUFFER_SIZE-8)){
        try_to_go_back();
        //PC.print("ERROR C\n");
        return NULL;
    }
    original_position = original_position+4;

    
    while(*original_position != '\0' && *original_position != '\n' && answer_index < 3 && original_position < ((const char*)input_buffer+INPUT_BUFFER_SIZE)){
        answer[answer_index++] = *original_position;
        original_position++;
    }

    answer[answer_index] = 0;
    answer[3] = 0;
    return answer;

}

int char_pointer_content_to_int(const char* pointer){
    int answer = 0;
    bool negative = false;
    const char* current_pointer = pointer;

    if(*pointer == '-'){
        negative = true;
        pointer++;
    }else if (*pointer == '+'){
        negative = false;
        pointer++;
    }
    while(*pointer >= '0' && *pointer <= '9'){
        answer *= 10;
        answer += *pointer-'0';
        pointer++;
    }
    if (negative){
        answer *= -1;
    }
    return answer;
}

bool get_last_events(void){
    bool received = false;

    const char* last_event_index_pointer = NULL;
    int last_event_index = 0;
    const char* looping_events_value_pointer = NULL;
    bool looping_events = false;
    
    const char* last_event_pointer = NULL;
    const char* beggining_of_list_pointer = NULL;
    const char* ending_of_list_pointer = NULL;
    const char* printing_pointer = NULL;
    int i = 0;

    if (get_current_menu() != DEBUG_MENU){
        try_to_go_back();
        return false;
    }
    send_command_to_msp('>');
    received = receive_answer_from_msp();
    if (get_current_menu() != USER_MENU){
        try_to_go_back();
        return false;
    }    
    send_command_to_msp('E');
    received = receive_answer_from_msp();    
    if (get_current_menu() != USER_MENU){
        try_to_go_back();
        return false;
    }    
    // PRINTEAR LO QUE ME IMPORTE!!!

    last_event_index_pointer = strstr(input_buffer, "=");
    if(last_event_index_pointer == NULL){
        try_to_go_back();
        return false;    
    }
    last_event_index_pointer = last_event_index_pointer+2;

    looping_events_value_pointer = strstr(last_event_index_pointer, "=");
    if(looping_events_value_pointer == NULL){
        try_to_go_back();
        return false;    
    }
    looping_events_value_pointer = looping_events_value_pointer+2;

    beggining_of_list_pointer = strstr(looping_events_value_pointer,"\n");
    if(beggining_of_list_pointer == NULL){
        try_to_go_back();
        return false;    
    }    
    beggining_of_list_pointer = beggining_of_list_pointer+1;

    ending_of_list_pointer = strstr(beggining_of_list_pointer,"\n\n");
    if(ending_of_list_pointer == NULL){
        try_to_go_back();
        return false;    
    }    
    ending_of_list_pointer = ending_of_list_pointer+1;

    // GET LAST_EVENT_INDEX!!!
    last_event_index = char_pointer_content_to_int(last_event_index_pointer);
    last_event_pointer = beggining_of_list_pointer;
    for(i=0; i<last_event_index && i<99;i++){
        last_event_pointer = strstr(last_event_pointer,"\n");
        if(last_event_pointer == NULL){
            try_to_go_back();
            return false;    
        }
        last_event_pointer = last_event_pointer+1;
    }

    looping_events = *looping_events_value_pointer == 'T';
    
    //PC.print("LAST EVENT INDEX = ");
    //PC.print(last_event_index);
    //PC.print("\nLOOPING = ");
    //PC.print((int)looping_events);
    //PC.print("\n");

    if(!looping_events){
        for(printing_pointer = beggining_of_list_pointer; printing_pointer<last_event_pointer; printing_pointer++){
            PC.write(*printing_pointer);
        }
    }else{
        printing_pointer = last_event_pointer;
        do{
            PC.write(*printing_pointer);
            printing_pointer++;
            if(printing_pointer == ending_of_list_pointer){
                printing_pointer = beggining_of_list_pointer;
            }
        }while(printing_pointer != last_event_pointer);
        
    }




    // OJO! SI OCURRE ERROR LUEGO DE ESO, IGUAL PC RECIBE EVENTOS!!!
    send_command_to_msp('<');
    received = receive_answer_from_msp();
    if (get_current_menu() != DEBUG_MENU){
        try_to_go_back();
        return false;
    }     
    return true;
}

bool set_helper(int menu, char command, const char* input){
    bool received = false;
    int i = 0;

    if (get_current_menu() != DEBUG_MENU){
        try_to_go_back();
        return false;
    }

    if(menu == DEBUG_MENU){
        send_command_to_msp(command);
        received = receive_answer_from_msp();
        if(!received){
            try_to_go_back();
            return false;
        }
        while(received && i<255 && input[i] != '\0'){
            send_command_to_msp(input[i++]);
            received = receive_answer_from_msp();
        }
        if(!received){
            try_to_go_back();
            return false;
        }
        send_command_to_msp('>');
        received = receive_answer_from_msp();
        if(!received || get_current_menu() != DEBUG_MENU){
            try_to_go_back();
            return false;
        }
        return true;
    }else if(menu == SET_MENU){
        send_command_to_msp('S');
        received = receive_answer_from_msp();
        if(!received || get_current_menu() != SET_MENU){
            try_to_go_back();
            return false;
        }
        while(received && i<255 && input[i] != '\0'){
            send_command_to_msp(input[i++]);
            received = receive_answer_from_msp();
        }
        if(!received){
            try_to_go_back();
            return false;
        }
        send_command_to_msp('>');
        received = receive_answer_from_msp();        
        if(!received || get_current_menu() != SET_MENU2){
            try_to_go_back();
            return false;
        }
        send_command_to_msp(command);
        received = receive_answer_from_msp();
        if(!received || get_current_menu() != DEBUG_MENU){
            try_to_go_back();
            return false;
        }
        return true;
    }
    return false;
}

const char* get_from_user_menu_helper(int variable1, int variable2){
    static char answer[256] = "";
    const char* original_position = 0;
    const char* first_variable_pointer = NULL;
    const char* second_variable_pointer = NULL;
    bool received = false;
    int answer_index = 0;
    int i;

    if (get_current_menu() != DEBUG_MENU){
        try_to_go_back();
        return NULL;
    }
    send_command_to_msp('>');
    received = receive_answer_from_msp();
    if(! received || get_current_menu() != USER_MENU){
        try_to_go_back();
        return NULL; 
    }   
    original_position = input_buffer;
    for (i=0; i<variable1 || i<variable2;i++){
        original_position = strstr(original_position," = ");
        if(original_position == NULL || original_position> ((const char*)input_buffer+INPUT_BUFFER_SIZE-4)){
            try_to_go_back();
            return NULL;
        }
        original_position = original_position + 3;
        if(i == variable1-1){
            first_variable_pointer = original_position;
        }
        if(i == variable2-1){
            second_variable_pointer = original_position;
        }
    }

    if(first_variable_pointer == NULL || (variable2 != 0 && second_variable_pointer ==NULL)){
        try_to_go_back();
        return NULL;
    }

    while(*first_variable_pointer != '\0' && *first_variable_pointer != '\n' && answer_index < 255 && first_variable_pointer < ((const char*)input_buffer+INPUT_BUFFER_SIZE)){
        answer[answer_index++] = *first_variable_pointer;
        first_variable_pointer++;
    }
    if(variable2 != 0){
        answer[answer_index++] = '\t';
        while(*second_variable_pointer != '\0' && *second_variable_pointer != '\n' && answer_index < 255 && second_variable_pointer < ((const char*)input_buffer+INPUT_BUFFER_SIZE)){
            answer[answer_index++] = *second_variable_pointer;
            second_variable_pointer++;
        }
    }
    answer[answer_index] = 0;
    answer[255] = 0;

    send_command_to_msp('<');
    received = receive_answer_from_msp();
    if(! received || get_current_menu() != DEBUG_MENU){
        try_to_go_back();
        return NULL; 
    }   

    return answer;
}

const char* get_helper(char option, int variable){
    static char answer[256] = "";
    const char* original_position = 0;
    bool received = false;
    int answer_index = 0;
    int i;
    
    if (get_current_menu() != DEBUG_MENU){
        //PC.print("ERROR D\n");
        try_to_go_back();
        return NULL;
    }
    send_command_to_msp('G');
    received = receive_answer_from_msp();
    if(! received || get_current_menu() != GET_MENU){
        try_to_go_back();
        return NULL; 
    }
    send_command_to_msp(option);
    received = receive_answer_from_msp();
    if(! received || get_current_menu() != DEBUG_MENU){
        try_to_go_back();
        //PC.print("ERROR B\n");
        return NULL;
        
    }
    original_position = input_buffer;
    for (i=0; i<variable;i++){
        original_position = strstr(original_position,"=");
        if(original_position == NULL || original_position> ((const char*)input_buffer+INPUT_BUFFER_SIZE-4)){
            try_to_go_back();
            //PC.print("ERROR C\n");
            return NULL;
        }
        original_position = original_position + 2;
    }
    while(*original_position != '\0' && *original_position != '\n' && answer_index < 255 && original_position < ((const char*)input_buffer+INPUT_BUFFER_SIZE)){
        answer[answer_index++] = *original_position;
        original_position++;
    }
    answer[answer_index] = 0;
    answer[255] = 0;
    return answer;
}


// NOTE: use receive_answer_from_msp at least once before
int get_current_menu(void){
    //print_input_buffer();
    if (strstr(input_buffer,"\nDEBUG MENU\n") != NULL)
        return DEBUG_MENU;
    if (strstr(input_buffer,"\nGETTING...\n") != NULL)
        return GET_MENU;
    if (strstr(input_buffer,"\nSETTING...\n\n0") != NULL)
        return SET_MENU;    
    if (strstr(input_buffer,"\nSETTING...\n\n1") != NULL)
        return SET_MENU2;   
    if (strstr(input_buffer,"\nCHANGING NAME...\n") != NULL)
        return NAME_MENU;      
    if (strstr(input_buffer,"\nUSER MENU\n") != NULL)
        return USER_MENU;   
    if (strstr(input_buffer,"\nENTER PASSWORD\n") != NULL)
        return PASSWORD_SCREEN;   
    if (strstr(input_buffer,"\nINSERT DATE\n") != NULL)
        return DATE_MENU;
    if (strstr(input_buffer,"\nSELECT NEW STATE AND QUIT\n") != NULL)
        return NEW_STATE_MENU;
    return UNKNOWN_MENU;

}



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

void send_command_to_msp(char command){
    MSP.print(command);
}

int receive_last_message(int time_out){
    int starting_time = millis();    
    int time_out_time = starting_time + time_out;
    //bool message_received = false;
    int input_from_MSP = 0;
    bool valid_message = true;

    input_buffer_index = 0;
    while(valid_message && millis() < time_out_time && input_buffer_index < INPUT_BUFFER_SIZE-1 && input_buffer_index < sizeof(ending_message)/sizeof(ending_message[0])-1){
        if(MSP.sent_bytes()){
            input_from_MSP = MSP.read();
            if(input_from_MSP != ending_message[input_buffer_index]){
                valid_message = false;
            }
            if(input_from_MSP != '\0'){
                input_buffer[input_buffer_index++] = (char) input_from_MSP;
            }
        }   
    }
    if(input_buffer_index < INPUT_BUFFER_SIZE){
        input_buffer[input_buffer_index] = 0;
    }
    input_buffer[INPUT_BUFFER_SIZE-1] = 0;
    

    input_buffer_time = millis()-starting_time;
    return valid_message &&  (input_buffer_index == sizeof(ending_message)/sizeof(ending_message[0])-1);

}

int receive_answer_from_msp(int time_out, bool long_data){
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

    while(!message_received && millis() < time_out_time && input_buffer_index < INPUT_BUFFER_SIZE-1){
        if(MSP.sent_bytes()){
            input_from_MSP  = MSP.read();
            if(input_from_MSP != '\0'){
                input_buffer[input_buffer_index++] = (char) input_from_MSP;
            }
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
    if(input_buffer_index < INPUT_BUFFER_SIZE){
        input_buffer[input_buffer_index] = 0;
    }
    input_buffer[INPUT_BUFFER_SIZE-1] = 0;


    input_buffer_time = millis()-starting_time;
    return message_received && (tracking_buffer_valid || !long_data);
}



bool print_fast_data(void){
    int tracking_index = 0;
    int last_time = millis();

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
    return tracking_buffer_valid;

}

void print_input_buffer(bool long_data){
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
        if(input_buffer[printing_index]<0x20 && input_buffer[printing_index]!= '\n' && input_buffer[printing_index]!= '\t'){
            PC.print("0x");
            PC.print((int)input_buffer[printing_index]);
        }else{
            PC.print(input_buffer[printing_index]);
        }
        printing_index++;
    }
}
