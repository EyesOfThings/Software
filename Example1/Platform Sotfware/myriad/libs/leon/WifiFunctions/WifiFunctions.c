/*!
    \file
 
    \brief     Library for common wifi functions. File
*/
#include <WifiFunctions.h>

_u32  _status = 0;
_u32  _pingPacketsRecv = 0;
_u32  _gatewayIP = 0;
_u32  _stationIP = 0;
_i16  _device_mode = 0;
int   _already_initialized = 0;

WifiConnectionState _connection_state = { .ssid_name = "", .password = "", 
                    .security = 0, .channel = -1, .mode = NOT_CONNECTED}; ;

static void SimpleLinkPingReport(SlPingReport_t *pPingReport);
_i32 establishConnectionWithAP(char* ssid_name, char* password, _u8 security, int timeout);
_i32 initializeAppVariables();

/*!
    \brief This function handles WLAN events

    \param[in]      pWlanEvent is the event passed to the handler
*/
void SimpleLinkWlanEventHandler(SlWlanEvent_t *pWlanEvent) {
    if(pWlanEvent == NULL) {

        CLI_Write((_u8 *)"[WLAN EVENT] NULL Pointer Error \n");
        return;
    }
    
    switch(pWlanEvent->Event) {
        case SL_WLAN_CONNECT_EVENT:
        {
            SET_STATUS_BIT(_status, STATUS_BIT_CONNECTION);
        }
        break;

        case SL_WLAN_DISCONNECT_EVENT:
        {
            slWlanConnectAsyncResponse_t*  pEventData = NULL;

            CLR_STATUS_BIT(_status, STATUS_BIT_CONNECTION);
            CLR_STATUS_BIT(_status, STATUS_BIT_IP_ACQUIRED);

            pEventData = &pWlanEvent->EventData.STAandP2PModeDisconnected;

            if(SL_USER_INITIATED_DISCONNECTION == pEventData->reason_code)
                CLI_Write((_u8 *)"Device disconnected from the AP on application's request \n");

            else
                CLI_Write((_u8 *)"Device disconnected from the AP on an ERROR..!! \n");
            
        }
        break;

        case SL_WLAN_STA_CONNECTED_EVENT:
        {
            SET_STATUS_BIT(_status, STATUS_BIT_STA_CONNECTED);
        }
        break;

        case SL_WLAN_STA_DISCONNECTED_EVENT:
        {
            CLR_STATUS_BIT(_status, STATUS_BIT_STA_CONNECTED);
            CLR_STATUS_BIT(_status, STATUS_BIT_IP_LEASED);
        }
        break;

        default:
        {
            CLI_Write((_u8 *)"[WLAN EVENT] Unexpected event \n");
        }
        break;
    }
}


/*!
    \brief This function handles events for IP address acquisition via DHCP
           indication

    \param[in]      pNetAppEvent is the event passed to the handler
*/
void SimpleLinkNetAppEventHandler(SlNetAppEvent_t *pNetAppEvent) {
    if(pNetAppEvent == NULL) {

        CLI_Write((_u8 *)"[NETAPP EVENT] NULL Pointer Error \n");
        return;
    }
 
    switch(pNetAppEvent->Event) {

        case SL_NETAPP_IPV4_IPACQUIRED_EVENT:
        {
            SlIpV4AcquiredAsync_t *pEventData = NULL;

            SET_STATUS_BIT(_status, STATUS_BIT_IP_ACQUIRED);

            pEventData = &pNetAppEvent->EventData.ipAcquiredV4;
            _gatewayIP = pEventData->gateway;
        }
        break;

        case SL_NETAPP_IP_LEASED_EVENT:
        {
            _stationIP = pNetAppEvent->EventData.ipLeased.ip_address;
            SET_STATUS_BIT(_status, STATUS_BIT_IP_LEASED);
        }
        break;

        default:
        {
            CLI_Write((_u8 *)"[NETAPP EVENT] Unexpected event \n");
        }
        break;
    }
}

/*!
    \brief This function handles callback for the HTTP server events

    \param[in]      pHttpEvent - Contains the relevant event information
    \param[in]      pHttpResponse - Should be filled by the user with the
                    relevant response information
*/
void SimpleLinkHttpServerCallback(SlHttpServerEvent_t *pHttpEvent, SlHttpServerResponse_t *pHttpResponse) {
    CLI_Write((_u8 *)"[HTTP EVENT] Unexpected event \n");
}

/*!
    \brief This function handles general error events indication

    \param[in]      pDevEvent is the event passed to the handler
*/
void SimpleLinkGeneralEventHandler(SlDeviceEvent_t *pDevEvent) {
    CLI_Write((_u8 *)"[GENERAL EVENT] \n");
}

/*!
    \brief This function handles socket events indication

    \param[in]      pSock is the event passed to the handler
*/
void SimpleLinkSockEventHandler(SlSockEvent_t *pSock) {

    if(pSock == NULL) {

        CLI_Write("[SOCK EVENT] NULL Pointer Error \n");
        return;
    }

    switch(pSock->Event) {
        case SL_SOCKET_TX_FAILED_EVENT:
        {
            switch( pSock->socketAsyncEvent.SockTxFailData.status ) {
                case SL_ECLOSE:
                    CLI_Write((_u8 *)"[SOCK EVENT] Close socket operation failed to transmit all queued packets\n");
                break;


                default:
                    CLI_Write((_u8 *)"[SOCK EVENT] Unexpected event \n");
                break;
            }
        }
        break;

        default:
            CLI_Write((_u8 *)"[SOCK EVENT] Unexpected event \n");
        break;
    }
}

