#include "AT_Interface_Driver.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#pragma warning(disable:4996)

#define AT_MAX_PARAMETERS_LENGTH 255
#define AT_OK "OK"
#define AT_TEST_REQUEST "AT"
#define AT_ECHO_REQUEST_ENABLE "ATE1"
#define AT_ECHO_REQUEST_DISABLE "ATE0"
#define AT_WIFI_CONNECT_REQUEST "AT+CWJAP"
#define AT_AUTO_CONNECT_ENABLE_REQUEST "AT+CWMODE=1"
#define AT_AUTO_CONNECT_DISABLE_REQUEST "AT+CWMODE=0"
#define AT_RESTART_REQUEST "AT+RST"
#define AT_MULTI_REQUEST_ENABLE "AT+CIPMUX=1"
#define AT_MULTI_REQUEST_DISABLE "AT+CIPMUX=0"
#define AT_IP_CONNECT_REQUEST "AT+CIPSTART"
#define AT_IP_CLOSE_REQUEST "AT+CIPCLOSE=0"
#define AT_INIT_SEND_REQUEST "AT+CIPSEND"
#define AT_INIT_SEND_RESPONSE_SUCCESS ">"
#define AT_SEND_SUCCESS "SEND OK"


inline void ClearBuffer(struct AT_Interface interface)
{
	memset(interface.buffer, '\0', interface.bufferSize);
}

enum Result IsString(uint8_t *a, uint8_t *b)
{
	if (strcmp(a, b) == 0)
		return Success;
	return Error;
}

enum Result StringEndsWith(uint8_t *string, uint8_t *end)
{
	size_t stringLength = strlen(string);
	size_t endLength = strlen(end);

	uint8_t *cursor = string;

	if (endLength <= stringLength)
	{
		cursor += stringLength - endLength;
		return IsString(cursor, end);
	}
	return Error;
}

enum Result StringStartsWith(uint8_t *string, uint8_t *start)
{
	size_t stringLength = strlen(string);
	size_t startLength = strlen(start);

	if(startLength <= stringLength)
	{
		if (memcmp(string, start, startLength) == 0)
			return Success;
	}
	return Error;
}

enum Result SendExecuteCommand(struct AT_Interface interface, uint8_t *cmd, uint8_t *expectedResult)
{
	ClearBuffer(interface);
	uint32_t requestLength = sprintf(interface.buffer, "%s%s", cmd, AT_EOF);
	interface.sendCommandCallback(interface.buffer, requestLength);

	ClearBuffer(interface);
	interface.receiveCommandCallback(interface.buffer, interface.bufferSize);

	if (IsString(interface.buffer, expectedResult))
		return Success;
	return Error;
}

enum Result SendSetCommand(struct AT_Interface interface, uint8_t *cmd, uint8_t *params, uint8_t *expectedResult)
{
	ClearBuffer(interface);
	uint32_t requestLength = sprintf(interface.buffer, "%s=%s%s", cmd, params, AT_EOF);
	interface.sendCommandCallback(interface.buffer, requestLength);

	ClearBuffer(interface);
	interface.receiveCommandCallback(interface.buffer, interface.bufferSize);

	if (IsString(interface.buffer, expectedResult))
		return Success;
	return Error;
}

enum Result SendQueryCommand()
{
	return Error;
}

enum Result SendTestCommand()
{
	return Error;
}

enum Result AT_TestInterfaceConnection(struct AT_Interface interface)
{
	return SendExecuteCommand(interface, AT_TEST_REQUEST, AT_OK);
}

enum Result AT_EnableEcho(struct AT_Interface interface)
{
	return SendExecuteCommand(interface, AT_ECHO_REQUEST_ENABLE, AT_OK);
}

enum Result AT_DisableEcho(struct AT_Interface interface)
{
	return SendExecuteCommand(interface, AT_ECHO_REQUEST_DISABLE, AT_OK);
}

enum Result AT_Restart(struct AT_Interface interface)
{
	return SendExecuteCommand(interface, AT_RESTART_REQUEST, AT_OK);
}

enum Result AT_EnableAutoConnect(struct AT_Interface interface)
{
	return SendExecuteCommand(interface, AT_AUTO_CONNECT_ENABLE_REQUEST, AT_OK);
}

enum Result AT_DisableAutoConnect(struct AT_Interface interface)
{
	return SendExecuteCommand(interface, AT_AUTO_CONNECT_DISABLE_REQUEST, AT_OK);
}

enum Result AT_ConnectWifi(struct AT_Interface interface, uint8_t* ssid, uint8_t* passwd)
{
	char parameters[AT_MAX_PARAMETERS_LENGTH];
	sprintf(parameters, "\"%s\",\"%s\"", ssid, passwd);
	return SendSetCommand(interface, AT_WIFI_CONNECT_REQUEST, parameters, AT_OK);
}

enum Result AT_DisableMultiConnection(struct AT_Interface interface)
{
	return SendExecuteCommand(interface, AT_MULTI_REQUEST_DISABLE, AT_OK);
}

enum Result AT_ConnectTCP(struct AT_Interface interface, uint8_t* host, uint16_t port)
{
	AT_DisableMultiConnection(interface);
	
	ClearBuffer(interface);
	uint32_t requestLength = sprintf(interface.buffer, "%s=\"TCP\",\"%s\",%u%s", AT_IP_CONNECT_REQUEST, host, port, AT_EOF);

	interface.sendCommandCallback(interface.buffer, requestLength);

	return Success; //TODO: Do it right
}

enum Result AT_CloseTCP(struct AT_Interface interface)
{
	return SendExecuteCommand(interface, AT_IP_CLOSE_REQUEST, AT_OK);
}

enum Result AT_SendPayload(struct AT_Interface interface, uint8_t* payload)
{
	ClearBuffer(interface);

	size_t payloadLength = strlen(payload);

	uint8_t *cursor = interface.buffer;

	memcpy(cursor, AT_INIT_SEND_REQUEST, sizeof(AT_INIT_SEND_REQUEST) - 1);
	cursor += sizeof(AT_INIT_SEND_REQUEST) - 1;

	memcpy(cursor, "=", 1);
	cursor += 1;

	uint8_t payloadLengthString[5];
	itoa(payloadLength, payloadLengthString, 10);
	memcpy(cursor, payloadLengthString, strlen(payloadLengthString));
	cursor += strlen(payloadLengthString);

	memcpy(cursor, "\r\n", 2);
	cursor += 2;

	interface.sendCommandCallback(interface.buffer, cursor - interface.buffer);

	ClearBuffer(interface);
	interface.receiveCommandCallback(interface.buffer, interface.bufferSize);

	if(IsString(interface.buffer, AT_INIT_SEND_RESPONSE_SUCCESS))
	{
		ClearBuffer(interface);
		interface.sendCommandCallback(payload, payloadLength);

		ClearBuffer(interface);
		interface.receiveCommandCallback(interface.buffer, interface.bufferSize);

		if (StringEndsWith(interface.buffer, AT_SEND_SUCCESS))
			return Success;
	}

	return Error;
}

enum Result AT_ReceivePayload(struct AT_Interface interface, uint8_t* payload)
{
	ClearBuffer(interface);
	interface.receiveCommandCallback(interface.buffer, interface.bufferSize);

	if(StringStartsWith(interface.buffer, "+IPD"))
	{
		uint8_t *cursor = interface.buffer;
		while (*cursor != ':')
			cursor++;
		cursor++;

		strcpy(payload, cursor);
		return Success;
	}

	return Error;
}