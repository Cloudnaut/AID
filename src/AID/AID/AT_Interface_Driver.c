#include "AT_Interface_Driver.h"
#include <stdlib.h>
#include <string.h>

#pragma warning(disable:4996)

#define AT_OK "OK"
#define AT_TEST_REQUEST "AT\r\n"
#define AT_ECHO_REQUEST_ENABLE "ATE1\r\n"
#define AT_ECHO_REQUEST_DISABLE "ATE0\r\n"
#define AT_WIFI_CONNECT_REQUEST "AT+CWJAP"
#define AT_AUTO_CONNECT_ENABLE_REQUEST "AT+CWMODE=1\r\n"
#define AT_AUTO_CONNECT_DISABLE_REQUEST "AT+CWMODE=0\r\n"
#define AT_RESTART_REQUEST "AT+RST\r\n"
#define AT_MULTI_REQUEST_ENABLE "AT+CIPMUX=1\r\n"
#define AT_MULTI_REQUEST_DISABLE "AT+CIPMUX=0\r\n"
#define AT_IP_CONNECT_REQUEST "AT+CIPSTART"
#define AT_IP_CLOSE_REQUEST "AT+CIPCLOSE=0\r\n"
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

enum Result AT_TestInterfaceConnection(struct AT_Interface interface)
{
	ClearBuffer(interface);
	interface.sendCommandCallback(AT_TEST_REQUEST, sizeof(AT_TEST_REQUEST) - 1);
	interface.receiveCommandCallback(interface.buffer, interface.bufferSize);

	if (IsString(interface.buffer, AT_OK))
		return Success;
	return Error;
}

void AT_EnableEcho(struct AT_Interface interface)
{
	interface.sendCommandCallback(AT_ECHO_REQUEST_ENABLE, sizeof(AT_ECHO_REQUEST_ENABLE) - 1);
}

void AT_DisableEcho(struct AT_Interface interface)
{
	interface.sendCommandCallback(AT_ECHO_REQUEST_DISABLE, sizeof(AT_ECHO_REQUEST_DISABLE) - 1);
}

void AT_Restart(struct AT_Interface interface)
{
	interface.sendCommandCallback(AT_RESTART_REQUEST, sizeof(AT_RESTART_REQUEST) - 1);
}

void AT_EnableAutoConnect(struct AT_Interface interface)
{
	interface.sendCommandCallback(AT_AUTO_CONNECT_ENABLE_REQUEST, sizeof(AT_AUTO_CONNECT_ENABLE_REQUEST) - 1);
	AT_Restart(interface);
}

void AT_DisableAutoConnect(struct AT_Interface interface)
{
	interface.sendCommandCallback(AT_AUTO_CONNECT_DISABLE_REQUEST, sizeof(AT_AUTO_CONNECT_DISABLE_REQUEST) - 1);
	AT_Restart(interface);
}

void AT_ConnectWifi(struct AT_Interface interface, uint8_t* ssid, uint8_t* passwd)
{
	ClearBuffer(interface);
	size_t ssidLength = strlen(ssid);
	size_t passwdLength = strlen(passwd);

	uint8_t *cursor = interface.buffer;

	memcpy(cursor, AT_WIFI_CONNECT_REQUEST, sizeof(AT_WIFI_CONNECT_REQUEST) - 1);
	cursor += sizeof(AT_WIFI_CONNECT_REQUEST) - 1;

	memcpy(cursor, "=\"", 2);
	cursor += 2;

	memcpy(cursor, ssid, ssidLength);
	cursor += ssidLength;

	memcpy(cursor, "\",\"", 3);
	cursor += 3;

	memcpy(cursor, passwd, passwdLength);
	cursor += passwdLength;

	memcpy(cursor, "\"\r\n", 3);
	cursor += 3;

	interface.sendCommandCallback(interface.buffer, cursor - interface.buffer);
}

void AT_DisableMultiConnection(struct AT_Interface interface)
{
	interface.sendCommandCallback(AT_MULTI_REQUEST_DISABLE, sizeof(AT_MULTI_REQUEST_DISABLE) - 1);
}

void AT_ConnectTCP(struct AT_Interface interface, uint8_t* host, uint16_t port)
{
	AT_DisableMultiConnection(interface);
	ClearBuffer(interface);

	size_t hostLength = strlen(host);

	uint8_t *cursor = interface.buffer;

	memcpy(cursor, AT_IP_CONNECT_REQUEST, sizeof(AT_IP_CONNECT_REQUEST) - 1);
	cursor += sizeof(AT_IP_CONNECT_REQUEST) - 1;

	memcpy(cursor, "=\"TCP\",\"", 8);
	cursor += 8;

	memcpy(cursor, host, hostLength);
	cursor += hostLength;

	memcpy(cursor, "\",", 2);
	cursor += 2;

	uint8_t portString[5];
	itoa(port, portString, 10);
	memcpy(cursor, portString, strlen(portString));
	cursor += strlen(portString);

	memcpy(cursor, "\r\n", 2);
	cursor += 2;

	interface.sendCommandCallback(interface.buffer, cursor - interface.buffer);
}

void AT_CloseTCP(struct AT_Interface interface)
{
	interface.sendCommandCallback(AT_IP_CLOSE_REQUEST, sizeof(AT_IP_CLOSE_REQUEST) - 1);
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