void printWifiParams(WifiConnectionState state) {
    CLI_Write((_u8 *)"SSID name: %s\n", state.ssid_name);
    CLI_Write((_u8 *)"channel: %d\n", state.channel);
    CLI_Write((_u8 *)"password: %s\n", state.password);

    switch(state.security) {
        case 0:
            CLI_Write((_u8 *)"security: OPEN\n");
        break;
        case 1:
            CLI_Write((_u8 *)"security: WEP\n");

        break;
        case 5:
        case 2:
            CLI_Write((_u8 *)"security: WPA/WPA2\n");
    }
}

void printCurrentWifiParams() {
    printWifiParams(_connection_state);
}

WifiConnectionState setWifiConnectionState(char *ssid_name, char* password, _u8 security,
                                 int channel, ConnectionMode mode) {
    WifiConnectionState state = {
        .security = security,
        .channel = channel,
        .mode = mode
    };

    memcpy(state.ssid_name, ssid_name, strlen(ssid_name));
    memcpy(state.password, password, strlen(password));
    
    return state;
}

void init_device() {

    _i32 retVal = -1;
    _i32 mode;

    retVal = configureSimpleLinkToDefaultState();

    if(retVal < 0) {
        if (retVal == DEVICE_NOT_IN_STATION_MODE)
            CLI_Write((_u8 *)"Failed to configure the device in its default state \n");
    }

    #ifdef DEBUG
        CLI_Write((_u8 *)"Device is configured in default state \n");
    #endif

    mode = sl_Start(0, 0, 0);
    _device_mode = mode;
    _already_initialized = 1;
}

/*!
    \brief This function is used for connect to an Access Point

    \param[in]      ssid_name is the name of the Access point

    \param[in]      password is the password of the Access Point

    \param[in]      security is the security of the WiFi network. Can be:
                        - 0 or SL_SEC_TYPE_OPEN
                        - 1 or SL_SEC_TYPE_WEP
                        - 2 or SL_SEC_TYPE_WPA_WPA2
    
    \param[in]      timeout time in seconds trying connect

    \return         On success, zero is returned. On error, negative is returned
*/
_i32 connectToAP(char* ssid_name, char* password, _u8 security, int timeout) {
    _connection_state = setWifiConnectionState(ssid_name, password, security, -1, MODE_STATION);
    
    #ifdef DEBUG
        CLI_Write((_u8 *)"Connect to AP\n");
        printCurrentWifiParams();
    #endif

    _i32 retVal = -1;
    if(_already_initialized == 0)
        init_device();

    wlanSetMode(ROLE_STA);

    retVal = establishConnectionWithAP(_connection_state.ssid_name, 
        _connection_state.password, _connection_state.security, timeout);

    if(retVal < 0) {
        CLI_Write((_u8 *)"Failed to establish connection with an AP \n");
        return retVal;
    }

    #ifdef DEBUG
        CLI_Write("Connection established with AP and IP is acquired \n");
    #endif

    return retVal;
}

/*!
    \brief  This function is used for generate an Access Point with default 
            configuration defines on wifi_config.h

    \return error if less than 0
*/
int generateAPFromDefaultProfile() {
    return generateAP(DEFAULT_SSID, DEFAULT_PASSWORD, 
                DEFAULT_SECURITY, DEFAULT_CHANNEL);
}

/*!
    \brief  This function is used for generate an Access Point with
            profile saved on given index. On error generate AP on
            default profile.
    \param[in] index where the profile is saved

    \return error if less than 0
*/
int generateAPFromProfileOnErrorDefault(int index) {
    int retVal;
    retVal = restoreProfile(index);
    if(retVal > 0)
        return retVal;
    #ifdef DEBUG
        CLI_Write((_u8 *)"Generating Ap from default profile\n");
    #endif
    
    return generateAPFromDefaultProfile();
}

/*!
    \brief  This function is used for generate an Access Point with
            profile saved on given index.
    \param[in] index where the profile is saved

    \return error if less than 0
*/
int generateAPFromProfile(int index) {
    return restoreProfile(index);
}

