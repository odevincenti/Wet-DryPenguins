#include <msp430.h>
#include <my_config.h>
#include <uart_functions.h>
#include <stdio.h>

#define AUX_STIRNG_SIZE     12
char auxiliar_string[AUX_STIRNG_SIZE];

void UART_setup(){
    //PIN_SEL_01(tx_pin);
    //PIN_SEL_01(rx_pin);

    __bis_SR_register(SCG0);                // Disable FLL
    CSCTL3 = SELREF__REFOCLK;               // Set REFO as FLL reference source
    CSCTL1 = DCOFTRIMEN_1 | DCOFTRIM0 | DCOFTRIM1 | DCORSEL_0;// DCOFTRIM=3, DCO Range = 1MHz
    CSCTL2 = FLLD_0 + 30;                   // DCODIV = 1MHz
    __delay_cycles(3);
    __bic_SR_register(SCG0);                // Enable FLL

    Software_Trim();                        // Software Trim to get the best DCOFTRIM value
    CSCTL4 = SELMS__DCOCLKDIV | SELA__REFOCLK; // set XT1(~32768Hz) as ACLK source, ACLK = 32768Hz
                                               // default DCODIV as MCLK and SMCLK source

    // Configure UART
    UCA0CTLW0 |= UCSWRST;                     // Put eUSCI in reset
    //UCA0CTLW0 |=UCSSEL__SMCLK;
    UCA0CTLW0 |= UCPEN | UCPAR__ODD | UCSPB | UCSSEL__SMCLK; // ODD PARITY BIT, 2 STOP BITS
    // Baud Rate calculation
    UCA0BR0 = 8;                              // 1000000/115200 = 8.68
    UCA0MCTLW = 0xD600;                       // 1000000/115200 - INT(1000000/115200)=0.68
                                              // UCBRSx value = 0xD6 (See UG)
    UCA0BR1 = 0;
    //UCA0CTLW0 |= UCPEN | UCPAR__ODD | UCSPB | UCRXEIE;
    UCA0CTLW0 |= UCRXEIE;
    UCA0CTLW0 &= ~UCSWRST;                    // Initialize eUSCI
    //UCA0IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt

}

void UART_reset(void){
    while((UCA0STATW & UCBUSY));
    UCA0CTLW0 = UCSWRST;

    //PIN_SEL_00(tx_pin);
    //PIN_SEL_00(rx_pin);
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


void tx_string(const char * string){
    const char * index;
    for(index = string; *index != '\0';index++){
        while(!(UCA0IFG & UCTXIFG));
        UCA0TXBUF = (unsigned int)(*index);
    }
}

void tx_int(int var){
    snprintf(auxiliar_string, AUX_STIRNG_SIZE-1,"%d",var);
    tx_string(auxiliar_string);
}
