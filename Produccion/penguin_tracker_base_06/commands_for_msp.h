#ifndef __commands_for_msp_h__
#define __commands_for_msp_h__

#define PC  Serial
#define MSP Serial2
#define sent_bytes  available

// #define VERBOSE

#define at30C 0
#define at105C 1

void send_and_print_received(char sending, bool long_data = false, int time_out = 2000);
void keep_awake_msp(void);
int get_current_menu(void);

enum current_menu{
    UNKNOWN_MENU,
    DEBUG_MENU, // "\nDEBUG MENU\n"
    GET_MENU,   // "\nGETTING...\n"
    SET_MENU,   // "\nSETTING...\n\n0"
    SET_MENU2,  // "\nSETTING...\n\n1"
    NAME_MENU,  // "\nCHANGING NAME...\n"
    USER_MENU,  // "\nUSER MENU\n"
    PASSWORD_SCREEN, // "\nENTER PASSWORD\n"
    DATE_MENU,       // "\nINSERT DATE\n"
    NEW_STATE_MENU   // "\nSELECT NEW STATE AND QUIT\n"
};


// send_command_to_msp
//      FUNCTION:
//  Sends single char to MSP via UART
//      INPUT:
//  command: char to send
//      OUTPUT:
//  Nothing
// NOTE: to receive message into input_buffer, use receive_answer_from_msp
void send_command_to_msp(char command);

// receive_answer_from_msp
//      FUNCTION:
//  Loads into input_buffer the message sent from the MSP
//      INPUT:
//  time_out: msecs since the start of reception before time_out
//  long_data: if true, first loads bytes into tracking_buffer
//      OUTPUT:
//  0 = data not valid
//  1 = data valid, message received
int receive_answer_from_msp(int time_out = 2000, bool long_data = false);  // TODO: separar entre "tiempo hasta terminar mensaje" y "tiempo sin recivir nada". No olvida posibilidad de ruido!!!
int receive_last_message(int time_out = 2000);

//  print_input_buffer
//      FUNCTION:
//  Sends input_buffer to PC via Serial
//      INPUT:
//  long_data: if true, also sends tracking_buffer
//      OUTPUT:
//  Nothing  
void print_input_buffer(bool long_data = false);
//      OUTPUT:
//  tracking_buffer_valid (0=bad, 1=good)
bool print_fast_data(void);

//      OUTPUT:
//  Pointer to answer string. If error, output = NULL
const char* get_helper(char option, int variable = 1);
const char* get_from_user_menu_helper(int variable1, int variable2 = 0);
const char* toggle_led(void);
//      OUTPUT:
//  0 = error; 1 = valid
bool set_helper(int menu, char command, const char* input);
bool get_last_events(void);
bool quit_helper(char operating_mode, const char* activation_time);
bool get_fast_data(void);

#endif