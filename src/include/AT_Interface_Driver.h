#ifndef AT_INTERFACE_DRIVER
#define AT_INTERFACE_DRIVER
#include <stdint.h>

#define AT_EOF "\r\n"

enum Result
{
	Success = 1,
	Error = 0
};

struct AT_Interface
{
	uint8_t *buffer;
	uint16_t bufferSize;
	uint32_t defaultResponseTimeout;
	void (*sendCommandCallback)(uint8_t * buffer, uint16_t size);
	enum Result (*receiveCommandCallback)(uint8_t * buffer, uint16_t size, uint32_t timeout);
	void (*sleepCallback)(uint32_t milliseconds);
	void (*logCallback) (uint8_t *log);
};

enum Result AT_InitInterface(struct AT_Interface interface);
enum Result AT_TestInterfaceConnection(struct AT_Interface interface);
enum Result AT_EnableEcho(struct AT_Interface interface);
enum Result AT_DisableEcho(struct AT_Interface interface);
enum Result AT_Restart(struct AT_Interface interface);
enum Result AT_Restore(struct AT_Interface interface);
enum Result AT_SetStationMode(struct AT_Interface interface);
enum Result AT_ConnectWifi(struct AT_Interface interface, uint8_t *ssid, uint8_t *passwd);
enum Result AT_EnableDHCP(struct AT_Interface interface);
enum Result AT_DisableDHCP(struct AT_Interface interface);
enum Result AT_DisableMultiConnection(struct AT_Interface interface);
enum Result AT_SetTransparentTransmissionMode(struct AT_Interface interface);
enum Result AT_ConnectTCP(struct AT_Interface interface, uint8_t *host, uint16_t port);
enum Result AT_CloseTCP(struct AT_Interface interface);
enum Result AT_SendPayload(struct AT_Interface interface, uint8_t *payload, uint8_t *responsePayload);
enum Result AT_ReceivePayload(struct AT_Interface interface, uint8_t *payload, uint32_t responseTimeout);

#endif //AT_INTERFACE_DRIVER
