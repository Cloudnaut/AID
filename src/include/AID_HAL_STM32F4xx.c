#include "AID_HAL_STM32F4xx.h"

static GPIO_InitTypeDef GPIOInit;
void InitESPUARTPins(GPIO_TypeDef *GPIOx, uint32_t GPIO_PIN_TX, uint32_t GPIO_PIN_RX, uint32_t GPIO_ALTERNATE)
{
	//Common settings
	GPIOInit.Pull = GPIO_NOPULL;
	GPIOInit.Speed = GPIO_SPEED_HIGH;
	
	GPIOInit.Alternate = GPIO_ALTERNATE; //ESP connected to USART
	//TX Pin FOR ESP
	GPIOInit.Pin = GPIO_PIN_TX;
	GPIOInit.Mode = GPIO_MODE_AF_PP;
	HAL_GPIO_Init(GPIOx, &GPIOInit);
	
	//RX Pin FOR ESP
	GPIOInit.Pin = GPIO_PIN_RX;
	GPIOInit.Mode = GPIO_MODE_AF_OD;
	HAL_GPIO_Init(GPIOx, &GPIOInit);
}

static UART_HandleTypeDef UART_ESP_Handle;
uint32_t InitESPUART(USART_TypeDef *USART)
{
	UART_ESP_Handle.Instance = USART;
	UART_ESP_Handle.Init.BaudRate = 115200;
	UART_ESP_Handle.Init.WordLength = UART_WORDLENGTH_8B;
	UART_ESP_Handle.Init.StopBits = UART_STOPBITS_1;
	UART_ESP_Handle.Init.Parity = UART_PARITY_NONE;
	UART_ESP_Handle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	UART_ESP_Handle.Init.Mode = UART_MODE_TX_RX;
	
	return HAL_UART_Init(&UART_ESP_Handle);
}

#define AT_MAX_DELAY 1
void sendESPCallback(uint8_t *bytes, uint16_t size) //Callback method to abstract sending data to ESP
{
	uint8_t buffer[1];
	while(HAL_UART_Receive(&UART_ESP_Handle, buffer, sizeof(buffer), AT_MAX_DELAY) == HAL_OK); //Flush bus
	
	HAL_Delay(100);
	
	HAL_UART_Transmit(&UART_ESP_Handle, bytes, size, HAL_MAX_DELAY);
}

enum Result receiveESPCallback(uint8_t *response, uint16_t size, uint32_t timeout) //Callback method to abstract receiving data from ESP
{
	uint8_t buffer[1]; //One byte buffer to read the stream byte by byte
	uint8_t *responseCursor = response;
	if(HAL_UART_Receive(&UART_ESP_Handle, buffer, sizeof(buffer), timeout) != HAL_OK)
		return Error;
	while(*buffer == '\r' || *buffer == '\n') //Skip linefeeds and linebreaks
	{
		if(HAL_UART_Receive(&UART_ESP_Handle, buffer, sizeof(buffer), timeout) != HAL_OK)
			return Error;
	}
	while((responseCursor - response) < (size - 1)) //Read message byte by byte until newline but drop before overflow
	{
		*responseCursor = *buffer;
		responseCursor++;
		if(HAL_UART_Receive(&UART_ESP_Handle, buffer, sizeof(buffer), AT_MAX_DELAY) != HAL_OK) //Stop if the ESP hasn't sent data within the timeout
			break;
	}
	
	while(HAL_UART_Receive(&UART_ESP_Handle, buffer, sizeof(buffer), AT_MAX_DELAY) == HAL_OK) //Flush remaining bytes on bus, if buffer is full
		continue;
}

void sleepCallback(uint32_t milliseconds)
{
	HAL_Delay(milliseconds);
}

void AID_Init_STM32F4xx(struct AT_Interface *interface, uint32_t GPIO_PIN_TX, uint32_t GPIO_PIN_RX, uint32_t GPIO_ALTERNATE, GPIO_TypeDef *GPIOx, USART_TypeDef *USART, uint8_t *buffer, size_t bufferSize)
{
	InitESPUARTPins(GPIOx, GPIO_PIN_TX, GPIO_PIN_RX, GPIO_ALTERNATE);
	InitESPUART(USART);
	
	interface->buffer = buffer;
	interface->bufferSize = bufferSize;
	interface->defaultResponseTimeout = HAL_MAX_DELAY;
	interface->sendCommandCallback = sendESPCallback;
	interface->receiveCommandCallback = receiveESPCallback;
	interface->sleepCallback = sleepCallback;
	
	AT_InitInterface(*interface);
}