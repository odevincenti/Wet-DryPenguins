/* --COPYRIGHT--,BSD_EX
 * Copyright (c) 2018, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *******************************************************************************
 *
 *                       MSP430 CODE EXAMPLE DISCLAIMER
 *
 * MSP430 code examples are self-contained low-level programs that typically
 * demonstrate a single peripheral function or device feature in a highly
 * concise manner. For this the code may rely on the device's power-on default
 * register values and settings such as the clock configuration and care must
 * be taken when combining code from several examples to avoid potential side
 * effects. Also see www.ti.com/grace for a GUI- and www.ti.com/msp430ware
 * for an API functional library-approach to peripheral configuration.
 *
 * --/COPYRIGHT--*/
//******************************************************************************

//ALGUN DIA:
// [ ] ver cuanto timepo esta encendido el micro
// [ ] TODO(ish): ver como logear baja tension
// [ ] TODO: checkear sin Cap de rst
// [ ] TODO: checkear con digilent con 3 y 5V simultaneas

//PRONTO:
// [x] que pasa si RST durante UART?
// [ ] TODO: probar solo patas exteriores
// [x] QUITARLE LOS CONSUMOS DE LED INNECESARIOS!!! (MORE_LED)
// [x] QUITARLE EL SECURE
// [x] QUITARLE EL RTC ON DEFAULT










// NOTA: el problema del ruido se da cuando, con los cables de base_detect y gnd exterior desconectados, trato de conectar el cable macho-hembra al pin externo del base detect (con la otra terminal aun al aire)
// [x] TODO: corroborar si al minimizar el tiempo de debouncing no aumento el riesgo de falso positivo considerado positivo. Analizar si se arregla al tomar varias mediciones con deltas de tiempo mas chicas.




#include <msp430.h>
#include <stdio.h>

#include <my_config.h>
#include <uart_functions.h>

#pragma PERSISTENT(config_rtc_active)
unsigned int config_rtc_active = 0;
#pragma PERSISTENT(state_on_process)
unsigned int state_on_process = 0;
#pragma PERSISTENT(state_index)
int state_index = -1;
#pragma PERSISTENT(warning_memory_full)
unsigned int warning_memory_full = 0;

int i = 0;


char quit = 0;
//const char* index = 0;
unsigned char RXData = 0;

#pragma PERSISTENT(counter_array)
unsigned int counter_array[AMOUNT_OF_COUNTERS] = {[0 ... AMOUNT_OF_COUNTERS-1] = 0};
#pragma PERSISTENT(counter_description_array)
char counter_description_array[AMOUNT_OF_COUNTERS] = {[0 ... AMOUNT_OF_COUNTERS-1] = 0};

unsigned int UART_FSM_STATE = 0;
// don't automatically change these at reset:


const char message_new_message[] = "////////////////////////////////////\n\n";
const char message_main_menu[] = "Penguin Tracker Beta\nG = get\nS = set\nR = reset counters and warnings\nQ = quit\n\n";
const char message_getting_menu[] = "A = all variables\nX = cancel operation\n\n";

const char message_counter_descriptions[8][50] = {
                                                "CURRENT counter\n",
                                                "Counter FULL\n",
                                                "Counter interrupted by RESET\n",
                                                "Counter interrupted WHILE IN PROGRESS\n",
                                                "Counter interrupted by RESET WHILE IN PROGRESS\n",
                                                "Counter interrupted by FALSE base detection\n",
                                                "Counter interrupted by BASE DETECTION\n",
                                                "Counter interrupted by RESET WHILE IN UART\n"
};

char auxiliar_string[AUX_STIRNG_SIZE];

void count_up_current_counter(void);
void go_to_next_counter(char cause);
void reset_counters(void);

