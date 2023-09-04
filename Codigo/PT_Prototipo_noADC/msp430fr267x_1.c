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
 * P3.1 - Base_Detect  -   Input w/pulldown    HI-Z                Input w/pulldown?
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

#include <msp430.h>

void initGpio(void);
void Software_Trim();                       // Software Trim to get the best DCOFTRIM value

#define MCLK_FREQ_MHZ 1                     // MCLK = 1MHz

unsigned char RXData = 0;
unsigned char message1[] = "Fire!\n";
unsigned char message2[] = "Ice Storm!\n";
unsigned char message3[] = "Diacute!\n";
unsigned char message4[] = "BAYOEN!!!\n";
unsigned char quit = 0;
const char *index = 0;

unsigned char* current_isq = "None";

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
        if (P3IFG & BIT1){
            P2OUT     |=   BIT4;

            RTCCTL &= ~RTCIE;
            P3IE |= BIT1;               // The Port interrupt should trigger now... (I HOPE)
            __enable_interrupt();
            P3IFG &= ~BIT1;

            P1SEL0    |=  (BIT4|BIT5);
            // P1SEL1 &= ~(BIT4|BIT5); // P1SEL = 01 ( UART )


            __bis_SR_register(SCG0);                // Disable FLL
            CSCTL3 = SELREF__XT1CLK;               // Set XT1 as FLL reference source
            CSCTL1 = DCOFTRIMEN_1 | DCOFTRIM0 | DCOFTRIM1 | DCORSEL_0;// DCOFTRIM=3, DCO Range = 1MHz
            CSCTL2 = FLLD_0 + 30;                   // DCODIV = 1MHz
            __delay_cycles(3);
            __bic_SR_register(SCG0);                // Enable FLL



            Software_Trim();                        // Software Trim to get the best DCOFTRIM value
            CSCTL4 = SELMS__DCOCLKDIV | SELA__XT1CLK; // set XT1(~32768Hz) as ACLK source, ACLK = 32768Hz
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
            UCA0IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt



            //
            while(!(UCA0IFG & UCTXIFG));
            UCA0TXBUF = 'O';
            while(!(UCA0IFG & UCTXIFG));
            UCA0TXBUF = 'K';
            while(!(UCA0IFG & UCTXIFG));
            UCA0TXBUF = 'P';
            while(!(UCA0IFG & UCTXIFG));
            UCA0TXBUF = 'E';
            while(!(UCA0IFG & UCTXIFG));
            UCA0TXBUF = 'K';
            while(!(UCA0IFG & UCTXIFG));
            UCA0TXBUF = 'O';
            while(!(UCA0IFG & UCTXIFG));
            UCA0TXBUF = '\r';
            while(!(UCA0IFG & UCTXIFG));
            UCA0TXBUF = '\n';



            quit = 0;
            do{
                P3OUT |= BIT7;

                __bis_SR_register(LPM0_bits|GIE);     // Enter LPM0
                __no_operation();                     // For debugger
                P4OUT |= BIT0;

                P2OUT     |=   BIT5;
                P2OUT ^= BIT5;
                P2OUT ^= BIT5;
                switch(RXData){
                case 'Q':
                    quit = 1;
                    break;
                case 'A':
                    index = message1;
                    break;
                case 'B':
                    index = message2;
                    break;
                case 'C':
                    index = message3;
                    break;
                default:
                    index = message4;
                    break;
                }
                P4OUT |= BIT1;
                while (index != 0){
                    while(!(UCA0IFG & UCTXIFG));
                    UCA0TXBUF = (unsigned int)(*index);
                    if(*index == '\n'){
                        index = 0;
                    }else{
                        index++;
                    }
                }
                P4OUT |= BIT2;

            }while(!quit);
            //

            P1DIR &= ~BIT5;
            P1SEL0 &= ~BIT5;

            while((P1IN & BIT5) || (P3IN & BIT1)){
                P2OUT ^= BIT4;
            }

            UCA0IE &= ~UCRXIE;
            UCA0CTLW0 = UCSWRST;
            P2OUT     &=  ~BIT4;

            RTCCTL |= RTCIE;

            P1SEL0    &= ~(BIT4|BIT5);
            P1DIR |= BIT5;

        }else{
            __enable_interrupt();               // The RTC interrupt should trigger now...
        }

    }
    else
    {
        P3IFG &= ~BIT1;
        // Device powered up from a cold start.
        // It configures the device and puts the device into LPM3.5

        // Initialize RTC
        // Interrupt and reset happen every 1024/32768 * 32 = 1 sec.
        RTCMOD = 4-1;//64-1;
        RTCCTL = RTCSS__XT1CLK | RTCSR | RTCPS__1024 | RTCIE;

    }


    P3IE |= BIT1; // allow interrupt from "base detect"
    // Enter LPM3.5 mode with interrupts enabled. Note that this operation does
    // not return. The LPM3.5 will exit through a RESET event, resulting in a
    // re-start of the code.
    PMMCTL0_H = PMMPW_H;                    // Open PMM Registers for write
    PMMCTL0_L |= PMMREGOFF;                 // and set PMMREGOFF
    __bis_SR_register(LPM3_bits | GIE);
    __no_operation();

    return 0;
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_A0_VECTOR))) USCI_A0_ISR (void)
#else
#error Compiler not supported!
#endif
{
    switch(__even_in_range(UCA0IV,USCI_UART_UCTXCPTIFG))
    {
        case USCI_NONE: // No interrupt pending
            //P3OUT |= BIT6; // (bit 8 de Digilent)
            break;

        case USCI_UART_UCRXIFG: // Receive buffer full
              UCA0IFG &= ~UCRXIFG;            // Clear interrupt

              if (UCA0STATW & UCRXERR){
                  if(UCA0STATW & UCFE)
                      P2OUT |= BIT7; // (bit 11 de Digilent)
                  if(UCA0STATW & UCOE)
                      P3OUT |= BIT5; // (bit 10 de Digilent)
                  if(UCA0STATW & UCPE)
                      P3OUT |= BIT2; // (bit 9 de Digilent)
              }


              RXData = UCA0RXBUF;             // Clear buffer

              if(UCA0STATW & UCOE)
                  P3OUT |= BIT6; // (bit 8 de Digilent)

              P2OUT     |=   BIT5;
              __bic_SR_register_on_exit(LPM0_bits); // Exit LPM0 on reti
              break;
        case USCI_UART_UCTXIFG: // Transmit buffer empty
            //P3OUT |= BIT2; // (bit 9 de Digilent)
            break;

        case USCI_UART_UCSTTIFG: // Start bit received
            //P3OUT |= BIT5; // (bit 10 de Digilent)
            break;

        case USCI_UART_UCTXCPTIFG: // Transmit complete
            //P2OUT |= BIT7; // (bit 11 de Digilent)
            break;

    }
}

