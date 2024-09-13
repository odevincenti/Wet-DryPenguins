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

#include <my_config.h>
#include <my_project_config.h>
#include <my_project_variables.h>
#include <uart_functions.h>
#include <fsm.h>
#include <logging.h>

unsigned char RXData = 0;

void initGpio(void);
void uart_function(void);

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

            if(remaining_mins_until_wet_and_dry == 0){
                wet_and_dry_function(0);
                UPDATE_PERSISTENT_VAR(remaining_mins_until_wet_and_dry, wet_and_dry_period);
            }

            if(remaining_mins_until_temperature == 0){
                temperature_function(0);
                UPDATE_PERSISTENT_VAR(remaining_mins_until_temperature, temperature_period);
            }


            UPDATE_PERSISTENT_VAR(sensing,0);
            set_rtc_to_next_active();
            measurements_pending = 0;

        }


    }
    else
    {   // Device powered up from a cold start.

        WRITE_FRAM_DISABLE;

        if(sensing){
            new_event(RESET_WHILE_SENSING);
        }else if(uart){
            new_event(RESET_WHILE_UART);
        }else{
            new_event(RESET);
        }

        UPDATE_PERSISTENT_VAR(sensing,0);
        UPDATE_PERSISTENT_VAR(uart,0);

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

        if(resets_since_last_communication < (unsigned int)0xFFFF){
            UPDATE_PERSISTENT_VAR(resets_since_last_communication,resets_since_last_communication+1);
        }

        //UPDATE_PERSISTENT_VAR(sensing,1);

        if(remaining_mins_until_wet_and_dry == 0){
            wet_and_dry_function(1); //answer = 0
            UPDATE_PERSISTENT_VAR(remaining_mins_until_wet_and_dry, wet_and_dry_period);
        }
        if(remaining_mins_until_temperature == 0){
            temperature_function(1); //answer = 0xFFFF
            UPDATE_PERSISTENT_VAR(remaining_mins_until_temperature, temperature_period);
        }

        //UPDATE_PERSISTENT_VAR(sensing,0);
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

            //PIN_OUT_SET(green_pin);
            SET_RTC_TO(DEBOUNCING_T);
        }

    }


    PMMCTL0_H = PMMPW_H;                    // Open PMM Registers for write
    PMMCTL0_L |= PMMREGOFF;                 // and set PMMREGOFF

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

    //PIN_OUT_SET(green_pin);
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
                if(is_calibrating_minute){
                    // should not happen
                }else{  // INACTIVITY
                    communicating_with_base = 0;
                }
            }else if(detecting_base_debouncing == 0){
                if(device_is_active){
                    measurements_pending = 1;
                }else{ // POR LAS DUDAS; PERO NO DEBERIA PASAR NUNCA
                    PIN_IFLAG_CLR(base_detect_pin);
                    PIN_I_EN(base_detect_pin);
                }
            }else{ //detecting_base_debouncing
                detecting_base_debouncing--;
                if(PIN_IN_GET(base_detect_pin)){
                    if(detecting_base_debouncing == 0){
                        base_detected = 1;
                        PIN_IFLAG_CLR(base_detect_pin);
                        //PIN_I_EN(base_detect_pin);
                        //PIN_OUT_CLR(green_pin);

                    }else{
                        //PIN_OUT_SET(green_pin);
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
                    //PIN_OUT_CLR(green_pin);
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

void uart_function(void){

    int exit_loop = 0;
    unsigned int password_index = 0;

    DISABLE_RTC;
    PIN_I_DIS(base_detect_pin);

    UPDATE_PERSISTENT_VAR(uart,1);

    PIN_SEL_01(rx_pin);
    PIN_SEL_01(tx_pin);
    UART_setup();
    tx_string("OK\n");

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
        SEND_NAK;
        UART_reset();
        PIN_SEL_00(tx_pin);
        PIN_SEL_00(rx_pin);
        UPDATE_PERSISTENT_VAR(uart,0);
        new_event(FAILED_STARTING_PASSWORD);
        return;
    }

    exit_loop = 0;
    is_calibrating_minute = 0;
    communicating_with_base = 1;
    new_event(UART_COMMUNICATION);
    fsm_init();

    do{
        DISABLE_RTC;
        communicating_with_base = 1;

        if(is_calibrating_minute){
            while(!(UCA0IFG & UCRXIFG));
        }else{
            CLEAR_RTCIFG;
            __enable_interrupt();
            __no_operation();
            SET_RTC_TO(INACTIVITY_TIME);
            while(!(UCA0IFG & UCRXIFG) && communicating_with_base);
            DISABLE_RTC;
            __disable_interrupt();
            __no_operation();
        }

        // RTC IS DISABLED HERE! (i hope)

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
    PIN_OUT_SET(red_pin);
    __delay_cycles(1600);
    PIN_OUT_CLR(red_pin);
    tx_string("OK\n");

    while(PIN_IN_GET(base_detect_pin));
    UART_reset();
    PIN_SEL_00(tx_pin);
    PIN_SEL_00(rx_pin);
    communicating_with_base = 0;
    UPDATE_PERSISTENT_VAR(uart,0);

}

