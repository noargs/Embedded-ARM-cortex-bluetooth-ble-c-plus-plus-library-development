#ifndef MAIN_H_
#define MAIN_H_

#ifdef __cplusplus
  extern "C" {
#endif

#include "stm32f4xx_hal.h"

#define HM10_PORT               GPIOA
#define HM10_TX_PORT            GPIOA
#define HM10_RX_PORT            GPIOA
#define HM10_RX_PIN             GPIO_PIN_10  // (1U<<10) = 0x0400 = 0b 0000 0100 0000 0000
#define HM10_TX_PIN             GPIO_PIN_9

#define DEBUG_PORT              GPIOA
#define DEBUG_RX_PIN            GPIO_PIN_3
#define DEBUG_TX_PIN            GPIO_PIN_2


#ifdef __cplusplus
  }
#endif

#endif /* MAIN_H_ */
