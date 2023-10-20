#ifndef __commands_for_msp_h__
#define __commands_for_msp_h__

#define PC  Serial
#define MSP Serial2
#define sent_bytes  available

#define at30C 0
#define at105C 1

void get_operating_mode(void);
void get_indexes(void);
void get_calibration_C(bool atC);
void toggle_led();
void get_helper_function(char option);
void set_helper_function(char option);


#endif