void initGpio(void);
void uart_communication(void);

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;               // Stop WDT
    initGpio();                             // Configure GPIO

    do {
        CSCTL7 &= ~(XT1OFFG | DCOFFG);      // Clear XT1 and DCO fault flag
        SFRIFG1 &= ~OFIFG;
    } while (CSCTL7 & DCOFFG);              // Test oscillator fault flag


    // First determine whether we are coming out of an LPMx.5 or a regular RESET.
    if (SYSRSTIV == SYSRSTIV_LPM5WU)        // When woken up from LPM3.5, reinit
    {
        __enable_interrupt();
        __disable_interrupt();
        if(BKUP_UART_ACTIVE){
            PIN_OUT_SET(yellow_pin);
            //PIN_I_DIS(base_detect_pin);
            //RTCCTL &= ~RTCIE;
            go_to_next_counter(COUNTER_INT_UART);
            uart_communication();
            PIN_OUT_SET(yellow_pin);

            BKUP_LED_STATE = 1;
            BKUP_DEBOUNCING = 0;
            BKUP_UART_ACTIVE = 0;
            BKUP_UNCONNECTING = 1;
            BKUP_AUX_UNCONNECTING = 0;
            //BKUP_COUNT_PENDING = 0;

            PIN_I_DIS(base_detect_pin);
            PIN_IFLAG_CLR(base_detect_pin);

            RTCMOD = MOD_DEBOUNCE;
            RTCCTL = RTCSS__VLOCLK | RTCSR | RTCPS__16 | RTCIE;
        }

    }
    else
    {   // Device powered up from a cold start.
        PIN_IFLAG_CLR(base_detect_pin);

        if (state_on_process){
            go_to_next_counter(COUNTER_INT_RESETTED_ON_PROCESS);
        }else if(BKUP_UART_ACTIVE){
            go_to_next_counter(COUNTER_INT_RESETTED_DURING_UART);
        }else{
            go_to_next_counter(COUNTER_INT_RESETTED);
        }

        BKUP_LED_STATE = 1;
        BKUP_DEBOUNCING = 0;
        BKUP_UART_ACTIVE = 0;
        BKUP_UNCONNECTING = 0;
        BKUP_AUX_UNCONNECTING = 1;

        //BKUP_COUNT_PENDING = 0;
        // CON VLO: Interrupt and reset happen every  16/10000 * 4 =  6.4 msec (156.25Hz)




        RTCMOD = MOD_CORTO;
        if(config_rtc_active){
            RTCCTL = RTCSS__VLOCLK | RTCSR | RTCPS__16 | RTCIE;
        }else{
            RTCCTL = RTCSS__VLOCLK | RTCSR | RTCPS__16; // Good // 0.4uA ish, en vez de 2uA. Nice
        }


    }

    PMMCTL0_H = PMMPW_H;                    // Open PMM Registers for write
    PMMCTL0_L |= PMMREGOFF;                 // and set PMMREGOFF

    __bis_SR_register(LPM3_bits | GIE);
    __no_operation();

    return 0;
}

