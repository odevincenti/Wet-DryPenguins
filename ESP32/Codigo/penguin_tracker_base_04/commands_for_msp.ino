#include "commands_for_msp.h"

void wait_until_character_and_discard(char character, unsigned int times = 1){
  unsigned int i = 0;
  for(i = 0; i < times; i++){
    while((char)MSP.read() != character);
  }
}

// void wait_until_receiving_data_from_msp(void){
//   while(MSP.sent_bytes() <= 0);
// }

// void discard_data_from_msp(void){
//   while(MSP.sent_bytes()) MSP.read();
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
  PC.flush();

  MSP.write("<");
  wait_until_character_and_discard('*',25);

  change_led_color(CONNECTED);
}

void get_indexes(void){
  char aux[10] = "";
  unsigned int index = 0;
  char byte_from_msp;
  unsigned int i;
  change_led_color(PROCESSING);
  MSP.write(">");

  wait_until_character_and_discard('\n',9);
  wait_until_character_and_discard('=');
  wait_until_character_and_discard(' ');

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
  wait_until_character_and_discard('\n');
  wait_until_character_and_discard('=');
  wait_until_character_and_discard(' ');
  
  do{
    byte_from_msp = (char) MSP.read();
    if(byte_from_msp >= 0x20 && byte_from_msp < 0x7F){
      PC.print(byte_from_msp);
    }
  }while(byte_from_msp != '\n');
  PC.print('\n');
  PC.print(aux);
  PC.print('\n');
  PC.flush();

  MSP.write("<");
  wait_until_character_and_discard('*',25);
  
  change_led_color(CONNECTED);
}

void get_calibration_C(bool atC){
  char byte_from_msp;

  change_led_color(PROCESSING);
  MSP.write("G");
  wait_until_character_and_discard('*',25);
  MSP.write("0");
  wait_until_character_and_discard('\n');
  if(atC){
    wait_until_character_and_discard('\n',2);
  }
  wait_until_character_and_discard('=');
  wait_until_character_and_discard(' ');
  do{
    byte_from_msp = (char) MSP.read();
    if(byte_from_msp >= 0x20 && byte_from_msp < 0x7F){
      PC.print(byte_from_msp);
    }
    
  }while(byte_from_msp != '\n');
  PC.print('\n');
  PC.flush();

  wait_until_character_and_discard('*',25);
  
  change_led_color(CONNECTED);
}

void toggle_led(){
  char byte_from_msp;

  change_led_color(PROCESSING);
  MSP.write("L");
  wait_until_character_and_discard('\n');
  wait_until_character_and_discard(' ');
  do{
    byte_from_msp = (char) MSP.read();
    if(byte_from_msp >= 0x20 && byte_from_msp < 0x7F){
      PC.print(byte_from_msp);
    }
    
  }while(byte_from_msp != '\n');
  PC.print('\n');
  PC.flush();

  wait_until_character_and_discard('*',25);
  
  change_led_color(CONNECTED);
}

void get_helper_function(char option){
  char byte_from_msp;

  change_led_color(PROCESSING);
  MSP.write("G");
  wait_until_character_and_discard('*',25);
  MSP.write(option);
  wait_until_character_and_discard('\n');
  wait_until_character_and_discard('=');
  wait_until_character_and_discard(' ');
  do{
    byte_from_msp = (char) MSP.read();
    if(byte_from_msp >= 0x20 && byte_from_msp < 0x7F){
      PC.print(byte_from_msp);
    }
    
  }while(byte_from_msp != '\n');
  PC.print('\n');
  PC.flush();

  wait_until_character_and_discard('*',25);

  change_led_color(CONNECTED);
}

void set_helper_function(char option){
  char input_from_base = '0';
  change_led_color(PROCESSING);
  MSP.write("S");
  wait_until_character_and_discard('*',25);
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