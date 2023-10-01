#include <adc_functions.h>
#include <msp430.h>
#include <my_config.h>

unsigned int measure_internal_ref(void){
    unsigned int temp = 0;
    // Configure ADC12
    ADCCTL0 &= ~ADCENC;                     // Disable ADC
    ADCCTL0 = ADCSHT_2 | ADCON;             // ADCON, S&H=16 ADC clks
    ADCCTL1 = ADCSHP;                       // ADCCLK = MODOSC; sampling timer
    ADCCTL2 &= ~ADCRES;                     // clear ADCRES in ADCCTL
    ADCCTL2 = ADCRES_2;                     // 12-bit conversion results
    //ADCIE = ADCIE0;                         // Enable ADC conv complete interrupt
    ADCMCTL0 = ADCINCH_13 | ADCSREF_0;      // A13 ADC input select = 1.5V Ref
                                            // Vref = DVCC
    // Configure reference module located in the PMM
    PMMCTL0_H = PMMPW_H;                    // Unlock the PMM registers
    PMMCTL2 = INTREFEN | REFVSEL_0;         // Enable internal 1.5V reference
    __delay_cycles(400);                    // Delay for reference settling
    while(!(PMMCTL2 & REFGENRDY));          // Poll till internal reference settles

    ADCCTL0 |= ADCENC | ADCSC;                                    // Sampling and conversion start
    while (!(ADCIFG & ADCIFG0));

    temp = ADCMEM0;

    ADCCTL0 &= ~ADCENC;
    ADCCTL0 &= ~ADCON;
    PMMCTL0_H = PMMPW_H;
    PMMCTL2 &= ~(INTREFEN);

    return temp;
}

int ref_to_vcc_conversion(unsigned int ref){
    unsigned short temp;
    temp = ( ((unsigned long) ref * (signed long) ADC_GAIN) >> 15) + (signed short)(ADC_OFFSET);
    // ADCResult = ( ((unsigned long) ADCMEM0 * (signed long) ADC_Gain) >> 15) + (signed short)(ADC_Offset);
    return (int)(((unsigned long)4095 * (unsigned long)150) / (unsigned long) (temp));
}


unsigned int take_temperature(void){
    unsigned int temp = 0;
    // Configure ADC - Pulse sample mode; ADCSC trigger
    ADCCTL0 |= ADCSHT_8 | ADCON;                                  // ADC ON,temperature sample period>30us
    ADCCTL1 |= ADCSHP;                                            // s/w trig, single ch/conv, MODOSC
    ADCCTL2 &= ~ADCRES;                                         // clear ADCRES in ADCCTL
    ADCCTL2 |= ADCRES_2;                                          // 12-bit conversion results
    ADCMCTL0 |= ADCSREF_1 | ADCINCH_12;                           // ADC input ch A12 => temp sense
    //ADCIE |=ADCIE0;                                               // Enable the Interrupt request for a completed ADC_B conversion
    // Configure reference
    PMMCTL0_H = PMMPW_H;                                          // Unlock the PMM registers
    PMMCTL2 |= INTREFEN | TSENSOREN | REFVSEL_0;                  // Enable internal 1.5V reference and temperature sensor
    __delay_cycles(400);                                          // Delay for reference settling

    ADCCTL0 |= ADCENC | ADCSC;                                    // Sampling and conversion start
    while (!(ADCIFG & ADCIFG0));

    //SYSCFG0 = FRWPPW | DFWP;
    temp = ADCMEM0;
    //SYSCFG0 = FRWPPW | PFWP | DFWP;

    ADCCTL0 &= ~ADCENC;
    ADCCTL0 &= ~ADCON;
    PMMCTL0_H = PMMPW_H;
    PMMCTL2 &= ~(INTREFEN | TSENSOREN);

    return temp;
    //temperature_taken = (temp-CALADC_15V_30C)*(85-30)/(CALADC_15V_85C-CALADC_15V_30C)+30;

}

int temperature_conversion(unsigned int t){
    int answer = 0;
    answer = (int)((((float)(t)-CALADC_15V_30C)*(105-30)/(CALADC_15V_105C-CALADC_15V_30C)+30)*100.);
    return answer;
}