/*!
    \brief This function is used for generate an Access Point

    \param[in]      ssid_name is the name of the Access point

    \param[in]      password is the password of the Access Point

    \param[in]      security is the security of the WiFi network. Can be:
                        - 0 or SL_SEC_TYPE_OPEN
                        - 1 or SL_SEC_TYPE_WEP
                        - 2 or SL_SEC_TYPE_WPA_WPA2

    \param[in]      channel is the channel where the network is generated


    \return         0 - if mode was set correctly
*/
int generateAP(char* ssid_name, char* password, _u8 security, int channel) {

    _connection_state = setWifiConnectionState(ssid_name, password, security, channel, MODE_AP);

    #ifdef DEBUG
    CLI_Write((_u8 *)"create AP on channel %d\n", _connection_state.channel);
    printCurrentWifiParams();
    #endif

    _i32 mode = ROLE_STA;
    _i32 retVal = -1;

    if(_already_initialized == 0)
        init_device();

    retVal = sl_WlanSetMode(ROLE_AP);

    if(retVal < 0) CLI_Write((_u8 *)"Error\n");

    retVal = sl_WlanSet(SL_WLAN_CFG_AP_ID, WLAN_AP_OPT_SSID,
                strlen(_connection_state.ssid_name), 
                    (_u8 *) _connection_state.ssid_name);

    if(retVal < 0) CLI_Write((_u8 *)"Error\n");

    retVal = sl_WlanSet(SL_WLAN_CFG_AP_ID, WLAN_AP_OPT_SECURITY_TYPE, 1,
                (_u8 *) &_connection_state.security);

    if(retVal < 0) CLI_Write((_u8 *)"Error\n");


    retVal = sl_WlanSet(SL_WLAN_CFG_AP_ID, WLAN_AP_OPT_PASSWORD, 
                strlen(_connection_state.password),
                    (_u8 *) _connection_state.password);

    if(retVal < 0) CLI_Write((_u8 *)"Error\n");

        
    retVal=sl_WlanSet(SL_WLAN_CFG_AP_ID, WLAN_AP_OPT_CHANNEL, 1, 
                (unsigned char*) &_connection_state.channel);

    if(retVal < 0) CLI_Write((_u8 *)"Error\n");

    retVal = sl_Stop(SL_STOP_TIMEOUT);

    if(retVal < 0) CLI_Write((_u8 *)"Error\n");

    CLR_STATUS_BIT(_status, STATUS_BIT_IP_ACQUIRED);

    mode = sl_Start(0, 0, 0);
    _device_mode = mode;
    if (mode == ROLE_AP)
        while(!IS_IP_ACQUIRED(_status))  
            rtems_task_wake_after(100); 
    
    else
        CLI_Write((_u8 *)"Device couldn't be configured in AP mode \n");

    #ifdef DEBUG
    CLI_Write((_u8 *)"Device started as Access Point\n");
    #endif

    return 0;
}

/*!
    \brief This function is used for generate an Access Point

    \param[in]      ssid_name is the name of the Access point

    \param[in]      password is the password of the Access Point

    \param[in]      security is the security of the WiFi network. Can be:
                        - 0 or SL_SEC_TYPE_OPEN
                        - 1 or SL_SEC_TYPE_WEP
                        - 2 or SL_SEC_TYPE_WPA_WPA2

    \param[in]      channel is the channel where the network is generated


    \return         index of saved profile, less than 0 on error
*/

int generateAPSaveProfile(char* ssid_name, char* password, _u8 security, int channel) {
    int retVal;
    retVal = generateAP(ssid_name, password, security, channel);
    if(retVal != 0) {
        CLI_Write((_u8 *)"Error on generate AP\n");
        return -1;
    }

    retVal = saveCurrentProfile();
    if (retVal < 0)
        CLI_Write((_u8 *)"Error on save profile\n");

    return retVal;
}

/*!
    \brief This function is used for change the operation mode of the device

    \param[in]      new_mode is the name of the Access point. Can be:
                        - ROLE_STA    
                        - ROLE_AP     
                        - ROLE_P2P 
   
    \return         new_mode value if it was successfully completed. Less than 0 on error
*/
_i32 wlanSetMode(int new_mode) {

    _i32          retVal = -1;
    _i32          mode = -1;

    if(_already_initialized == 1) {
        sl_Stop(0);
    }
    mode = sl_Start(0, 0, 0);
    _device_mode = mode;
    if(mode < 0) return mode;
    if(mode == new_mode) return 0;
    if (mode == ROLE_AP) 
        while(!IS_IP_ACQUIRED(_status)) { rtems_task_wake_after(100);}

    retVal = sl_WlanSetMode(new_mode);
    if(retVal < 0) return retVal;


    retVal = sl_Stop(SL_STOP_TIMEOUT);
    if(retVal < 0) return retVal;

    retVal = sl_Start(0, 0, 0);
    _device_mode = retVal;
    if(retVal < 0) return retVal;

    if (retVal != new_mode) {
        CLI_Write((_u8 *)"Device couldn't be configured in new mode \n");
        return -1;
    }
 
    return retVal;
}

/*!
    \brief This function is used for change operation power of the device

    \param[in]      power is a number between 0-15, as dB offset from max power. 0 will set maximum power
   
    \return          On success, zero is returned. On error, negative is returned
*/
_i32 setWlanPower(_u8 power) {

    if(_device_mode == ROLE_STA)
        return sl_WlanSet(SL_WLAN_CFG_GENERAL_PARAM_ID, WLAN_GENERAL_PARAM_OPT_STA_TX_POWER, 1, (_u8 *) &power);

    else
        return sl_WlanSet(SL_WLAN_CFG_GENERAL_PARAM_ID, WLAN_GENERAL_PARAM_OPT_AP_TX_POWER, 1, (_u8 *) &power);
}

