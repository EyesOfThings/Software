/*!
    \file

    \brief     Library for common WiFi functions
*/

#ifndef WifiFunctions_H
#define WifiFunctions_H


#include <simplelink.h>
#include <rtems.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DEFAULT_SSID "Myriad2Wifi"
#define DEFAULT_PASSWORD "visilabap"
#define DEFAULT_SECURITY SL_SEC_TYPE_WPA_WPA2
#define DEFAULT_CHANNEL 8

#define SL_STOP_TIMEOUT        0xFFFF

#define pal_Memset(x,y,z)   memset((void *)x,y,z)
#define pal_Memcpy(x,y,z)   memcpy((void *)x, (const void *)y, z)
#define pal_Memcmp(x,y,z)   memcmp((const void *)x, (const void *)y, z)
#define pal_Strlen(x)       strlen((const char *)x)
#define pal_Strcmp(x,y)     strcmp((const char *)x, (const char *)y)
#define pal_Strcpy(x,y)     strcpy((char *)x, (const char *)y)
#define pal_Strstr(x,y)     strstr((const char *)x, (const char *)y)
#define pal_Strncmp(x,y,z)  strncmp((const char *)x, (const char *)y, z)
#define pal_Strcat(x,y)     strcat((char *)x, (const char *)y)

/* Status bits - These are used to set/reset the corresponding bits in a 'status_variable' */
typedef enum{
    STATUS_BIT_CONNECTION =  0, /* If this bit is:
                                 *      1 in a 'status_variable', the device is connected to the AP
                                 *      0 in a 'status_variable', the device is not connected to the AP
                                 */

    STATUS_BIT_STA_CONNECTED,    /* If this bit is:
                                  *      1 in a 'status_variable', client is connected to device
                                  *      0 in a 'status_variable', client is not connected to device
                                  */

    STATUS_BIT_IP_ACQUIRED,       /* If this bit is:
                                   *      1 in a 'status_variable', the device has acquired an IP
                                   *      0 in a 'status_variable', the device has not acquired an IP
                                   */

    STATUS_BIT_IP_LEASED,           /* If this bit is:
                                      *      1 in a 'status_variable', the device has leased an IP
                                      *      0 in a 'status_variable', the device has not leased an IP
                                      */

    STATUS_BIT_CONNECTION_FAILED,   /* If this bit is:
                                     *      1 in a 'status_variable', failed to connect to device
                                     *      0 in a 'status_variable'
                                     */

    STATUS_BIT_P2P_NEG_REQ_RECEIVED,/* If this bit is:
                                     *      1 in a 'status_variable', connection requested by remote wifi-direct device
                                     *      0 in a 'status_variable',
                                     */
    STATUS_BIT_SMARTCONFIG_DONE,    /* If this bit is:
                                     *      1 in a 'status_variable', smartconfig completed
                                     *      0 in a 'status_variable', smartconfig event couldn't complete
                                     */

    STATUS_BIT_SMARTCONFIG_STOPPED  /* If this bit is:
                                     *      1 in a 'status_variable', smartconfig process stopped
                                     *      0 in a 'status_variable', smartconfig process running
                                     */
}e_StatusBits;


#define SET_STATUS_BIT(status_variable, bit)    status_variable |= ((unsigned long)1<<(bit))
#define CLR_STATUS_BIT(status_variable, bit)    status_variable &= ~((unsigned long)1<<(bit))
#define GET_STATUS_BIT(status_variable, bit)    (0 != (status_variable & ((unsigned long)1<<(bit))))

#define IS_PING_DONE(status_variable)             GET_STATUS_BIT(status_variable, STATUS_BIT_PING_DONE)
#define IS_CONNECTED(status_variable)             GET_STATUS_BIT(status_variable, STATUS_BIT_CONNECTION)
#define IS_STA_CONNECTED(status_variable)         GET_STATUS_BIT(status_variable, STATUS_BIT_STA_CONNECTED)
#define IS_IP_ACQUIRED(status_variable)           GET_STATUS_BIT(status_variable, STATUS_BIT_IP_ACQUIRED)
#define IS_IP_LEASED(status_variable)             GET_STATUS_BIT(status_variable, STATUS_BIT_IP_LEASED)
#define IS_CONNECTION_FAILED(status_variable)     GET_STATUS_BIT(status_variable, STATUS_BIT_CONNECTION_FAILED)
#define IS_P2P_NEG_REQ_RECEIVED(status_variable)  GET_STATUS_BIT(status_variable, STATUS_BIT_P2P_NEG_REQ_RECEIVED)
#define IS_SMARTCONFIG_DONE(status_variable)      GET_STATUS_BIT(status_variable, STATUS_BIT_SMARTCONFIG_DONE)
#define IS_SMARTCONFIG_STOPPED(status_variable)   GET_STATUS_BIT(status_variable, STATUS_BIT_SMARTCONFIG_STOPPED)