void uart_communication(void){
    PIN_SEL_01(tx_pin);
    PIN_SEL_01(rx_pin);

    __bis_SR_register(SCG0);                // Disable FLL
    CSCTL3 = SELREF__REFOCLK;               // Set XT1 as FLL reference source
    CSCTL1 = DCOFTRIMEN_1 | DCOFTRIM0 | DCOFTRIM1 | DCORSEL_0;// DCOFTRIM=3, DCO Range = 1MHz
    CSCTL2 = FLLD_0 + 30;                   // DCODIV = 1MHz
    __delay_cycles(3);
    __bic_SR_register(SCG0);                // Enable FLL

    Software_Trim();                        // Software Trim to get the best DCOFTRIM value
    CSCTL4 = SELMS__DCOCLKDIV | SELA__REFOCLK; // set XT1(~32768Hz) as ACLK source, ACLK = 32768Hz
                                               // default DCODIV as MCLK and SMCLK source

    // Configure UART
    UCA0CTLW0 |= UCSWRST;                     // Put eUSCI in reset
    UCA0CTLW0 |= UCSSEL__SMCLK;
    // Baud Rate calculation
    UCA0BR0 = 8;                              // 1000000/115200 = 8.68
    UCA0MCTLW = 0xD600;                       // 1000000/115200 - INT(1000000/115200)=0.68
                                              // UCBRSx value = 0xD6 (See UG)
    UCA0BR1 = 0;
    UCA0CTLW0 |= UCRXEIE;
    UCA0CTLW0 &= ~UCSWRST;                    // Initialize eUSCI
    //UCA0IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt

    quit = 0;

    tx_string("Hello!\n\n");
    tx_string(message_main_menu);
    PIN_OUT_CLR(yellow_pin);
    UART_FSM_STATE = FSM_START;

    do{
       while(!(UCA0IFG & UCRXIFG));// && () != '\n');
       //RXData = UCA0RXBUF;  // Clear buffer
       RXData = UCA0RXBUF;

       if (RXData == '\n')
           continue;
       switch(UART_FSM_STATE){


       case FSM_START:
           switch(RXData){
           case 'G': case 'g':
               UART_FSM_STATE = FSM_GETTING;
               tx_string(message_new_message);
               tx_string("Getting ...\n");//("Getting ...\nA = all variables\nC = counters and descriptions\nW = warnings\nX = cancel operation\n\n");
               tx_string(message_getting_menu);
               break;
           case 'S': case 's':
               UART_FSM_STATE = FSM_SETTING;
               tx_string(message_new_message);
               tx_string("Setting ...\nR = enable/disable RTC\nX = cancel operation\n\n");
               break;
           case 'R': case 'r':
               UART_FSM_STATE = FSM_RESETTING_0;
               tx_string(message_new_message);
               tx_string("RESETTING COUNTERS AND WARNINGS\nPlease insert password\n\n");
               break;
           case 'Q': case 'q':
               quit = 1;
               tx_string(message_new_message);
               tx_string("Goodbye!\nPlease, disconnect base\n\n");
               break;
           default:
               tx_string(message_new_message);
               tx_string("Option not valid\nG = get\nS = set\nR = reset all\nQ = quit\n\n");
               break;
           }
           break;

       case FSM_GETTING:
           switch(RXData){
           case 'A': case 'a':
               UART_FSM_STATE = FSM_START;
               tx_string(message_new_message);

               tx_string("RTC = ");
               tx_string(config_rtc_active ? "enabled\n" : "disabled\n");
               tx_string("Memory full = ");
               tx_string(warning_memory_full ? "True\n" : "False\n");
               tx_string("Current index = ");
               snprintf(auxiliar_string, AUX_STIRNG_SIZE-1,"%d\n",state_index);
               tx_string(auxiliar_string);
               for(i = 0; i<= state_index; i++){
                   snprintf(auxiliar_string, AUX_STIRNG_SIZE-1,"C:%d = ",i);
                   tx_string(auxiliar_string);
                   snprintf(auxiliar_string, AUX_STIRNG_SIZE-1,"%d : ",counter_array[i]);
                   tx_string(auxiliar_string);
                   tx_string(message_counter_descriptions[counter_description_array[i]]);
               }
               tx_string("\n");
               tx_string(message_main_menu);
               break;
           default:
               tx_string(message_new_message);
               tx_string("Option not valid\n");
               tx_string(message_getting_menu);
               break;
           case 'X': case 'x':
               UART_FSM_STATE = FSM_START;
               tx_string(message_main_menu);
               break;
           }
           break;

       case FSM_SETTING_RTC_ENABLE:
           switch(RXData){
           case '0':
               UART_FSM_STATE = FSM_START;
               SYSCFG0 = FRWPPW | DFWP;
               config_rtc_active = 0;
               SYSCFG0 = FRWPPW | PFWP | DFWP;
               tx_string(message_new_message);
               tx_string("RTC disabled\nGoing back to main menu\n\n");
               tx_string(message_main_menu);
               break;
           case '1':
               UART_FSM_STATE = FSM_START;
               SYSCFG0 = FRWPPW | DFWP;
               config_rtc_active = 1;
               SYSCFG0 = FRWPPW | PFWP | DFWP;
               tx_string(message_new_message);
               tx_string("RTC enabled\nGoing back to main menu\n\n");
               tx_string(message_main_menu);
               break;
           default:
               tx_string(message_new_message);
               tx_string("Option not valid\n0 = disable\n1 = enable\n\n");
               break;
           case 'X': case 'x':
               UART_FSM_STATE = FSM_START;
               tx_string(message_main_menu);
               break;
           }
           break;

       case FSM_SETTING:
           switch(RXData){
           case 'R': case 'r':
               UART_FSM_STATE = FSM_SETTING_RTC_ENABLE;
               tx_string(message_new_message);
               tx_string("0 = disable\n1 = enable\n\n");
               break;
           default:
               tx_string(message_new_message);
               tx_string("Option not valid\nR = enable/disable RTC\nX = cancel operation\n\n");
               break;
           case 'X': case 'x':
               UART_FSM_STATE = FSM_START;
               tx_string(message_main_menu);
               break;
           }
           break;

       case FSM_RESETTING_0:
           if(RXData == '1')
               UART_FSM_STATE = FSM_RESETTING_1;
           else
               UART_FSM_STATE = FSM_RESETTING_NOTVALID_1;
           break;
       case FSM_RESETTING_1:
           if(RXData == '2')
               UART_FSM_STATE = FSM_RESETTING_2;
           else
               UART_FSM_STATE = FSM_RESETTING_NOTVALID_2;
           break;
       case FSM_RESETTING_2:
           if(RXData == '3')
               UART_FSM_STATE = FSM_RESETTING_3;
           else
               UART_FSM_STATE = FSM_RESETTING_NOTVALID_3;
           break;
       case FSM_RESETTING_3:
           if(RXData == '4'){
               UART_FSM_STATE = FSM_START;
               reset_counters();
               tx_string(message_new_message);
               tx_string("Resetting counters and warnings now\nGoing back to main menu\n\n");
               tx_string(message_main_menu);
           }
           else{
               UART_FSM_STATE = FSM_START;
               tx_string(message_new_message);
               tx_string("Incorrect password!\nGoing back to main menu\n\n");
               tx_string(message_main_menu);
           }
           break;
       case FSM_RESETTING_NOTVALID_1:
           UART_FSM_STATE = FSM_RESETTING_2;
           break;
       case FSM_RESETTING_NOTVALID_2:
           UART_FSM_STATE = FSM_RESETTING_3;
           break;
       case FSM_RESETTING_NOTVALID_3:
           UART_FSM_STATE = FSM_START;
           tx_string(message_new_message);
           tx_string("Incorrect password!\nGoing back to main menu\n\n");
           tx_string(message_main_menu);
           break;

       default:
           UART_FSM_STATE = FSM_START;
           tx_string(message_new_message);
           tx_string("Unknown UART state!\nGoing back to main menu...\n\n");
           tx_string(message_main_menu);
           break;
       }


    }while(!quit);

    while(PIN_IN_GET(base_detect_pin)){
#if MORE_LED
        PIN_OUT_SET(yellow_pin);
        PIN_OUT_CLR(yellow_pin);
        __delay_cycles(10);
#endif
        ;
    }
    PIN_OUT_CLR(yellow_pin);

    UCA0CTLW0 = UCSWRST;
#if SECURE
    PIN_SEL_01(tx_pin);
    PIN_SEL_01(rx_pin);
#else
    PIN_SEL_00(tx_pin);
    PIN_SEL_00(rx_pin);
#endif
}



