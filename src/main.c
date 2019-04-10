#include "AT_Interface_Driver.h"
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
			continue;

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

	while(1)
	{
		AT_ConnectTCP(interface, "host", 123);

	}

}