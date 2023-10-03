enum COLOR {BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE};
void change_led_color(int color){
  digitalWrite(25, color & 0x1);
  digitalWrite(26, (color & 0x2) );
  digitalWrite(27, (color & 0x4) );
}
void setup_led(void){
  pinMode(25, OUTPUT);
  pinMode(26, OUTPUT);
  pinMode(27, OUTPUT);
  digitalWrite(25, LOW);
  digitalWrite(26, LOW);
  digitalWrite(27, LOW);
}

#define ENABLE_5V   digitalWrite(15,LOW)//digitalWrite(26, HIGH);//
#define DISABLE_5V  digitalWrite(15,HIGH)//digitalWrite(26, LOW);//

#define PC  Serial
#define MSP Serial2

#define sent_bytes  available

#define CONNECT_TO_MSP  'S'
#define GET_FSM_STATE   'F'
#define GET_LOGGER_ID   'I'
#define GET_NAME        'N'
#define SET_NAME        'n'
#define GET_WET_AND_DRY_FREC  'W'
#define SET_WET_AND_DRY_FREC  'w'
#define GET_TEMPERATURE_FREC  'T'
#define SET_TEMPERATURE_FREC  't'
#define GET_OPERATING_MODE  'M'
#define SET_OPRETAING_MODE  'm'
#define SET_OPERATING_MODE__INACTIVE    '0'
#define SET_OPERATING_MODE__WET_AND_DRY '1'
#define SET_OPERATING_MODE__TEMPERATURE '2'
#define SET_OPERATING_MODE__BOTH        '3'
#define GET_CALIBRATED_MINUTE 'C'
#define SET_CALIBRATED_MINUTE 'c'
#define GET_ACTIVATION_TIME   'A'
#define SET_ACTIVATION_TIME   'a'
#define GET_AT_30C            '3'
#define GET_AT_105C           '1'
#define ACTIVATE_LOGGER       'Q'
#define DEACTIVATE_LOGGER     'q'
#define TOGGLE_LED            'L'
#define GET_DATA_FAST         'D'
#define GET_EVENTS            'E'
#define GET_ALL_INDEX         'X'

//13AaDECcFILMmNnQqSTtWwX


void setup() {
  // put your setup code here, to run once:
  pinMode(15,OUTPUT);
  DISABLE_5V;
  setup_led();

  PC.begin(115200);
  MSP.begin(115200, SERIAL_8O2);

}


void discard_data_from_msp(void){
  do{
    while(MSP.available()){
      MSP.read();
    }
    delay(100);
  }while(MSP.available());
}



void loop(){
  //static bool timeout = true;
  static bool blue_led_state = false;
  char byte_from_msp = '\0';
  char byte_from_pc = '\0';
  int last_time;
  bool quit = false;


  while(PC.sent_bytes() <= 0);
  if(PC.read() == CONNECT_TO_MSP){
    change_led_color(GREEN);
    ENABLE_5V;
    discard_data_from_msp();
    delay(1000);
    MSP.write("1234");
    discard_data_from_msp();
    last_time = millis();
    while(!quit){
      if(millis() - last_time >= 1000){
        MSP.write("\n");
        last_time = millis();
        blue_led_state = !blue_led_state;
        if(blue_led_state){
          change_led_color(CYAN);
        }else{
          change_led_color(GREEN);
        }
        //change_led_color(CYAN);
        //PC.write("\nKeeping MSP connected...");
        //change_led_color(GREEN);
        PC.flush();
      }

      if(PC.available()){
        byte_from_pc = PC.read();
        //PC.write("\nREAD FROM PC = ");
        //PC.write((char) byte_from_pc);
        //PC.flush();
        //PC.write("\n");
        //MSP.write((char) byte_from_pc);
        //MSP.flush();

        switch((char) byte_from_pc){
          case GET_WET_AND_DRY_FREC:
            change_led_color(BLUE);
            MSP.write("G");
            discard_data_from_msp();
            MSP.write("4");
            MSP.flush();
            while((char)MSP.read() != '\n');
            while((char)MSP.read() != '=');
            while((char)MSP.read() != ' ');
            do{
              byte_from_msp = (char) MSP.read();
              if(byte_from_msp >= 0x20 && byte_from_msp < 0x7F){
                PC.print(byte_from_msp);
              }
              
            }while(byte_from_msp != '\n');
            PC.print('\n');
            discard_data_from_msp();
            PC.flush();
            change_led_color(GREEN);
          break;

          default:
          break;
        }

        last_time = millis();
      }
      if (MSP.available()){
        byte_from_msp = MSP.read();
        PC.write((char) byte_from_msp);
        PC.flush();
      }

    }
    DISABLE_5V;
  }
}


