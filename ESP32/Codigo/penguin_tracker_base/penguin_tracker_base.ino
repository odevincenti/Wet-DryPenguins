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

#define ENABLE_5V   digitalWrite(15,LOW)
#define DISABLE_5V  digitalWrite(15,HIGH)

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

void msp_communication(void){
  bool connected = true;
  int last_time = millis();
  change_led_color(GREEN);
  do{
    if(millis() - last_time >= 1000){
      MSP.print("\n");
      last_time = millis();
    }

    if(PC.read() == 'S'){
      connected = false;
    }

  }while(connected);
  change_led_color(BLACK);
}

void loop(){
  //static bool connected_to_msp = false;
  //static char byte_from_pc = '\0';
  //static char byte_from_msp = '\0';

  while(PC.sent_bytes() <= 0);
  //byte_from_pc = PC.read();
  if(PC.read() == CONNECT_TO_MSP){
    msp_communication();
  }
}

/*void loop() {
  static char byte_from_msp = '\0';
  // put your main code here, to run repeatedly:
  if(MSP.sent_bytes() > 0){
    byte_from_msp = MSP.read();
    MSP.print("RECEIVED = ");
    MSP.print(byte_from_msp);
    MSP.print('\n');
  }
}*/
