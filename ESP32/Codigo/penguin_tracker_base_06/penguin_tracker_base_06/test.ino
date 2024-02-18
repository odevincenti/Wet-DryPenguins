#include "test.h"
#include "commands_for_msp.h"

void test_millis(void){
    int last = millis();
    int diff = 0;
    while(last < 10000){
        //sleep(1);

        //diff=millis()-last;
        //if(diff>0)
        //    PC.print(diff);
        //PC.write('\n');
        PC.print("LAST = ");
        PC.print(last);
        PC.print("\tNOW =");
        PC.print(millis());
        PC.write('\n');

        last = millis();
    }
}

void test_buffer(void){
    unsigned int i = 0;
    int byte_from_msp = '\0';
    MSP.write("?");
    delay(500);
    MSP.write("?");
    delay(500);
    MSP.write("?");
    delay(500);
    MSP.write("?");
    delay(500);
    MSP.write("?");
    delay(500);
    MSP.write("?");
    delay(500);
    MSP.flush();
    PC.print(MSP.available());
    PC.write('\n');
    while(MSP.available()){
        PC.print(i++);
        PC.write('\t');
        byte_from_msp = MSP.read();
        PC.print(byte_from_msp);
        PC.write('\t');
        PC.write(byte_from_msp);
        PC.print("\tRemaining: ");
        PC.print(MSP.available());
        PC.write('\n');
        
    }

}
/*
void test_times(void){
    unsigned int operations[1000] = {0};
    unsigned int index=0;
    unsigned int index2 = 0;
    char byte_from_msp = '\0';

    change_led_color(PROCESSING);
    MSP.write("?");

    do{
        while(!MSP.available()){
            operations[index]++;
        }
        index++;
        byte_from_msp = MSP.read();

    }while(byte_from_msp != '*');
    
    wait_until_character_and_discard('*',24);
    change_led_color(CONNECTED);

    for(index2 = 0; index2 <= index; index2++){
        PC.print(operations[index2]);
        PC.print('\n');
    }
    
}
*/