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


#include <msp430.h>
//#include <stdio.h>

#include <my_config.h>
#include <my_project_config.h>
#include <my_project_variables.h>
#include <uart_functions.h>
//#include <adc_functions.h>
#include <fsm.h>
#include <logging.h>

//const char message_new_message[] = "\n////////////////////////////////////\n\n";
unsigned char RXData = 0;
//#define TX_INT(var)           snprintf(auxiliar_string, AUX_STIRNG_SIZE-1,"%d",var); tx_string(auxiliar_string)

void initGpio(void);
void uart_function(void);
//void update_leds(void);


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
        __no_operation();
        __disable_interrupt();
        __no_operation();

        if(base_detected){
            uart_function();
            base_detected = 0;
            PIN_IFLAG_CLR(base_detect_pin);
            PIN_I_EN(base_detect_pin);
            if(password_timeout){
                if(device_is_active){
                    RETURN_TO_SAVED_RTCCNT;
                }else{
                    DISABLE_RTC;
                }
            }else if(new_state_just_configured){
                clear_events();
                start_rtc_counters();
            }else{
                if(device_is_active){
                    RETURN_TO_SAVED_RTCCNT;
                }else{
                    DISABLE_RTC;
                }
            }

        }else if(measurements_pending){
            UPDATE_PERSISTENT_VAR(sensing,1);

            if(remaining_mins_until_wet_and_dry == 0)
                wet_and_dry_function();
            if(remaining_mins_until_temperature == 0)
                temperature_function();

            UPDATE_PERSISTENT_VAR(sensing,0);
            set_rtc_to_next_active();
            measurements_pending = 0;

        }


    }
    else
    {   // Device powered up from a cold start.
        //RTC_STATE = 0;
        if(sensing){
            new_event(RESET_WHILE_SENSING);
        }else if(uart){
            new_event(RESET_WHILE_UART);
        }else{
            new_event(RESET);
        }

        UPDATE_PERSISTENT_VAR(sensing,0);
        UPDATE_PERSISTENT_VAR(sensing,0);

        detecting_base_debouncing = 0;
        measurements_pending = 0;
        yellow_state = 0;
        base_detected = 0;
        red_state = 0;
        expecting_starting_pasword = 0;
        communicating_with_base = 0;
        password_timeout = 0;
        new_state_just_configured = 0;
        PIN_IFLAG_CLR(base_detect_pin);
        PIN_I_EN(base_detect_pin);

        UPDATE_PERSISTENT_VAR(resets_since_last_communication,resets_since_last_communication+1);


        UPDATE_PERSISTENT_VAR(sensing,1);
        if(remaining_mins_until_wet_and_dry == 0)
            wet_and_dry_function();
        if(remaining_mins_until_temperature == 0)
            temperature_function();
        UPDATE_PERSISTENT_VAR(sensing,0);
        set_rtc_to_next_active();

        if(PIN_IN_GET(base_detect_pin)){
            if(device_is_active){
                SAVE_RTCCNT_AND_DISABLE_RTC;
                SUBSTRACT_SAVED_RTCCNT(DEBOUNCING_T);
            }else{
                DISABLE_RTC; // (por las dudas)
            }
            PIN_IFLAG_CLR(base_detect_pin);
            detecting_base_debouncing = DEBOUNCING_AMMOUNT;
            PIN_I_DIS(base_detect_pin);

            PIN_OUT_SET(green_pin);
            SET_RTC_TO(DEBOUNCING_T);
        }

    }
    //update_leds();


    PMMCTL0_H = PMMPW_H;                    // Open PMM Registers for write
    PMMCTL0_L |= PMMREGOFF;                 // and set PMMREGOFF
#if EXTRA_SIGNALS_FOR_DEBUG
    PIN_OUT_CLR(pinAM);
#endif
    __bis_SR_register(LPM3_bits | GIE);
    __no_operation();

    return 0;
}


