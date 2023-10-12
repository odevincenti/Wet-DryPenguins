#include "commands_from_pc.h"
#include "commands_for_msp.h"
#include "led_functions.h"

#define ENABLE_5V   digitalWrite(15,LOW)
#define DISABLE_5V  digitalWrite(15,HIGH)

#define sent_bytes  available

void setup() {
  pinMode(15,OUTPUT);
  DISABLE_5V;
  setup_led();

  PC.begin(115200);
  MSP.begin(115200, SERIAL_8O2);

  change_led_color(BASE_POWER_ON);
}

void loop(){
  char byte_from_msp = '\0';
  char byte_from_pc = '\0';
  int last_time;
  bool quit = false;

  while(PC.sent_bytes() <= 0);
  if(PC.read() == CONNECT_TO_MSP){
    change_led_color(CONNECTING);
    ENABLE_5V;
    //discard_data_from_msp();
    delay(1000);
    MSP.write("1234");
    delay(500);
    //discard_data_from_msp();
    last_time = millis();
    change_led_color(CONNECTED);
    while(!quit){
      if(millis() - last_time >= 1000){
        MSP.write("\n");
        last_time = millis();
      }

      if(PC.available()){
        byte_from_pc = PC.read();

        switch((char) byte_from_pc){
        case GET_FSM_STATE:         // F
          PC.print("Y\n");
          break;
        case GET_WET_AND_DRY_FREC:  // W
          get_helper_function('4');
          last_time = millis();
          break;
        case GET_TEMPERATURE_FREC:  // T
          get_helper_function('5');
          last_time = millis();
          break;
        case GET_CALIBRATED_MINUTE: // C
          get_helper_function('6');
          last_time = millis();
          break;
        case GET_NAME:              // N
          get_helper_function('7');
          last_time = millis();
          break;
        case GET_LOGGER_ID:         // I
          get_helper_function('8');
          last_time = millis();
          break;
        case TOGGLE_LED:            // L
          toggle_led();
          last_time = millis();
          break;
        case GET_ACTIVATION_TIME:   // A
          get_helper_function('9');
          last_time = millis();
          break;
        case GET_AT_30C:            // 3
          get_calibration_C(at30C);
          last_time = millis();
          break;
        case GET_AT_105C:           // 1
          get_calibration_C(at105C);
          last_time = millis();
          break;  
        case GET_ALL_INDEX:         // X
          get_indexes();
          last_time = millis();
          break;
        case GET_OPERATING_MODE:    //M
          get_operating_mode();
          last_time = millis();
          break;

        default:
          break;
        }

        
      }
      if (MSP.available()){
        byte_from_msp = MSP.read();
      }

    }
    DISABLE_5V;
  }
}


