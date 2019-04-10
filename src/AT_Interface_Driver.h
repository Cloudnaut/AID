#ifndef AT_INTERFACE_DRIVER
#define AT_INTERFACE_DRIVER
#include <stdint.h>

#define AT_EOF "\r\n"

struct AT_Interface
{
	uint8_t *buffer;
	uint16_t bufferSize;
	void (*sendCommandCallback)(uint8_t * buffer, uint16_t size);
	void (*receiveCommandCallback)(uint8_t * buffer, uint16_t size);
};

enum Result
{
	Success = 1,
	Error = 0
};

enum Result AT_TestInterfaceConnection(struct AT_Interface interface);
void AT_EnableEcho(struct AT_Interface interface);
void AT_DisableEcho(struct AT_Interface interface);
void AT_Restart(struct AT_Interface interface);
void AT_EnableAutoConnect(struct AT_Interface interface);
void AT_DisableAutoConnect(struct AT_Interface interface);
void AT_ConnectWifi(struct AT_Interface interface, uint8_t *ssid, uint8_t *passwd);
void AT_DisableMultiConnection(struct AT_Interface interface);
void AT_ConnectTCP(struct AT_Interface interface, uint8_t *host, uint16_t port);
void AT_CloseTCP(struct AT_Interface interface);
enum Result AT_SendPayload(struct AT_Interface interface, uint8_t *payload);
enum Result AT_ReceivePayload(struct AT_Interface interface, uint8_t *payload);

#endif //AT_INTERFACE_DRIVER