void initGpio(void)
{
    ALL_REN_EN;
    ALL_DIR_OUT;
    ALL_OUT_CLR;

    PIN_DIR_IN(base_detect_pin);
    PIN_IEDGE_L2H(base_detect_pin);
    if(detecting_base_debouncing == 0)
        PIN_I_EN(base_detect_pin);


#if EXTRA_SIGNALS_FOR_DEBUG
    PIN_OUT_SET(pinAM);
#endif
    PM5CTL0 &= ~LOCKLPM5;
}

///////////////////// PORT4 VECTOR ///////////////////////////////////
#pragma vector=PORT4_VECTOR
__interrupt void Port_4(void){
    if(device_is_active){
        SAVE_RTCCNT_AND_DISABLE_RTC;
        SUBSTRACT_SAVED_RTCCNT(DEBOUNCING_T);
    }else{
        DISABLE_RTC; // (por las dudas)
    }

    PIN_IFLAG_CLR(base_detect_pin);
    detecting_base_debouncing = DEBOUNCING_AMMOUNT;
    PIN_I_DIS(base_detect_pin);

    PIN_OUT_SET(green_pin);
    SET_RTC_TO(DEBOUNCING_T);
}


///////////////////// RTC VECTOR ///////////////////////////////////
#pragma vector = RTC_VECTOR
__interrupt void RTC_ISR(void){
    //int aux_unconnecting = 0;
    switch(__even_in_range(RTCIV, RTCIV_RTCIF))
    {

        case RTCIV_NONE : break;            // No interrupt pending
        case RTCIV_RTCIF:                   // RTC Overflow

            DISABLE_RTC;
            if(expecting_starting_pasword){
                password_timeout = 1;
            } else if(communicating_with_base){
                communicating_with_base = 0;
            }else if(detecting_base_debouncing == 0){
                if(device_is_active){
                    measurements_pending = 1;
                }else{ // POR LAS DUDAS; PERO NO DEBERIA PASAR NUNCA
                    PIN_IFLAG_CLR(base_detect_pin);
                    PIN_I_EN(base_detect_pin);
                }
            }else{
                detecting_base_debouncing--;
                if(PIN_IN_GET(base_detect_pin)){
                    if(detecting_base_debouncing == 0){
                        base_detected = 1;
                        PIN_IFLAG_CLR(base_detect_pin);
                        //PIN_I_EN(base_detect_pin);
                        PIN_OUT_CLR(green_pin);

                    }else{
                        PIN_OUT_SET(green_pin);
                        if(device_is_active){
                            SUBSTRACT_SAVED_RTCCNT(DEBOUNCING_T);
                        }
                        SET_RTC_TO(DEBOUNCING_T);
                    }
                }
                else{
                    new_event(FALSE_BASE_DETECTION);
                    detecting_base_debouncing = 0;
                    PIN_IFLAG_CLR(base_detect_pin);
                    PIN_I_EN(base_detect_pin);
                    PIN_OUT_CLR(green_pin);
                    if(device_is_active){
                        RETURN_TO_SAVED_RTCCNT;
                    }else{
                        DISABLE_RTC;
                    }
                }

            }



            break;
        default:          break;
    }
}


//void update_leds(void){
//    last_events[0][0] = 2;
//    if(red_state){
//        PIN_OUT_SET(red_pin);
//    }else{
//        PIN_OUT_CLR(red_pin);
//    }
//    if(yellow_state){
//        PIN_OUT_SET(yellow_pin);
//    }else{
//        PIN_OUT_CLR(yellow_pin);
//    }
//
//#if EXTRA_SIGNALS_FOR_DEBUG
//    if(detecting_base_debouncing & (1<<0))
//        PIN_OUT_SET(pin0);
//    else
//        PIN_OUT_CLR(pin0);
//    if(detecting_base_debouncing & (1<<1))
//        PIN_OUT_SET(pin1);
//    else
//        PIN_OUT_CLR(pin1);
//    if(detecting_base_debouncing & (1<<2))
//        PIN_OUT_SET(pin2);
//    else
//        PIN_OUT_CLR(pin2);
//    if(detecting_base_debouncing & (1<<3))
//        PIN_OUT_SET(pin3);
//    else
//        PIN_OUT_CLR(pin3);
////    if(cnt & (1<<4))
////        PIN_OUT_SET(pin4);
////    else
////        PIN_OUT_CLR(pin4);
////    if(cnt & (1<<5))
////        PIN_OUT_SET(pin5);
////    else
////        PIN_OUT_CLR(pin5);
////    if(cnt & (1<<6))
////        PIN_OUT_SET(pin6);
////    else
////        PIN_OUT_CLR(pin6);
////    if(cnt & (1<<7))
////        PIN_OUT_SET(pin7);
////    else
////        PIN_OUT_CLR(pin7);
//#endif
//}