/*!
    \brief This function is used for set power policy of the device

    \param[in]      policy is the power policy to set. Can be:
                      - SL_ALWAYS_ON_POLICY
                      - SL_NORMAL_POLICY
                      - SL_LOW_POWER_POLICY
                      - SL_LONG_SLEEP_INTERVAL_POLICY
     
    \return         On success, zero is returned. On error, negative is returned
*/
_i32 setPowerPolicy(_u8 policy) {
    #ifdef DEBUG
        CLI_Write((_u8 *)"Set power policy to ");
        switch(policy){
            case SL_ALWAYS_ON_POLICY:
                CLI_Write((_u8 *)"SL_ALWAYS_ON_POLICY\n");
            break;

            case SL_NORMAL_POLICY:
                CLI_Write((_u8 *)"SL_NORMAL_POLICY\n");

            break;

            case SL_LOW_POWER_POLICY:
                CLI_Write((_u8 *)"SL_LOW_POWER_POLICY\n");
        }
    #endif

    return sl_WlanPolicySet(SL_POLICY_PM, policy, NULL,0);
}

/*!
    \brief This function is used for sleep the device

    \param[in]      time is a value between 100-2000 ms
     
    \return         On success, zero is returned. On error, negative is returned
*/
_i32 sleepWlanDevice(int time) {
    #ifdef DEBUG
        CLI_Write((_u8 *)"Sleeping Wlan Device %d ms\n", time);
    #endif

    _u8 pBuff[4] = {0, 0, time, 0};
    return sl_WlanPolicySet(SL_POLICY_PM, SL_LONG_SLEEP_INTERVAL_POLICY, pBuff, sizeof(pBuff));
}

/*!
    \brief This function configure the SimpleLink device in its default state. It:
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
_i32 configureSimpleLinkToDefaultState() {

    SlVersionFull ver = {.ChipFwAndPhyVersion = {.ChipId = 0, .FwVersion = 0, .PhyVersion = 0}, 
                .NwpVersion = 0, .Padding = 0, .RomVersion = 0};

    _WlanRxFilterOperationCommandBuff_t  RxFilterIdMask = {.FilterIdMask = 0, .Padding = 0};

    _u8           val = 1;
    _u8           configOpt = 0;
    _u8           configLen = 0;
    _u8           power = 0;

    _i32          retVal = -1;
        
    retVal = initializeAppVariables();
    if(retVal < 0) return retVal;

    stopWDT();
    initClk();

    CLI_Configure();

    retVal = wlanSetMode(ROLE_STA);

    configOpt = SL_DEVICE_GENERAL_VERSION;
    configLen = sizeof(ver);

    retVal = sl_DevGet(SL_DEVICE_GENERAL_CONFIGURATION, &configOpt, &configLen, (_u8 *) (&ver));
    if(retVal < 0) return retVal;

    retVal = sl_WlanPolicySet(SL_POLICY_CONNECTION, SL_CONNECTION_POLICY(0, 1, 0, 0, 0), NULL, 0);
    if(retVal < 0) return retVal;

    retVal = sl_WlanDisconnect();

    if(retVal == 0)
        while(IS_CONNECTED(_status)) 
            rtems_task_wake_after(100);


    retVal = sl_NetCfgSet(SL_IPV4_STA_P2P_CL_DHCP_ENABLE, 1, 1, &val);
    if(retVal < 0) return retVal;

    configOpt = SL_SCAN_POLICY(0);
    retVal = sl_WlanPolicySet(SL_POLICY_SCAN , configOpt, NULL, 0);
    if(retVal < 0) return retVal;

    retVal = setWlanPower(0);
    if(retVal < 0) return retVal;

    //setPowerPolicy(SL_NORMAL_POLICY);
    setPowerPolicy(SL_ALWAYS_ON_POLICY);

    retVal = sl_NetAppMDNSUnRegisterService(0, 0);
    if(retVal < 0) return retVal;

    memset(RxFilterIdMask.FilterIdMask, 0xFF, 8);
    retVal = sl_WlanRxFilterSet(SL_REMOVE_RX_FILTER, (_u8 *)&RxFilterIdMask,
                       sizeof(_WlanRxFilterOperationCommandBuff_t));
    if(retVal < 0) return retVal;

    retVal = sl_Stop(SL_STOP_TIMEOUT);
    if(retVal < 0) return retVal;

    retVal = initializeAppVariables();
    if(retVal < 0) return retVal;

    return retVal;
}

/*!
    \brief Connecting to a WLAN Access point

    This function connects to the required AP (SSID_NAME).
    The function will return once we are connected and have acquired IP address

    \param[in]      ssid_name is the name of the Access point

    \param[in]      password is the password of the Access Point

    \param[in]      security is the security of the WiFi network. Can be:
                        - 0 or SL_SEC_TYPE_OPEN
                        - 1 or SL_SEC_TYPE_WEP
                        - 2 or SL_SEC_TYPE_WPA_WPA2
    
    \param[in]      timeout time in seconds trying connect

    \return     0 on success, negative error-code on error

    \warning    If the WLAN connection fails or we don't acquire an IP address,
                We will be stuck in this function forever.
*/
_i32 establishConnectionWithAP(char* ssid_name, char* password, _u8 security, int timeout) {

    SlSecParams_t secParams;

    _i32 retVal = 0;

    secParams.Key = password;
    secParams.KeyLen = strlen(secParams.Key);
    secParams.Type = security;

    retVal = sl_WlanConnect(ssid_name, strlen(ssid_name), NULL, &secParams, NULL);
    if(retVal < 0) return retVal;

    while(((!IS_CONNECTED(_status)) || (!IS_IP_ACQUIRED(_status))) && timeout > 0) {
        CLI_Write((_u8 *)"Connecting...\n");
        rtems_task_wake_after(1000);
        timeout--;
    }

    return timeout > 0 ? retVal : -1;
}

