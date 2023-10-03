#ifndef UART_FUNCTIONS_H_
#define UART_FUNCTIONS_H_

#define MCLK_FREQ_MHZ 1                     // MCLK = 1MHz

#define ACK "\x06"
#define NAK "\x15"
#define NUL "\x00"

void UART_setup();
void UART_reset();
void Software_Trim(); // esta dentro de UART_setup
void tx_string(const char * string);
void tx_int(int var);

#define SEND_ACK    tx_string(ACK)
#define SEND_NAK    tx_string(NAK)
#define SEND_NUL    tx_string(NUL)

#define SEND_BOOL(var)   tx_string("\n" #var " = "); (var)? tx_string("TRUE\n") : tx_string("FALSE\n")
#define SEND_INT(var)   tx_string("\n" #var " = "); tx_int(var); tx_string("\n");
#define SEND_STRING(var)    tx_string("\n" #var " = "); tx_string(var); tx_string("\n");

#endif /* UART_FUNCTIONS_H_ */