void initGpio(void)
{
    // PxDIR: 0 = Input; 1 = Output
    PADIR = 0xFFFF; PBDIR = 0xFFFF; PCDIR = 0xFFFF; //P1DIR = 0xFF; P2DIR = 0xFF; P3DIR = 0xFF; P4DIR = 0xFF; P5DIR = 0xFF; P6DIR = 0xFF;
    // PxREN = 1 -> pullup/pulldown resistor
    PAREN = 0xFFFF; PBREN = 0xFFFF; PCREN = 0xFFFF; //P1REN = 0xFF; P2REN = 0xFF; P3REN = 0xFF; P4REN = 0xFF; P5REN = 0xFF; P6REN = 0xFF;
    // PxOUT: self-explanatory
    PAOUT = 0x0000; PBOUT = 0x0000; PCOUT = 0x0000; //P1OUT = 0x00; P2OUT = 0x00; P3OUT = 0x00; P4OUT = 0x00; P5OUT = 0x00; P6OUT = 0x00;

    // Initialize XT1 32kHz crystal
    P2SEL0    |=  (BIT0|BIT1);  // P2.0~P2.1: crystal pins
    // P2SEL1 &= ~(BIT0|BIT1);

    // Base, Agua
    // P2SEL0 &= ~(BIT4|BIT5);
    // P2SEL1 &= ~(BIT4|BIT5);  // P2SEL = 00 ( GPIO )
    // P2DIR  |=  (BIT4|BIT5);  // P2DIR = 11 (output)
    // P2OUT  &= ~(BIT4|BIT5);  // P2OUT = 00 ( = 0V )

    // Base_detect
    // P3SEL0 &= ~BIT1
    // P3SEL1 &= ~BIT1
    P3DIR     &= ~BIT1; // (input)
    // P3REN  |=  BIT1; // ( with R )
    // P3OUT  &= ~BIT1; // (pulldown)
    P3IES     &= ~BIT1; // (flag set with low-to-high transition)

    // TX,RX
    // P1SEL0 &= ~(BIT4|BIT5);
    // P1SEL1 &= ~(BIT4|BIT5); // P1SEL = 00 ( GPIO )
    // P1DIR  |=  (BIT4|BIT5); // (output)
    // P1OUT  &= ~(BIT4|BIT5); // ( = 0V )

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

            P1DIR  &= ~BIT5; // (RX = pulldown)
            P3REN  &= ~BIT1; // (BD = HiZ - without R)
            P1OUT  |=  BIT4; // (TX = 3V)

            P2OUT ^= BIT5;
            P2OUT ^= BIT5;
            if (P1IN & BIT5){
                P2OUT |= BIT5;
            }else{
                P2OUT &=~BIT5;
            }
            P2OUT ^= BIT5;
            P2OUT ^= BIT5;


            P3REN  |=  BIT1; // (BD = pulldown)
            P1OUT  &= ~BIT4; // (TX = 0V)
            P1DIR  |=  BIT5; // (RX = 0V)

            break;
        default:          break;
    }
}