/*!
    \brief Disconnecting from a WLAN Access point

    This function disconnects from the connected AP

    \warning        If the WLAN disconnection fails, we will be stuck in this function forever.
*/
int disconnectFromAP() {
    #ifdef DEBUG
        CLI_Write((_u8 *)"Disconnecting from AP\n");
    #endif

    _i32 retVal = -1;

    retVal = sl_WlanDisconnect();

    if(retVal == 0)
        while(IS_CONNECTED(_status)) 
            rtems_task_wake_after(100);

    return 0;
}

/*!
    \brief This function initializes the application variables

    \return     0 on success, negative error-code on error
*/
_i32 initializeAppVariables() {

    _status = 0;
    _pingPacketsRecv = 0;
    _stationIP = 0;
    _gatewayIP = 0;
    
    return 0;
}

/*!
    \brief Wait thread until the first client connects
*/
void waitClients() {
    #ifdef DEBUG
        CLI_Write((_u8 *)"Waiting for clients\n");
    #endif

    while((!IS_IP_LEASED(_status)) || (!IS_STA_CONNECTED(_status))) 
        rtems_task_wake_after(100);
}

_u32 getStationIP() {
    return _stationIP;
}

/*!
    \brief Get IPv4 value into an array

    \param[in]  val IP value code into a _u32 data type

    \param[out] returnIP IP value code into an array of four numbers
*/
void prettyIPv4(_u32 val, _u8* returnIP) {

    returnIP[0] = SL_IPV4_BYTE(val, 3);
    returnIP[1] = SL_IPV4_BYTE(val, 2);
    returnIP[2] = SL_IPV4_BYTE(val, 1);
    returnIP[3] = SL_IPV4_BYTE(val, 0);
}

/*!
    \brief Ping to a last connected device

    \param[in]  interval interval between ping commands

    \param[in]  size size of the ping package

    \param[in]  request_timeout timeout of the response

    \param[in]  ping_attemp times to retying
*/
_i32 pingToConnectedDevice(int interval, int size, int request_timeout, int ping_attemp) {
    return ping(interval, size, request_timeout, ping_attemp, _stationIP);
}

/*!
    \brief Ping to an IP

    \param[in]  interval interval between ping commands

    \param[in]  size size of the ping package

    \param[in]  request_timeout timeout of the response

    \param[in]  ping_attemp times to retying

    \param[in] ip Address to ping
*/
_i32 ping(int interval, int size, int request_timeout, int ping_attemp, _u32 ip) {
    #ifdef DEBUG
        CLI_Write((_u8 *)"pinging to ");
        printPrettyIPv4_u32(ip);
    #endif

    SlPingStartCommand_t PingParams = {0};

    PingParams.PingIntervalTime = interval;
    PingParams.PingSize = size;
    PingParams.PingRequestTimeout = request_timeout;
    PingParams.TotalNumberOfAttempts = ping_attemp;
    PingParams.Flags = 0;
    PingParams.Ip = ip;

    
    SlPingReport_t Report = {0};

    _u8 SecType = 0;
    _i32 mode = ROLE_STA;
    _i32 retVal = -1;

    retVal = initializeAppVariables();
    if(retVal < 0) return retVal;

    retVal = sl_NetAppPingStart((SlPingStartCommand_t*) &PingParams, SL_AF_INET,
                           (SlPingReport_t*) &Report, SimpleLinkPingReport);

    while(!IS_PING_DONE(_status)) { 
        CLI_Write((_u8 *)"Error on ping\n");
        rtems_task_wake_after(100); 
    }

    if (_pingPacketsRecv == 0) {
        CLI_Write((_u8 *)"A STATION couldn't connect to the device \n");
        if(LAN_CONNECTION_FAILED < 0) return LAN_CONNECTION_FAILED;
    }

    CLI_Write((_u8 *)"Device and the station are successfully connected \n");
}

static void SimpleLinkPingReport(SlPingReport_t *pPingReport) {

    SET_STATUS_BIT(_status, STATUS_BIT_PING_DONE);

    if(pPingReport == NULL) {

        CLI_Write((_u8 *)"[PING REPORT] NULL Pointer Error\n");
        return;
    }

    _pingPacketsRecv = pPingReport->PacketsReceived;
}

_u32 getOwnIP() {

    _u8 len = sizeof(SlNetCfgIpV4Args_t);
    _u8 dhcpIsOn = 0;
    SlNetCfgIpV4Args_t ipV4 = {0};

    if(_device_mode == ROLE_AP)
        sl_NetCfgGet(SL_IPV4_AP_P2P_GO_GET_INFO, &dhcpIsOn, &len, (_u8 *) &ipV4);
    else
        sl_NetCfgGet(SL_IPV4_STA_P2P_CL_GET_INFO, &dhcpIsOn, &len, (_u8 *) &ipV4);

    return ipV4.ipV4;
}

