#include "../../include/AT_Interface_Driver.h"
#include <stdio.h>
#include <string.h>


void sendCallback(uint8_t * bytes, uint16_t size)
{
	uint8_t *cursor = bytes;
	for(int i = 0; i < size; i++)
	{
		putchar(*cursor);
		cursor++;
	}
}

void receiveCallback(uint8_t * bytes, uint16_t size)
{
	int currentChar = 0;
	uint8_t *cursor = bytes;

	while(4 != (currentChar = getchar()))
	{
		if (currentChar == '\n')
		{
			*cursor = '\r';
			cursor++;
			*cursor = '\n';
			cursor++;
			continue;
		}
			
		*cursor = currentChar;
		cursor++;
	}
}

int main()
{
	uint8_t buffer[2048];
	struct AT_Interface interface;
	interface.buffer = buffer;
	interface.bufferSize = sizeof(buffer);
	interface.sendCommandCallback = sendCallback;
	interface.receiveCommandCallback = receiveCallback;

	uint8_t payload[2048];
	memset(payload, '\0', sizeof(payload));
	uint8_t responsePayload[2048];
	memset(responsePayload, '\0', sizeof(responsePayload));
	

	AT_InitInterface(interface);

	while(1)
	{
		while(!AT_ConnectWifi(interface, "premium ssid", "epic password")) {}
		while (!AT_ConnectTCP(interface, "lol.kek.de", 232)) {}

		while (!AT_SendPayload(interface, "kekerino ist lolerino", responsePayload)) {}

		while (!AT_CloseTCP(interface)) {}

		getchar();
		getchar();
	}

}