void initGpio(void)
{
    ALL_REN_EN;
    ALL_DIR_OUT;
    ALL_OUT_CLR;

    PIN_DIR_IN(base_detect_pin);
    PIN_IEDGE_L2H(base_detect_pin);
    if(!BKUP_DEBOUNCING && !BKUP_UART_ACTIVE){
        PIN_I_EN(base_detect_pin);
    }

#if SECURE
    PIN_SEL_01(tx_pin);
    PIN_SEL_01(rx_pin);
#endif


    PM5CTL0 &= ~LOCKLPM5;
}

///////////////////// PORT4 VECTOR ///////////////////////////////////
#pragma vector=PORT4_VECTOR
__interrupt void Port_4(void){

    PIN_IFLAG_CLR(base_detect_pin);
#if MORE_LED
    PIN_OUT_SET(red_pin);
#endif
    BKUP_DEBOUNCING = 3;
    BKUP_AUX_UNCONNECTING = 0;

    RTCMOD = MOD_DEBOUNCE_PARCIAL;
    RTCCTL = RTCSS__VLOCLK | RTCSR | RTCPS__16 | RTCIE;

    PIN_I_DIS(base_detect_pin);
}

///////////////////// RTC VECTOR ///////////////////////////////////
#pragma vector = RTC_VECTOR
__interrupt void RTC_ISR(void){
    //int aux_unconnecting = 0;
    switch(__even_in_range(RTCIV, RTCIV_RTCIF))
    {

        case RTCIV_NONE : break;            // No interrupt pending
        case RTCIV_RTCIF:                   // RTC Overflow

            RTCCTL &= ~RTCIE;

            if(BKUP_UNCONNECTING){
                PIN_IFLAG_CLR(base_detect_pin);
                BKUP_UNCONNECTING = 0;
                BKUP_AUX_UNCONNECTING = 1;
            }

            // completar si unconnecting?
            if(!BKUP_DEBOUNCING){       //  Estado "normal" (titilante)
                if(!BKUP_LED_STATE){    //  Si esta prendido el led, lo apaga
                    BKUP_LED_STATE = 1;
                    PIN_OUT_CLR(red_pin);
                    RTCMOD = MOD_LARGO;
                    if(config_rtc_active){  // Si el clock esta "activo"

                        if(!BKUP_AUX_UNCONNECTING && !warning_memory_full)
                            count_up_current_counter();
                        if(BKUP_AUX_UNCONNECTING)
                            BKUP_AUX_UNCONNECTING = 0;
                        RTCCTL = RTCSS__VLOCLK | RTCSR | RTCPS__16 | RTCIE;
                            //BKUP_COUNT_PENDING = 1; // Si no volvio inmediatamente de UART, y hay memoria, anota "conteo pendiente"
                    }else{                  // Si el clock no esta "activo"
                        if(BKUP_AUX_UNCONNECTING)
                            BKUP_AUX_UNCONNECTING = 0;
                        RTCCTL = RTCSS__VLOCLK | RTCSR | RTCPS__16; // Desabilita interrupcion de clock
                    }
                }else{ // Si esta apagado el led, lo prende
                    BKUP_LED_STATE = 0;
                    PIN_OUT_SET(red_pin);
                    RTCMOD = MOD_CORTO;
                    RTCCTL = RTCSS__VLOCLK | RTCSR | RTCPS__16 | RTCIE;
                }

            }else{ // if(BKUP_DEBOUNCING){
                PIN_IFLAG_CLR(base_detect_pin);
                BKUP_DEBOUNCING--;
                if(PIN_IN_GET(base_detect_pin)){
                    if(BKUP_DEBOUNCING == 0){
                        BKUP_LED_STATE = 1; // (por las dudas)
                        PIN_I_DIS(base_detect_pin);
                        //RTCCTL = RTCSS__DISABLED | RTCSR | RTCPS__16; //RTCCTL &= ~RTCIE; (redundante)
                        BKUP_UART_ACTIVE = 1;
                    }else{
#if MORE_LED
                        PIN_OUT_SET(red_pin);
#endif
                        RTCCTL = RTCSS__VLOCLK | RTCSR | RTCPS__16 | RTCIE;
                    }
                }else{
                    BKUP_DEBOUNCING = 0;
                    BKUP_LED_STATE = 1;
                    RTCMOD = MOD_CORTO;
                    PIN_I_EN(base_detect_pin);
                    BKUP_AUX_UNCONNECTING = 1;
                    go_to_next_counter(COUNTER_INT_FALSE_UART);
                    if(config_rtc_active){
                        RTCCTL = RTCSS__VLOCLK | RTCSR | RTCPS__16 | RTCIE;
                    }else{
                        RTCCTL = RTCSS__VLOCLK | RTCSR | RTCPS__16;
                    }

                }

            }

            break;
        default:          break;
    }
}

