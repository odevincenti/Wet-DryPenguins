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
//  MSP430FR267x Demo -  RTC, device enter LPM3.5 and toggle P1.0 in RTC
//                       interrupt handling every 1s
//
//
//   Description: Device enter LPM3.5 after configuring the RTC. The RTC wakes
//   the device up from LPM3.5 every second and toggles P1.0.
//   It also stores the state of P0OUT in the Backup Memory Registers.
//
//   XT1 = 32kHz, ACLK = default, MCLK = SMCLK = default, DCODIV = ~1MHz.
//
//           MSP430FR2676
//         ---------------
//     /|\|               |
//      | |      XIN(P2.1)|--
//      --|RST            |  ~32768Hz
//        |     XOUT(P2.0)|--
//        |               |
//        |          P1.0 |---> LED
//        |               |
//
//   Longyu Fang
//   Texas Instruments Inc.
//   August 2018
//   Built with IAR Embedded Workbench v7.12.1 & Code Composer Studio v8.1.0
//******************************************************************************


/*                       SLEEP:              MEDICION:           BASE:
 * P1.4 - Sense-1      -   0V (out)            3V                  TX?
 * P1.5 - Sense-2      -   0V (out)            Input w/pulldown    RX?
 * P2.4 - "Base"       -   output (0V)         output (0V)         output (3V)
 * P2.5 - "Agua"       -   output              output              output
 *
 *
 *  - Digilent-TX          3,3V                3,3V
 *  3K
 *  - MSP430-RX            0V                  Input w/pulldown
 *  97K
 *  - GND                  0V                  0V
 */

#define BYPASS 1

#include <msp430.h>

void initGpio(void);


unsigned char old_value = 0;

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;               // Stop WDT

    initGpio();                             // Configure GPIO

    do
    {
        CSCTL7 &= ~(XT1OFFG | DCOFFG);      // Clear XT1 and DCO fault flag
        SFRIFG1 &= ~OFIFG;
    } while (SFRIFG1 & OFIFG);              // Test oscillator fault flag


    // First determine whether we are coming out of an LPMx.5 or a regular RESET.
    if (SYSRSTIV == SYSRSTIV_LPM5WU)        // When woken up from LPM3.5, reinit
    {
        __enable_interrupt();               // The RTC interrupt should trigger now...

    }
    else
    {
        // Device powered up from a cold start.
        // It configures the device and puts the device into LPM3.5

        // Configure backup memory
        *(unsigned int *)BKMEM_BASE = 0;

        // Initialize RTC
        // Interrupt and reset happen every 256/32768 * 1 =  7.8125 msec (128Hz).
        RTCMOD = 1-1;//64-1;
        RTCCTL = RTCSS__XT1CLK | RTCSR | RTCPS__256 | RTCIE;

        old_value = 0;

    }

    P2OUT &=~BIT4;
    // Enter LPM3.5 mode with interrupts enabled. Note that this operation does
    // not return. The LPM3.5 will exit through a RESET event, resulting in a
    // re-start of the code.
    PMMCTL0_H = PMMPW_H;                    // Open PMM Registers for write
    PMMCTL0_L |= PMMREGOFF;                 // and set PMMREGOFF
    __bis_SR_register(LPM3_bits | GIE);
    __no_operation();

    return 0;
}


void initGpio(void)
{
    // PxDIR: 0 = Input; 1 = Output
    PADIR = 0xFFFF; PBDIR = 0xFFFF; PCDIR = 0xFFFF; //P1DIR = 0xFF; P2DIR = 0xFF; P3DIR = 0xFF; P4DIR = 0xFF; P5DIR = 0xFF; P6DIR = 0xFF;
    // PxREN = 1 -> pullup/pulldown resistor
    PAREN = 0xFFFF; PBREN = 0xFFFF; PCREN = 0xFFFF; //P1REN = 0xFF; P2REN = 0xFF; P3REN = 0xFF; P4REN = 0xFF; P5REN = 0xFF; P6REN = 0xFF;
    // PxOUT: self-explanatory
    PAOUT = 0x0000; PBOUT = 0x0000; PCOUT = 0x0000; //P1OUT = 0x00; P2OUT = 0x00; P3OUT = 0x00; P4OUT = 0x00; P5OUT = 0x00; P6OUT = 0x00;

    P2OUT |= BIT4;
    // Initialize XT1 32kHz crystal
    P2SEL0    |=  (BIT0|BIT1);  // P2.0~P2.1: crystal pins
    // P2SEL1 &= ~(BIT0|BIT1);
#if BYPASS==1
    CSCTL6 |= XT1BYPASS;
#endif


    // Disable the GPIO power-on default high-impedance mode
    // to activate previously configured port settings
    PM5CTL0 &= ~LOCKLPM5;
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = RTC_VECTOR
__interrupt void RTC_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(RTC_VECTOR))) RTC_ISR (void)
#else
#error Compiler not supported!
#endif
{
    switch(__even_in_range(RTCIV, RTCIV_RTCIF))
    {
        case RTCIV_NONE : break;            // No interrupt pending
        case RTCIV_RTCIF:                   // RTC Overflow

            if(*(unsigned int *)BKMEM_BASE < 1){
                P2OUT |= BIT5;
            }else{
                P2OUT &=~BIT5;
            }
            (*(unsigned int *)BKMEM_BASE)++;
            *(unsigned int *)BKMEM_BASE &= 0x3ff;

            break;
        default:          break;
    }
}

// 3.3V - 1,66V = 1.64V
// 1.64V / 2210 ohm = 0.74mA
// 0.74mA * (1/1024)(Duty) = 0.72uA

