#include "commands_from_pc.h"
#include "commands_for_msp.h"
#include "led_functions.h"
#include "test.h"

#define ENABLE_5V     digitalWrite(15,LOW)
#define DISABLE_5V    digitalWrite(15,HIGH)

// TODO: Agregar pin para TX enable!


void setup() {
    pinMode(15,OUTPUT);
    DISABLE_5V;
    setup_led();

    PC.begin(115200);
    MSP.begin(115200, SERIAL_8O2);

    change_led_color(BASE_POWER_ON);

    //test_millis();
}

void loop(){
    char byte_from_msp = '\0';
    char byte_from_pc = '\0';
    int last_time;
    bool quit = false;

    while(PC.sent_bytes() <= 0); // Se queda acá hasta que reciba datos. Se podría hacer algo en el medio? O hacer que este en "sleep"?
    if(PC.read() == CONNECT_TO_MSP){
        change_led_color(CONNECTING);
        ENABLE_5V;
        //discard_data_from_msp();
        delay(1000);
        MSP.write("1234");
        delay(500);
        //discard_data_from_msp();
        last_time = millis();
        change_led_color(CONNECTED); // TODO: CHECKEAR QUE ESTÉ CONECTADO!!! HACERLO BIEN!!! CHANGOS!!
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
                if (byte_from_pc == 'F'){
                    send_and_print_received((char)byte_from_pc, true, 3600000);
                }else{
                    send_and_print_received((char)byte_from_pc);
                }
                last_time = millis();
                

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
            }
            if (MSP.available()){
                PC.print("REMAINING = ");
                PC.print(MSP.available());
                PC.write('\n');
                byte_from_msp = MSP.read();
            }

        }
        DISABLE_5V;
    }
}