// Port 3 interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=PORT3_VECTOR
__interrupt void Port_3(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(PORT3_VECTOR))) Port_3 (void)
#else
#error Compiler not supported!
#endif
{
    P3IFG &= ~BIT1;                         // Clear P3.1 IFG

}


void Software_Trim()
{
    unsigned int oldDcoTap = 0xffff;
    unsigned int newDcoTap = 0xffff;
    unsigned int newDcoDelta = 0xffff;
    unsigned int bestDcoDelta = 0xffff;
    unsigned int csCtl0Copy = 0;
    unsigned int csCtl1Copy = 0;
    unsigned int csCtl0Read = 0;
    unsigned int csCtl1Read = 0;
    unsigned int dcoFreqTrim = 3;
    unsigned char endLoop = 0;

    do
    {
        CSCTL0 = 0x100;                         // DCO Tap = 256
        do
        {
            CSCTL7 &= ~DCOFFG;                  // Clear DCO fault flag
        }while (CSCTL7 & DCOFFG);               // Test DCO fault flag

        __delay_cycles((unsigned int)3000 * MCLK_FREQ_MHZ);// Wait FLL lock status (FLLUNLOCK) to be stable
                                                           // Suggest to wait 24 cycles of divided FLL reference clock
        while((CSCTL7 & (FLLUNLOCK0 | FLLUNLOCK1)) && ((CSCTL7 & DCOFFG) == 0));

        csCtl0Read = CSCTL0;                   // Read CSCTL0
        csCtl1Read = CSCTL1;                   // Read CSCTL1

        oldDcoTap = newDcoTap;                 // Record DCOTAP value of last time
        newDcoTap = csCtl0Read & 0x01ff;       // Get DCOTAP value of this time
        dcoFreqTrim = (csCtl1Read & 0x0070)>>4;// Get DCOFTRIM value

        if(newDcoTap < 256)                    // DCOTAP < 256
        {
            newDcoDelta = 256 - newDcoTap;     // Delta value between DCPTAP and 256
            if((oldDcoTap != 0xffff) && (oldDcoTap >= 256)) // DCOTAP cross 256
                endLoop = 1;                   // Stop while loop
            else
            {
                dcoFreqTrim--;
                CSCTL1 = (csCtl1Read & (~DCOFTRIM)) | (dcoFreqTrim<<4);
            }
        }
        else                                   // DCOTAP >= 256
        {
            newDcoDelta = newDcoTap - 256;     // Delta value between DCPTAP and 256
            if(oldDcoTap < 256)                // DCOTAP cross 256
                endLoop = 1;                   // Stop while loop
            else
            {
                dcoFreqTrim++;
                CSCTL1 = (csCtl1Read & (~DCOFTRIM)) | (dcoFreqTrim<<4);
            }
        }

        if(newDcoDelta < bestDcoDelta)         // Record DCOTAP closest to 256
        {
            csCtl0Copy = csCtl0Read;
            csCtl1Copy = csCtl1Read;
            bestDcoDelta = newDcoDelta;
        }

    }while(endLoop == 0);                      // Poll until endLoop == 1

    CSCTL0 = csCtl0Copy;                       // Reload locked DCOTAP
    CSCTL1 = csCtl1Copy;                       // Reload locked DCOFTRIM
    while(CSCTL7 & (FLLUNLOCK0 | FLLUNLOCK1)); // Poll until FLL is locked
}

/*
#pragma vector = ECOMP0_VECTOR
__interrupt void ECOMP0_ISR(void){
    current_isq = "RTC"
}
*/