void uart_function(void){
    //    PIN_SEL_01(tx_pin);
    //    PIN_SEL_01(rx_pin);
    int exit_loop = 0;
    unsigned int password_index = 0;

    DISABLE_RTC;
    PIN_I_DIS(base_detect_pin);

    UPDATE_PERSISTENT_VAR(uart,1);

    PIN_SEL_01(tx_pin);
    PIN_SEL_01(rx_pin);
    UART_setup();
    tx_string("OK\n");


#if EXTRA_SIGNALS_FOR_DEBUG
    update_leds();
    tx_string("\nInsert Password NOW!");
#endif

    exit_loop = 0;
    expecting_starting_pasword = 1;
    password_timeout = 0;

    SUBSTRACT_SAVED_RTCCNT(PASSWORD_TIME);
    __enable_interrupt();
    SET_RTC_TO(PASSWORD_TIME);
    do{
        while(!(UCA0IFG & UCRXIFG) && !password_timeout);
        if(UCA0IFG & UCRXIFG){
            RXData = UCA0RXBUF;
            if(RXData == PASSWORD[password_index]){
                if(password_index >= 3)
                    exit_loop = 1;
                else
                    password_index++;
            }else{
                password_index = 0;
            }
        }
    }while(!password_timeout && !exit_loop);
    __disable_interrupt();
    __no_operation();
    expecting_starting_pasword = 0;

    if(password_timeout){
#if EXTRA_SIGNALS_FOR_DEBUG
        tx_string("\nTimeout!");
#endif
        UART_reset();
        PIN_SEL_00(tx_pin);
        PIN_SEL_00(rx_pin);
        UPDATE_PERSISTENT_VAR(uart,0);
        new_event(FAILED_STARTING_PASSWORD);
        return;
    }

    exit_loop = 0;
    communicating_with_base = 1;
    //tx_string("\nUART");
    new_event(UART_COMMUNICATION);
    fsm_init();

    do{
        DISABLE_RTC;
        communicating_with_base = 1;
        __enable_interrupt();
        __no_operation();
        SET_RTC_TO(INACTIVITY_TIME);
        while(!(UCA0IFG & UCRXIFG) && communicating_with_base);
        DISABLE_RTC;
        __disable_interrupt();
        __no_operation();

        if(communicating_with_base){
            if(UCA0STATW & UCFE){
                SEND_NAK;
                tx_string("\nRECEIVED CHARACTER WITH FRAMING ERROR\n");
                SEND_NUL;
                RXData = UCA0RXBUF;
                continue;
            }
            if(UCA0STATW & UCPE){
                SEND_NAK;
                tx_string("\nRECEIVED CHARACTER WITH PARITY ERROR\n");
                SEND_NUL;
                RXData = UCA0RXBUF;
                continue;
            }

            RXData = UCA0RXBUF;
            if (RXData == '\n')
                continue;
            if(fsm_function(RXData)){
                exit_loop = 1;
            }

        }else{
            tx_string("\nDisconnecting because of inactivity\n");
            new_event(INACTIVITY);
            exit_loop = 1;
        }
    }while(!exit_loop);
    tx_string("OK\n");
    while(PIN_IN_GET(base_detect_pin));
    UART_reset();
    PIN_SEL_00(tx_pin);
    PIN_SEL_00(rx_pin);
    communicating_with_base = 0;
    UPDATE_PERSISTENT_VAR(uart,0);

}

