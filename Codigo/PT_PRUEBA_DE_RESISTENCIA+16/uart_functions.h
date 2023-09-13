#ifndef UART_FUNCTIONS_H_
#define UART_FUNCTIONS_H_

#define MCLK_FREQ_MHZ 1                     // MCLK = 1MHz

void Software_Trim();
void tx_string(const char * string);

#endif /* UART_FUNCTIONS_H_ */
