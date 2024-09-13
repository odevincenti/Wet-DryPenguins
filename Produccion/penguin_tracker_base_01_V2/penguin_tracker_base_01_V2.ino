enum COLOR {BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE};
void change_led_color(int color){
  digitalWrite(33, color & 0x1);
  digitalWrite(25, (color & 0x2) );
  digitalWrite(32, (color & 0x4) );
}
void setup_led(void){
  pinMode(33, OUTPUT);
  pinMode(25, OUTPUT);
  pinMode(32, OUTPUT);
  digitalWrite(33, LOW);
  digitalWrite(25, LOW);
  digitalWrite(32, LOW);
}

#define ENABLE_5V   digitalWrite(15,LOW)//digitalWrite(26, HIGH);//
#define DISABLE_5V  digitalWrite(15,HIGH)//digitalWrite(26, LOW);//

#define ENABLE_TX   digitalWrite(18,LOW)
#define DISABLE_TX  digitalWrite(18,HIGH)

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
  pinMode(18,OUTPUT);
  DISABLE_5V;
  DISABLE_TX;
  setup_led();

  PC.begin(115200);
  MSP.begin(115200, SERIAL_8O2, 16, 17);

}


void discard_data_from_msp(void){
  do{
    while(MSP.available()){
      MSP.read();
    }
    delay(100);
  } while(MSP.available());
}


void loop(){
  static bool color = false;
  //static bool timeout = true;
  char byte_from_msp = '\0';
  char byte_from_pc = '\0';
  int last_time;
  bool quit = false;

  while(PC.sent_bytes() <= 0);
  if(PC.read() == CONNECT_TO_MSP){
    change_led_color(RED);
    ENABLE_5V;
    ENABLE_TX;
    //discard_data_from_msp();
    delay(300);
    MSP.write("1");
    //delay(200);
    MSP.write("2");
    //delay(200);
    MSP.write("3");
    //delay(200);
    MSP.write("4");
    //delay(200);
    //discard_data_from_msp();
    last_time = millis();


    while(!quit){

      // If timeout
      if(millis() - last_time >= 1000){
        MSP.write("\n");
        last_time = millis();
        color = !color;
        if (color){
          change_led_color(GREEN);
        }else{
          change_led_color(CYAN);
        }
        //PC.write("\nKeeping MSP connected...");

        PC.flush();
      }

      // If PC Command
      if(PC.available()){
        byte_from_pc = PC.read();
        // PC.write("\nREAD FROM PC = ");
        // PC.write((char) byte_from_pc);
        MSP.write((char) byte_from_pc);
        MSP.flush();
        last_time = millis();
      }

      // If MSP answer
      if (MSP.available()){
        byte_from_msp = MSP.read();
        PC.write((char) byte_from_msp);
        if(byte_from_msp == '\x00'){
            PC.write("[NUL]");
        }else if(byte_from_msp == '\x15'){
            PC.write("[NAK]");
        }else if(byte_from_msp == '\x06'){
            PC.write("[ACK]");
        }
        PC.flush();
      }

    }
    DISABLE_TX;
    DISABLE_5V;
  }
}


