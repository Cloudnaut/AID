 # AID
 AT-Firmware Interface Driver - Firmware Abstraction Layer
 
 This is a simple and minimalistic abstraction layer for interfacing the AT-Firmware which is developed by espressif and used by the well known ESP8266 wifi module. It's designed to run hardware independent to support as many embedded systems as possible. There are no dependencies to other libraries or operating system functions.
 
 ## How does it work?
 By using only a minimal set of header files and inversion of control the driver is able to work absolutely hardware independent. Only system specific functions have to be implemented. These functions are passed to the driver so it can communicate with the ESP8266 wifi module.