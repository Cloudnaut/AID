#include "AT_Interface_Driver.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#pragma warning(disable:4996)

#define AT_MAX_PARAMETERS_LENGTH 150

#define AT_MSG_OK "OK\r\n"
#define AT_MSG_SEND_OK "SEND OK\r\n"
#define AT_MSG_SEND_RECV "Recv "
#define AT_MSG_WIFI_CONNECTED "WIFI CONNECTED\r\n"
#define AT_MSG_INIT_SEND_OK "> "

#define AT_CMD_TEST "AT"
#define AT_CMD_ECHO_ENABLE "ATE1"
#define AT_CMD_ECHO_DISABLE "ATE0"
#define AT_CMD_WIFI_CONNECT "AT+CWJAP_CUR"
#define AT_CMD_RESTART "AT+RST"
#define AT_CMD_RESTORE "AT+RESTORE"
#define AT_CMD_WIFI_MODE "AT+CWMODE_CUR"
#define AT_CMD_DHCP_MODE "AT+CWDHCP_CUR"
#define AT_CMD_MULTI "AT+CIPMUX"
#define AT_CMD_IP_CONNECT "AT+CIPSTART"
#define AT_CMD_IP_TRANS_MODE "AT+CIPMODE"
#define AT_CMD_IP_CLOSE "AT+CIPCLOSE"
#define AT_CMD_INIT_SEND "AT+CIPSEND"


void ClearBuffer(struct AT_Interface interface)
{
	memset(interface.buffer, '\0', interface.bufferSize);
}