/* Application specific status/error codes */
typedef enum{

    DEVICE_NOT_IN_STATION_MODE = -0x7D0,
    LAN_CONNECTION_FAILED = -0x7D0,
    SNTP_SEND_ERROR = DEVICE_NOT_IN_STATION_MODE - 1,
    SNTP_RECV_ERROR = SNTP_SEND_ERROR - 1,
    SNTP_SERVER_RESPONSE_ERROR = SNTP_RECV_ERROR - 1,
    STATUS_BIT_PING_DONE = 31,
    STATUS_CODE_MAX = -0xBB8

} e_AppStatusCodes;

typedef enum {

    NOT_CONNECTED = 0,
    MODE_AP = 1,
    MODE_STATION = 2

} ConnectionMode;

typedef struct {

    _i8 ssid_name[32];
    _i8 password[32];
    _u8 security;
    _u32 channel;
    ConnectionMode mode;

} WifiConnectionState;

/*!
    \brief This function is used for generating an Access Point

    \param[in]      ssid_name is the name of the Access point

    \param[in]      password is the password of the Access Point

    \param[in]      security is the security of the WiFi network. It can be:
                        - 0 or SL_SEC_TYPE_OPEN
                        - 1 or SL_SEC_TYPE_WEP
                        - 2 or SL_SEC_TYPE_WPA_WPA2

    \param[in]      channel is the channel where the network is generated


    \return         0 - if mode was set correctly
*/
int generateAP(char* ssid_name, char* password, _u8 security, int channel);
/*!
    \brief This function is used for generating an Access Point

    \param[in]      ssid_name is the name of the Access point

    \param[in]      password is the password of the Access Point

    \param[in]      security is the security of the WiFi network. It can be:
                        - 0 or SL_SEC_TYPE_OPEN
                        - 1 or SL_SEC_TYPE_WEP
                        - 2 or SL_SEC_TYPE_WPA_WPA2

    \param[in]      channel is the channel where the network is generated


    \return         index of the saved profile, less than 0 on error
*/
int generateAPSaveProfile(char* ssid_name, char* password, _u8 security, int channel);
/*!
    \brief  This function is used for generating an Access Point with the
            profile saved at a given index.
    \param[in] index where the profile is saved

    \return error if less than 0
*/
int generateAPFromProfile(int index);
/*!
    \brief  This function is used for generating an Access Point with default
            configuration defines on wifi_config.h

    \return error if less than 0
*/
int generateAPFromDefaultProfile();
/*!
    \brief  This function is used for generating an Access Point with the
            profile saved at a given index. On error generate AP on
            default profile.
    \param[in] index where the profile is saved

    \return error if less than 0
*/
int generateAPFromProfileOnErrorDefault(int index);
/*!
    \brief This function is used for connecting to an Access Point.

    \param[in]      ssid_name is the name of the Access point

    \param[in]      password is the password of the Access Point

    \param[in]      security is the security of the WiFi network. It can be:
                        - 0 or SL_SEC_TYPE_OPEN
                        - 1 or SL_SEC_TYPE_WEP
                        - 2 or SL_SEC_TYPE_WPA_WPA2

    \param[in]      timeout time in seconds trying connect

    \return         On success, zero is returned. On error, negative is returned
*/
_i32 connectToAP(char* ssid_name, char* password, _u8 security, int timeout);
/*!
    \brief This function is used for changing the operation mode of the device

    \param[in]      new_mode is the name of the Access point. It can be:
                        - ROLE_STA
                        - ROLE_AP
                        - ROLE_P2P

    \return         new_mode value if it was successfully completed. Less than 0 on error
*/
_i32 wlanSetMode(int new_mode);

/*!
    \brief This function is used for changing the operation power policy of the device

    \param[in]      power is a number between 0-15, as dB offset from max power. 0 will set maximum power

    \return          On success, zero is returned. On error, negative is returned
*/
_i32 setWlanPower(_u8 power);
/*!
    \brief This function is used for setting the power policy of the device

    \param[in]      policy is the power policy to set. It can be:
                      - SL_ALWAYS_ON_POLICY
                      - SL_NORMAL_POLICY
                      - SL_LOW_POWER_POLICY
                      - SL_LONG_SLEEP_INTERVAL_POLICY

    \return         On success, zero is returned. On error, negative is returned
*/
_i32 setPowerPolicy(_u8 policy);
/*!
    \brief This function is used to put the device in low power consumption mode

    \param[in]      time is a value between 100-2000 ms

    \return         On success, zero is returned. On error, negative is returned
*/
_i32 sleepWlanDevice(int time);
/*!
    \brief This function configures the SimpleLink device in its default state. It:
	- Sets the mode to STATION
	- Configures connection policy to Auto and AutoSmartConfig
	- Deletes all the stored profiles
	- Enables DHCP
	- Disables Scan policy
	- Sets Tx power to maximum
	- Sets power policy to normal
	- Unregisters mDNS services
	- Remove all filters

    \param[in]      none

    \return         On success, zero is returned. On error, negative is returned
*/
_i32 configureSimpleLinkToDefaultState();
/*!
    \brief Pings to the last connected device

    \param[in]  interval interval between ping commands

    \param[in]  size size of the ping package

    \param[in]  request_timeout timeout of the response

    \param[in]  ping_attemp times to retrying
*/
_i32 pingToConnectedDevice(int interval, int size, int request_timeout, int ping_attemp);
/*!
    \brief Pings to an IP

    \param[in]  interval interval between ping commands

    \param[in]  size size of the ping package

    \param[in]  request_timeout timeout of the response

    \param[in]  ping_attemp times to retrying

    \param[in] ip Address to ping
*/
_i32 ping(int interval, int size, int request_timeout, int ping_attemp, _u32 ip);
/*!
    \brief Thread that waits until the first client connects
*/
void waitClients();

