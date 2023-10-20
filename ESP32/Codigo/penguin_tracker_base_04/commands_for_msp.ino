#include "commands_for_msp.h"

void wait_until_character_and_discard(char character, unsigned int times = 1){
  unsigned int i = 0;
  for(i = 0; i < times; i++){
    while((char)MSP.read() != character);
  }
}

void wait_until_receiving_data_from_msp(void){
  while(MSP.sent_bytes() <= 0);
}

void discard_data_from_msp(void){
  while(MSP.sent_bytes()) MSP.read();
}

// void wait_until_character(char character){
//   while((char)MSP.peak() != character){MSP.read();}
// }

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
  wait_until_receiving_data_from_msp();
  discard_data_from_msp();
  PC.flush();
  change_led_color(CONNECTED);
}

void get_indexes(void){
  char aux[10] = "";
  unsigned int index = 0;
  char byte_from_msp;
  unsigned int i;
  change_led_color(PROCESSING);
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
  change_led_color(CONNECTED);
}

void get_calibration_C(bool atC){
  char byte_from_msp;

  change_led_color(PROCESSING);
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
  change_led_color(CONNECTED);
}

void toggle_led(){
  char byte_from_msp;

  change_led_color(PROCESSING);
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
  change_led_color(CONNECTED);
}

void get_helper_function(char option){
  char byte_from_msp;

  change_led_color(PROCESSING);
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
  change_led_color(CONNECTED);
}

void set_helper_function(char option){
  char input_from_base = '0';
  change_led_color(PROCESSING);
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
  
  while(1){
    //WIP
  }

}