void Log(struct AT_Interface interface, uint8_t *log)
{
	if(interface.logCallback != NULL)
		interface.logCallback(log);
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

enum Result ParseForPayload(uint8_t *haystack, uint8_t *payload)
{
	uint8_t *cursor = haystack;
	while(*cursor != '\0' && *cursor != '+')
		cursor++;
	
	if(StringStartsWith(cursor, "+IPD"))
	{
		while (*cursor != ':')
			cursor++;
		cursor++;

		strcpy(payload, cursor);
		return Success;
	}
	
	return Error;
}

enum Result SendExecuteCommand(struct AT_Interface interface, uint8_t *cmd, uint8_t *expectedResult)
{
	ClearBuffer(interface);
	uint32_t requestLength = sprintf(interface.buffer, "%s%s", cmd, AT_EOF);
	
	Log(interface, interface.buffer);
	
	interface.sendCommandCallback(interface.buffer, requestLength);

	ClearBuffer(interface);
	interface.receiveCommandCallback(interface.buffer, interface.bufferSize);

	Log(interface, interface.buffer);
	
	if (StringEndsWith(interface.buffer, expectedResult))
		return Success;
	return Error;
}

enum Result SendSetCommand(struct AT_Interface interface, uint8_t *cmd, uint8_t *params, uint8_t *expectedResult)
{
	ClearBuffer(interface);
	uint32_t requestLength = sprintf(interface.buffer, "%s=%s%s", cmd, params, AT_EOF);
	
	Log(interface, interface.buffer);
	
	interface.sendCommandCallback(interface.buffer, requestLength);

	ClearBuffer(interface);
	interface.receiveCommandCallback(interface.buffer, interface.bufferSize);

	Log(interface, interface.buffer);
	
	if (StringEndsWith(interface.buffer, expectedResult))
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

enum Result AT_InitInterface(struct AT_Interface interface)
{
	if (!AT_DisableEcho(interface))
		return Error;
	if (!AT_Restore(interface))
		return Error;
	if (!AT_DisableEcho(interface))
		return Error;
	if (!AT_SetStationMode(interface))
		return Error;
	if (!AT_DisableMultiConnection(interface))
		return Error;
	return Success;
}

enum Result AT_TestInterfaceConnection(struct AT_Interface interface)
{
	return SendExecuteCommand(interface, AT_CMD_TEST, AT_MSG_OK);
}

enum Result AT_EnableEcho(struct AT_Interface interface)
{
	return SendExecuteCommand(interface, AT_CMD_ECHO_ENABLE, AT_MSG_OK);
}

enum Result AT_DisableEcho(struct AT_Interface interface)
{
	return SendExecuteCommand(interface, AT_CMD_ECHO_DISABLE, AT_MSG_OK);
}

enum Result AT_Restart(struct AT_Interface interface)
{
	return SendExecuteCommand(interface, AT_CMD_RESTART, AT_MSG_OK);
}

enum Result AT_Restore(struct AT_Interface interface)
{
	enum Result result = SendExecuteCommand(interface, AT_CMD_RESTORE, AT_MSG_OK);
	
	interface.sleepCallback(2500);
	
	return result;
}

enum Result AT_SetStationMode(struct AT_Interface interface)
{
	return SendSetCommand(interface, AT_CMD_WIFI_MODE, "1", AT_MSG_OK);
}

enum Result AT_ConnectWifi(struct AT_Interface interface, uint8_t* ssid, uint8_t* passwd)
{
	uint8_t parameters[AT_MAX_PARAMETERS_LENGTH];
	sprintf(parameters, "\"%s\",\"%s\"", ssid, passwd);
	return SendSetCommand(interface, AT_CMD_WIFI_CONNECT, parameters, AT_MSG_WIFI_CONNECTED);
}

enum Result AT_EnableDHCP(struct AT_Interface interface)
{
	uint8_t parameters[AT_MAX_PARAMETERS_LENGTH];
	sprintf(parameters, "%u,%u", 1, 1);
	return SendSetCommand(interface, AT_CMD_DHCP_MODE, parameters, AT_MSG_OK);
}

enum Result AT_DisableDHCP(struct AT_Interface interface)
{
	uint8_t parameters[AT_MAX_PARAMETERS_LENGTH];
	sprintf(parameters, "%u,%u", 1, 0);
	return SendSetCommand(interface, AT_CMD_DHCP_MODE, parameters, AT_MSG_OK);
}

enum Result AT_DisableMultiConnection(struct AT_Interface interface)
{
	return SendSetCommand(interface, AT_CMD_MULTI, "0", AT_MSG_OK);
}

enum Result AT_SetTransparentTransmissionMode(struct AT_Interface interface)
{
	return SendSetCommand(interface, AT_CMD_IP_TRANS_MODE, "1", AT_MSG_OK);
}

enum Result AT_ConnectTCP(struct AT_Interface interface, uint8_t* host, uint16_t port)
{	
	uint8_t parameters[AT_MAX_PARAMETERS_LENGTH];
	sprintf(parameters, "\"TCP\",\"%s\",%u", host, port);
	return SendSetCommand(interface, AT_CMD_IP_CONNECT, parameters, AT_MSG_OK);
}

enum Result AT_CloseTCP(struct AT_Interface interface)
{
	return SendExecuteCommand(interface, AT_CMD_IP_CLOSE, AT_MSG_OK);
}

enum Result AT_SendPayload(struct AT_Interface interface, uint8_t* payload, uint8_t *responsePayload)
{	
	size_t payloadLength = strlen(payload);

	uint8_t parameters[AT_MAX_PARAMETERS_LENGTH];
	sprintf(parameters, "%u", payloadLength);
	
	if (!SendSetCommand(interface, AT_CMD_INIT_SEND, parameters, AT_MSG_INIT_SEND_OK))
		return Error;

	Log(interface, payload);
	
	ClearBuffer(interface);
	interface.sendCommandCallback(payload, payloadLength);

	ClearBuffer(interface);
	interface.receiveCommandCallback(interface.buffer, interface.bufferSize);
	
	Log(interface, interface.buffer);

	if (!StringStartsWith(interface.buffer, AT_MSG_SEND_RECV))
		return Error;

	ClearBuffer(interface);
	interface.receiveCommandCallback(interface.buffer, interface.bufferSize);
	
	Log(interface, interface.buffer);
	Log(interface, "\r\n");
	
	if (!StringStartsWith(interface.buffer, AT_MSG_SEND_OK))
		return Error;
	
	ParseForPayload(interface.buffer, responsePayload);
	
	return Success;
}

enum Result AT_ReceivePayload(struct AT_Interface interface, uint8_t* payload)
{
	ClearBuffer(interface);
	interface.receiveCommandCallback(interface.buffer, interface.bufferSize);

	Log(interface, interface.buffer);
	Log(interface, "\r\n");
	
	return ParseForPayload(interface.buffer, payload);
}