_u32 getHostIP() {
    return _gatewayIP;
}

void getOwnMAC(_u8 *macAddressVal) {
    #ifdef DEBUG
        CLI_Write((_u8 *)"get own MAC Address\n");
    #endif

    _u8 macAddressLen = SL_MAC_ADDR_LEN;
    sl_NetCfgGet(SL_MAC_ADDRESS_GET, NULL, &macAddressLen, macAddressVal);
}

/*!
    \brief Change MAC Address

    \param[in]  macAddressVal new MAC address

    \warning Before that the device can have a malfunction
*/
void setOwnMAC(_u8 *macAddressVal) {
    #ifdef DEBUG
        CLI_Write((_u8 *)"Set own MAC Address to ", 
            macAddressVal[0], macAddressVal[1], macAddressVal[2], 
            macAddressVal[3], macAddressVal[4], macAddressVal[5]);
    #endif

    sl_NetCfgSet(SL_MAC_ADDRESS_SET, 1, SL_MAC_ADDR_LEN, (_u8 *) macAddressVal);
    sl_Stop(0);
    _device_mode = sl_Start(NULL,NULL,NULL);
}

void printPrettyIPv4_u32(_u32 ip) {
    _u8 pretty_ip[4];

    prettyIPv4(ip, pretty_ip);

    printPrettyIPv4_char(pretty_ip);
}

void printPrettyIPv4_char(_u8* ip) {
    CLI_Write((_u8 *)"IP Address %u.%u.%u.%u\n", ip[0], ip[1], ip[2], ip[3]);
}

void printPrettyMAC(_u8 *macAddressVal) {
    CLI_Write((_u8 *)"MAC Address %02X:%02X:%02X:%02X:%02X:%02X\n", 
        macAddressVal[0], macAddressVal[1], macAddressVal[2], 
        macAddressVal[3], macAddressVal[4], macAddressVal[5]);
}

/*!
    \brief Get all available WiFi networks and restore previous state

    \param[in]  scan_table_size the maximum wifi networks to return

    \param[in] channel channel where scan. Have values between 1-11. Other value means all channels

    \param[in] timeout timeout in second for wifi network scan

    \param[out] netEntries array of found WiFi networks
*/
int scanWifiRestoreState(int scan_table_size, int channel, int timeout, 
                        Sl_WlanNetworkEntry_t *netEntries) {
    int idx = 0;
    WifiConnectionState wifi_state = getWifiState();

    idx = scanWifi(scan_table_size, channel, timeout, netEntries);

    setWifiState(wifi_state);

    return idx;
}

/*!
    \brief Get all available WiFi networks

    \param[in] scan_table_size the maximum wifi networks to return

    \param[in] channel channel where scan. Have values between 1-11. Other value means all channels

    \param[in] timeout timeout in second for wifi network scan

    \param[out] netEntries array of found WiFi networks

    \note this function ends turning off the device, if you want to keep using it set on again
*/
int scanWifi(int scan_table_size, int channel, int timeout, Sl_WlanNetworkEntry_t *netEntries) {
    #ifdef DEBUG
        CLI_Write((_u8 *)"scan wifi\n");
    #endif

    Sl_WlanNetworkEntry_t netentry = {.bssid = '0', .reserved = 0, . rssi = 0,
                .sec_type = 0, .ssid = '0', .ssid_len = 1};
    _u8   policyOpt = 0;
    _u16  idx = 0;
    _u16  runningIdx = 0;
    _u32  numOfEntries = 0;
    _i32  retVal = -1;
    _u32  policyVal = 0;

    if (_already_initialized == 1)
        sl_Stop(0);
    
    init_device();

    retVal = initializeAppVariables();
    if(retVal < 0) return retVal;

    slWlanScanParamCommand_t ScanParamConfig = {0};

    if(channel >= 1 && channel <= 11) {
        #ifdef DEBUG
            CLI_Write((_u8 *)"Scan on channel %d\n", channel);
        #endif

        ScanParamConfig.G_Channels_mask = channel;
        ScanParamConfig.rssiThershold = - 80;
    }
    else {
        #ifdef DEBUG
            CLI_Write((_u8 *)"Scan on all channels\n");
        #endif

        ScanParamConfig.G_Channels_mask = 12;
        ScanParamConfig.rssiThershold = - 80;
    }
    sl_WlanSet(SL_WLAN_CFG_GENERAL_PARAM_ID, WLAN_GENERAL_PARAM_OPT_SCAN_PARAMS, 
                sizeof(slWlanScanParamCommand_t), (_u8 *) &ScanParamConfig);

    policyOpt = SL_CONNECTION_POLICY(0, 0, 0, 0, 0);
    retVal = sl_WlanPolicySet(SL_POLICY_CONNECTION , policyOpt, NULL, 0);
    if (retVal < 0)
    {
        CLI_Write("Failed to set the connection policy \n");
        CLI_Write((_u8 *)"Error\n");
    }

    policyOpt = SL_SCAN_POLICY(1);

    policyVal = 15;
    
    #ifdef DEBUG
        CLI_Write("Enabling and configuring the scan policy \n");
    #endif

    retVal = sl_WlanPolicySet(SL_POLICY_SCAN , policyOpt,
                            (_u8 *)&policyVal, sizeof(policyVal));
    if (retVal < 0)
    {
        CLI_Write("Failed to Enable the scan policy \n");
        CLI_Write((_u8 *)"Error\n");
    }

    rtems_task_wake_after(timeout * 1000);

    runningIdx = 0;
    numOfEntries = scan_table_size;

    retVal = sl_WlanGetNetworkList(runningIdx, numOfEntries,
                                   &netEntries[runningIdx]);

    /* From Texas instrument CC3100 driver
     *
     * Because of a bug user should either read the maximum entries or read
     * entries one by one from the end and check for duplicates. Once a duplicate
     * is found process should be stopped.
     */

    runningIdx = 20;
    numOfEntries = 1;
    memset(netEntries, 0, sizeof(netEntries));

    do {
        runningIdx--;
        retVal = sl_WlanGetNetworkList(runningIdx, numOfEntries,
                                   &netentry);
        if(retVal < (int) numOfEntries) {
            CLI_Write((_u8 *)"No wifi found\n");
            return 0;
        }

        if(idx > 0) {
            if(0 == memcmp(netentry.bssid,
                      netEntries[idx - 1].bssid, SL_BSSID_LENGTH))
                break;
        }

        memcpy(&netEntries[idx], &netentry, sizeof(Sl_WlanNetworkEntry_t));
        idx++;

    } while (runningIdx > 0);

    #ifdef DEBUG
        CLI_Write("Scan Process completed \n");
    #endif

    policyOpt = SL_SCAN_POLICY(0);
    retVal = sl_WlanPolicySet(SL_POLICY_SCAN , policyOpt, NULL, 0);

    if (retVal < 0)
    {
        CLI_Write("Failed to to disable the scan policy \n");
        CLI_Write((_u8 *)"Error\n");
    }

    CLI_Write("Disabled the scan policy \n");
    
    retVal = sl_Stop(SL_STOP_TIMEOUT);
    _already_initialized = 0;

    if(retVal < 0)
       CLI_Write((_u8 *)"error\n");

    _connection_state = setWifiConnectionState("", "", 0, -1, NOT_CONNECTED);

    return idx;
}