/*!
    \brief Gets IPv4 value into an array

    \param[in]  val IP value code into a _u32 data type

    \param[out] returnIP IP value code into an array of four numbers
*/
void prettyIPv4(_u32 val, _u8* returnIP);
void printPrettyIPv4_u32(_u32 ip);
void printPrettyIPv4_char(_u8* ip);

void printPrettyMAC(_u8 *macAddressVal);

void printWifiParams(WifiConnectionState state);

/*!
    \brief Disconnects from a WLAN Access point

    This function disconnects from the connected AP

    \warning        If the WLAN disconnection fails, we will be stuck in this function forever.
*/
int disconnectFromAP();

void getOwnMAC(_u8 *macAddressVal);
/*!
    \brief Changes the device's MAC Address

    \param[in]  macAddressVal new MAC address

    \warning After that the device can have a malfunction
*/
void setOwnMAC(_u8 *macAddress);

_u32 getOwnIP();
_u32 getHostIP();
_u32 getStationIP();

/*!
    \brief Gets all available WiFi networks

    \param[in] scan_table_size the maximum wifi networks to return

    \param[in] channel channel where scan. Have values between 1-11. Other value means all channels

    \param[in] timeout timeout in seconds for wifi network scan

    \param[out] netEntries array of found WiFi networks

    \note this function ends turning off the device, if you want to keep using it set on again
*/
int scanWifi(int scan_table_size, int channel, int timeout, Sl_WlanNetworkEntry_t *netEntries);
/*!
    \brief Gets all available WiFi networks and restore previous state

    \param[in]  scan_table_size the maximum wifi networks to return

    \param[in] channel channel where scan. Have values between 1-11. Other value means all channels

    \param[in] timeout timeout in seconds for wifi network scan

    \param[out] netEntries array of found WiFi networks
*/
int scanWifiRestoreState(int scan_table_size, int channel, int timeout, Sl_WlanNetworkEntry_t *netEntries);
/*!
    \brief Gets the number of less saturated channel

    \return the number of less saturated channel
*/
int getLessSaturatedChannel();
/*!
    \brief Gets the wifi connection state

    \return the wifi connection state
*/
WifiConnectionState getWifiState();
/*!
    \brief Sets the wifi connection state
*/
void setWifiState(WifiConnectionState state);
/*!
    \brief Saves the current profile

    \return index where the profile has been stored, less than 0 on error
*/
_i16 saveCurrentProfile();
/*!
    \brief Saves the given profile

    \param[in]      ssid_name is the name of the Access point

    \param[in]      password is the password of the Access Point

    \param[in]      security is the security of the WiFi network. It can be:
                        - 0 or SL_SEC_TYPE_OPEN
                        - 1 or SL_SEC_TYPE_WEP
                        - 2 or SL_SEC_TYPE_WPA_WPA2

    \param[in]      channel is the channel where the network is generated

    \return index where the profile has been stored, less than 0 on error

    \note this function only support MODE_AP profiles

    \warning this function only support ONE profile
*/
_i16 saveProfile(char* ssid_name, char* password, _u8 security, int channel);
/*!
    \brief Gets the stored profile at a given index

    \param[in] index where the profile is saved

    \return less than 0 on error
*/
_i16 getProfile(_i16 index, WifiConnectionState *profile);
/*!
    \brief Restores the stored profile at a given index

    \param[in] index where the profile is saved

    \return less than 0 on error
*/
_i16 restoreProfile(int index);
/*!
    \brief Removes all the stored profiles

    \return less than 0 on error
*/
_i16 removeProfiles();
/*!
    \brief Removes all the stored profiles

    \param[in]      filepath destination path

    \param[in]      filecontent content of the file to flash

    \param[in]      file_size size of filecontent

    \return less than 0 on error
*/
_i32 storeFileFlash(char* filepath, char* filecontent, int file_size);

#ifdef __cplusplus
}
#endif

#endif
