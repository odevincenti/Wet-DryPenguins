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
#define GET_FSM_STATE   'F' // Yes
#define GET_LOGGER_ID   'I' //
#define GET_NAME        'N' //
#define SET_NAME        'n'
#define GET_WET_AND_DRY_FREC  'W' //
#define SET_WET_AND_DRY_FREC  'w'
#define GET_TEMPERATURE_FREC  'T' //
#define SET_TEMPERATURE_FREC  't'
#define GET_OPERATING_MODE  'M'   //
#define SET_OPRETAING_MODE  'm'
#define OPERATING_MODE__INACTIVE    '0'
#define OPERATING_MODE__WET_AND_DRY '1'
#define OPERATING_MODE__TEMPERATURE '2'
#define OPERATING_MODE__BOTH        '3'
#define GET_CALIBRATED_MINUTE 'C' //
#define SET_CALIBRATED_MINUTE 'c'
#define GET_ACTIVATION_TIME   'A' //
#define SET_ACTIVATION_TIME   'a'
#define GET_AT_30C            '3' //
#define GET_AT_105C           '1' //
#define ACTIVATE_LOGGER       'Q'
#define DEACTIVATE_LOGGER     'q'
#define TOGGLE_LED            'L' //
#define GET_DATA_FAST         'D' 
#define GET_EVENTS            'E' 
#define GET_ALL_INDEX         'X' // 

//13AaDECcFILMmNnQqSTtWwX


void setup() {
  // put your setup code here, to run once:
  pinMode(15,OUTPUT);
  DISABLE_5V;
  setup_led();

  PC.begin(115200);
  MSP.begin(115200, SERIAL_8O2);

}


/*void discard_data_from_msp(void){
  do{
    while(MSP.available()){
      MSP.read();
    }
    delay(10);
  }while(MSP.available());
}*/
bool blue_led_state = false;

void get_operating_mode(void){
  char aux[30] = "";
  unsigned int index = 0;

  char byte_from_msp;

  index = 0;
  change_led_color(BLUE);
  MSP.write(">");

  while((char)MSP.peek() != '\n')  {MSP.read();}
  while((char)MSP.peek() != '=')  {MSP.read();}
  MSP.read();
  MSP.read();
  
  // while((char)MSP.read() != '=');
  // while((char)MSP.read() != ' ');
  

  
  do{
    byte_from_msp = (char) MSP.read();
    if(byte_from_msp >= 0x20 && byte_from_msp < 0x7F){
      aux[index] = byte_from_msp;
      index++;
    }
  }while(byte_from_msp != '\n');
  //PC.print('\n');
  // PC.print(aux);
  // PC.print('\n');
  // if(aux == "INACTIVE"){
  //   PC.write("Oh yeah\n");
  //   //change_led_color(RED);
  //   while(1);
  // }else{
  //   PC.write("Oh no!\n");
  //   //change_led_color(YELLOW);
  //   while(1);
  // }
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

  MSP.write("<");
  while(MSP.available()) MSP.read(); //discard_data_from_msp();
  PC.flush();
  if(blue_led_state){
    change_led_color(CYAN);
  }else{
    change_led_color(GREEN);
  }
}

void get_indexes(void){
  char aux[10] = "";
  unsigned int index = 0;
  char byte_from_msp;
  unsigned int i;
  change_led_color(BLUE);
  MSP.write(">");

  for (i=0;i<9;i++){
    while((char)MSP.read() != '\n');
  }
  while((char)MSP.read() != '=');
  while((char)MSP.read() != ' ');

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
  PC.print(aux);
  PC.print('\n');

  MSP.write("<");
  while(MSP.available()) MSP.read(); //discard_data_from_msp();
  PC.flush();
  if(blue_led_state){
    change_led_color(CYAN);
  }else{
    change_led_color(GREEN);
  }
}