/*!
    \brief Get the number of less saturated channel

    \return the number of less saturated channel
*/
int getLessSaturatedChannel() {
    int scan_table_size = 10, less_channel = 10000, less_num_entries = 10000;
    int i, num_entries, j;

    Sl_WlanNetworkEntry_t netEntries[scan_table_size];
    
    for(i = 1; i <= 11; i++) {
        
        num_entries = scanWifi(scan_table_size, i, 3, netEntries);
        
        for(j = 0; j < num_entries; j++)
            CLI_Write((_u8 *)"SSID: %s\n", netEntries[j].ssid);

        if(num_entries < less_num_entries) {
            less_channel = i;
            less_num_entries = num_entries;
        }
    }

    return less_channel;
}

/*!
    \brief Get the wifi connection state

    \return the wifi connection state
*/
WifiConnectionState getWifiState() {
    return _connection_state;
}

/*!
    \brief Set the wifi connection state
*/
void setWifiState(WifiConnectionState state) {

    //printf("State.mode = %d, MODE_AP = %d, MODE_STATION = %d \n",state.mode, MODE_AP, MODE_STATION);
    switch(state.mode) {
        case MODE_AP:
            generateAP(state.ssid_name, state.password, 
                state.security, state.channel);
        break;
        case MODE_STATION:
            connectToAP(state.ssid_name, state.password, 
                state.security, 5);
    }
}

/*!
    \brief Save the given profile
    
    \param[in]      ssid_name is the name of the Access point

    \param[in]      password is the password of the Access Point

    \param[in]      security is the security of the WiFi network. Can be:
                        - 0 or SL_SEC_TYPE_OPEN
                        - 1 or SL_SEC_TYPE_WEP
                        - 2 or SL_SEC_TYPE_WPA_WPA2

    \param[in]      channel is the channel where the network is generated

    \return index where profile has been stored, less than 0 on error

    \note this function only support MODE_AP profiles

    \warning this function only support ONE profile 
*/
_i16 saveProfile (char* ssid_name, char* password, _u8 security, int channel) {

    const SlSecParams_t sec = {
            .Key = password, 
            .KeyLen = strlen(password), 
            .Type = security
        };

    const SlSecParamsExt_t sec_ext = {
            .AnonUser = password, 
            .AnonUserLen = strlen(password),
            .CertIndex = 0, 
            .EapMethod = SL_ENT_EAP_METHOD_PEAP1_MSCHAPv2, 
            .User = password, 
            .UserLen = strlen(password)
        };

    return sl_WlanProfileAdd(ssid_name, strlen(ssid_name), 
            NULL, &sec, &sec_ext, channel, 0);
}

/*!
    \brief Save the profile being used 

    \return index where profile has been stored, less than 0 on error
*/
_i16 saveCurrentProfile() {
    return saveProfile(_connection_state.ssid_name, 
            _connection_state.password, _connection_state.security, 
            _connection_state.channel);
}
 
