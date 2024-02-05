#ifndef UART_H_
#define UART_H_

#ifdef __cplusplus
  extern "C" {
#endif

#include "main.h"

void hm10_uart_init(void);
void debug_uart_init(void);
void uart_write(int ch);


#ifdef __cplusplus
  }
#endif

#endif /* UART_H_ */