#define at30C 0
#define at105C 1
void get_calibration_C(bool atC){
  char byte_from_msp;

  change_led_color(BLUE);
  MSP.write("G");
  while((char) MSP.read() != '*');
  MSP.write("0");
  //while((char) MSP.read() == '*');
  while((char)MSP.read() != '\n');
  if(atC){
    while((char)MSP.read() != '\n');
    while((char)MSP.read() != '\n');
  }
  while((char)MSP.read() != '=');
  while((char)MSP.read() != ' ');
  do{
    byte_from_msp = (char) MSP.read();
    if(byte_from_msp >= 0x20 && byte_from_msp < 0x7F){
      PC.print(byte_from_msp);
    }
    
  }while(byte_from_msp != '\n');
  PC.print('\n');
  //while((char) MSP.read() != '*');
  while(MSP.available()) MSP.read(); //discard_data_from_msp();
  PC.flush();
  if(blue_led_state){
    change_led_color(CYAN);
  }else{
    change_led_color(GREEN);
  }
}

void toggle_led(){
  char byte_from_msp;

  change_led_color(BLUE);
  MSP.write("L");
  while((char)MSP.read() != '\n');
  //while((char)MSP.read() != '=');
  while((char)MSP.read() != ' ');
  do{
    byte_from_msp = (char) MSP.read();
    if(byte_from_msp >= 0x20 && byte_from_msp < 0x7F){
      PC.print(byte_from_msp);
    }
    
  }while(byte_from_msp != '\n');
  PC.print('\n');
  //while((char) MSP.read() != '*');
  while(MSP.available()) MSP.read(); //discard_data_from_msp();
  PC.flush();
  if(blue_led_state){
    change_led_color(CYAN);
  }else{
    change_led_color(GREEN);
  }
}

void get_helper_function(char option){
  char byte_from_msp;

  change_led_color(BLUE);
  MSP.write("G");
  while((char) MSP.read() != '*');
  MSP.write(option);
  while((char) MSP.read() == '*');
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
  while((char) MSP.read() != '*');
  while(MSP.available()) MSP.read(); //discard_data_from_msp();
  //while(MSP.available()) MSP.read();
  PC.flush();
  if(blue_led_state){
    change_led_color(CYAN);
  }else{
    change_led_color(GREEN);
  }
}

void set_helper_function(char option){
  char input_from_base = '0';
  change_led_color(BLUE);
  MSP.write("S");
  while((char) MSP.read() != '*');
  do{
    MSP.write((char) input_from_base);
    input_from_base = PC.read();
  }while(input_from_base != '\n');

  while((char) MSP.read() != '*');
  MSP.write(">");
  delay(100);
  while((char) MSP.read() != '*');
  

}

void loop(){
  //static bool timeout = true;
  //static 
  char byte_from_msp = '\0';
  char byte_from_pc = '\0';
  int last_time;
  bool quit = false;


  while(PC.sent_bytes() <= 0);
  if(PC.read() == CONNECT_TO_MSP){
    change_led_color(GREEN);
    ENABLE_5V;
    //discard_data_from_msp();
    delay(1000);
    MSP.write("1234");
    delay(500);
    //discard_data_from_msp();
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

        PC.flush();
      }

      if(PC.available()){
        byte_from_pc = PC.read();

        switch((char) byte_from_pc){
        case GET_FSM_STATE:         // F
          PC.print("Y\n");
          break;
        case GET_WET_AND_DRY_FREC:  // W
          get_helper_function('4');
          break;
        case GET_TEMPERATURE_FREC:  // T
          get_helper_function('5');
          break;
        case GET_CALIBRATED_MINUTE: // C
          get_helper_function('6');
          break;
        case GET_NAME:              // N
          get_helper_function('7');
          break;
        case GET_LOGGER_ID:         // I
          get_helper_function('8');
          break;
        case TOGGLE_LED:            // L
          toggle_led();
          break;
        case GET_ACTIVATION_TIME:   // A
          get_helper_function('9');
          break;
        case GET_AT_30C:            // 3
          get_calibration_C(at30C);
          break;
        case GET_AT_105C:           // 1
          get_calibration_C(at105C);
          break;  
        case GET_ALL_INDEX:         // X
          get_indexes();
          break;
        case GET_OPERATING_MODE:    //M
          get_operating_mode();
          break;

        default:

          break;
        }

        last_time = millis();
      }
      if (MSP.available()){
        byte_from_msp = MSP.read();
        //PC.write((char) byte_from_msp);
        PC.flush();
      }

    }
    DISABLE_5V;
  }
}


