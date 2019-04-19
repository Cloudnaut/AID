#ifndef AID_HAL_STM32F4XX
#define AID_HAL_STM32F4XX

#include "AT_Interface_Driver.h"
#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_uart.h"
#include "stm32f4xx_hal_usart.h"

void AID_Init_STM32F4xx(struct AT_Interface *interface, uint32_t GPIO_PIN_TX, uint32_t GPIO_PIN_RX, uint32_t GPIO_ALTERNATE, GPIO_TypeDef *GPIOx, USART_TypeDef *USART, uint8_t *buffer, size_t bufferSize);


#endif //AID_HAL_STM32F4XX