void reset_counters(void){
    SYSCFG0 = FRWPPW | DFWP;
    warning_memory_full = 0;
    state_index = 0;
    counter_array[0] = 0;
    counter_description_array[0] = COUNTER_CURRENT;
    SYSCFG0 = FRWPPW | PFWP | DFWP;
}

void count_up_current_counter(void){
    if(!warning_memory_full && state_index >= 0 && state_index < AMOUNT_OF_COUNTERS){
        if(state_on_process){
            go_to_next_counter(COUNTER_INT_ON_PROCESS);
        }else{
            SYSCFG0 = FRWPPW | DFWP;
            state_on_process = 1;
            counter_array[state_index]++;
            state_on_process = 0;
            SYSCFG0 = FRWPPW | PFWP | DFWP;

            if(counter_array[state_index] == (COUNTER_MAX_VALUE & 0xFFFF)){
                go_to_next_counter(COUNTER_FULL); //check consumo de toda la implementacion. Hacer que valor maximo sea 10, por ej
            }
        }

    }
}

void go_to_next_counter(char cause){
    if(!warning_memory_full){
        SYSCFG0 = FRWPPW | DFWP;
        state_on_process = 1;

        if(state_index >= 0 && state_index < AMOUNT_OF_COUNTERS){
            counter_description_array[state_index] = cause;
        }
        if(state_index < AMOUNT_OF_COUNTERS-1){
            state_index++;
            counter_array[state_index] = 0;
            counter_description_array[state_index] = COUNTER_CURRENT;
        }else{
            warning_memory_full = 1;
        }

        state_on_process = 0;
        SYSCFG0 = FRWPPW | PFWP | DFWP;
    }
}

