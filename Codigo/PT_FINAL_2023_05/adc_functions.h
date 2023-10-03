#ifndef ADC_FUNCTIONS_H_
#define ADC_FUNCTIONS_H_

unsigned int measure_internal_ref(void);
int ref_to_vcc_conversion(unsigned int ref);

unsigned int take_temperature(void);
int temperature_conversion(unsigned int t);

#endif /* ADC_FUNCTIONS_H_ */