/*!
    \brief Get stored profile at the given index 

    \param[in] index where the profile is saved

    \return less than 0 on error
*/
_i16 getProfile(_i16 index, WifiConnectionState *profile) {

    _i16 retVal = 0;
    _i16 ssid_name_len;
    _u8 mac[6];

    SlSecParams_t sec = {.Key = "", .KeyLen = 0, .Type = 0};
    SlGetSecParamsExt_t sec_ext = {.AnonUser = {0}, .AnonUserLen = 1, 
                .CertIndex = 0, .EapMethod = -1, .User = {0}, .UserLen = 1};

    if(_already_initialized == 0)
        init_device();

    retVal = sl_WlanProfileGet(index, profile->ssid_name, &ssid_name_len, mac, &sec, 
            &sec_ext, &profile->channel);
    if(retVal < 0) {
        CLI_Write((_u8 *)"Failed to get a profile\n");
        return retVal;
    }

    sec_ext.User[sec_ext.AnonUserLen] = '\0';
    profile->ssid_name[ssid_name_len] = '\0';
    profile->security = sec.Type == 5 ? 2 : sec.Type;
    profile->mode = MODE_AP;
    memcpy(profile->password, sec_ext.User, strlen(sec_ext.User));
    profile->password[sec_ext.UserLen]='\0';

    return retVal;
}

/*!
    \brief Restore stored profile at the given index 

    \param[in] index where the profile is saved

    \return less than 0 on error
*/
_i16 restoreProfile(int index) {
    #ifdef DEBUG
        CLI_Write((_u8 * )"Restoring profile\n");
    #endif

    WifiConnectionState profile;
    _i16 retVal;

    retVal = getProfile(index, &profile);
    if (retVal < 0) {
        CLI_Write((_u8 * )"No profile found on index %d\n", index);
        return retVal;
    }
    //printWifiParams(profile);
    setWifiState(profile);
    return retVal;
}

/*!
    \brief Remove all stored profiles

    \return less than 0 on error
*/
_i16 removeProfiles(){
    return sl_WlanProfileDel(0xFF);
}


_i32 storeFileFlash(char* filepath, char* filecontent, int file_size){

    _i32        fileHandle = -1;
    _i32        retVal = -1;
    _u32        Token = 0;

    // Open a user file for writing 
    retVal = sl_FsOpen((_u8 *)filepath, FS_MODE_OPEN_WRITE, &Token, &fileHandle);
    if(retVal < 0)
    {
        /* File Doesn't exit create a new of 63 KB file */
        retVal = sl_FsOpen((_u8 *)filepath, FS_MODE_OPEN_CREATE(file_size,_FS_FILE_OPEN_FLAG_COMMIT|_FS_FILE_PUBLIC_WRITE), &Token, &fileHandle);
        if(retVal < 0)
        {
            CLI_Write(" Error in creating the file \n\r");
            exit(-1);
        }
    }

    // Write  (SIZE_64K/sizeof(filecontent))
    retVal = sl_FsWrite(fileHandle, 0, (_u8 *)filecontent, file_size);

    if (retVal < 0) {
        CLI_Write(" Error in writing the file \n\r");
        retVal = sl_FsClose(fileHandle, 0, 0, 0);
        retVal = sl_FsDel((_u8 *) filepath, Token);
        exit(-1);
    }

    // Close the user file
    retVal = sl_FsClose(fileHandle, 0, 0, 0);
    if (retVal < 0)
    {
        CLI_Write(" Error in closing the file \n\r");
        retVal = sl_FsDel((_u8 *)filepath, Token);
        exit(-1);
    }

    // Open a user file for reading 
    retVal = sl_FsOpen((_u8 *)filepath, FS_MODE_OPEN_READ, &Token, &fileHandle);
    if (retVal < 0)
    {
        CLI_Write(" Error in opening the file for reading \n\r");
        retVal = sl_FsDel((_u8 *)filepath, Token);
        exit(-1);
    }

    //Read the data and compare with the stored buffer 
    char* fileReadcontent = (char*)malloc(file_size*sizeof(char));
    int sizeFileRead = sl_FsRead(fileHandle, 0, fileReadcontent, file_size);
    if (sizeFileRead < 0) {
        CLI_Write(" Error in reading the file \n\r");
        retVal = sl_FsClose(fileHandle, 0, 0, 0);
        retVal = sl_FsDel((_u8 *) filepath, Token);
        exit(-1);
    }
    retVal = pal_Memcmp(filecontent, fileReadcontent, file_size);
    if (retVal != 0) {
        CLI_Write(" Contents that were written and read back didn't match \n\r");
        exit(-1);
    }

    // Close the user file 
    retVal = sl_FsClose(fileHandle, 0, 0, 0);
    if (retVal < 0)
    {
        CLI_Write(" Error in closing the file \n\r");
        retVal = sl_FsDel((_u8 *)filepath, Token);
        exit(-1);
    }

    // Delete the user file
    //retVal = sl_FsDel((_u8 *)filepath, Token);
    //if(retVal < 0)
    //{
    //    CLI_Write(" Error in deleting the file \n\r");
    //    exit(-1);
    //}

}

