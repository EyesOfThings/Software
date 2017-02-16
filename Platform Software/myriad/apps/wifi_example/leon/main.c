#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <semaphore.h>
#include <pthread.h>
#include <sched.h>
#include <fcntl.h>
#include <mv_types.h>
#include <rtems/cpuuse.h>
#include <bsp.h>

#include "rtems_config.h"

#include <WifiFunctions.h>


void POSIX_Init (void *args) {

    initClocksAndMemory();
    printf("\n");
/*
    generateAP("prueba_dani", "123456789", 2, 7);
    printf("Own IP\n");
    printPrettyIPv4_u32(getOwnIP());
    waitClients();
    connectToAP("visilab", "123456789", SL_SEC_TYPE_WPA_WPA2, 20);

    printf("Own IP\n");
    printPrettyIPv4_u32(getOwnIP());
/**///-----------------------------------------------------------------------------

    _i16 retVal = 0;
    WifiConnectionState profile = {
        .ssid_name = "testProfile",
        .password = "123456789",
        .security = SL_SEC_TYPE_WPA_WPA2,
        .channel = 3
    };

    generateAP("test", "123456789", SL_SEC_TYPE_WPA_WPA2, 5);

    retVal = saveProfile(profile.ssid_name, profile.password, 
            profile.security, profile.channel);
    WifiConnectionState profile_readed = {
        .ssid_name = "",
        .password = "",
        .security = 0,
        .channel = 0
    };

    retVal = getProfile(retVal, &profile_readed);

    printf("channel: %d, channel_readed %d\n", profile.channel, profile_readed.channel);
    
    printf("security: %d, security_readed %d\n", profile.security, profile_readed.security);

/**///-----------------------------------------------------------------------------
/*
    restoreProfile(0);
    waitClients();
    printf("have a client\n");
/**///-----------------------------------------------------------------------------
/*
    generateAP("prueba_dani", "123456789", 2, 7);
    _i16 index = saveCurrentProfile();
    printf("Saved profile on %d index \n", index);
    WifiConnectionState profile;
    getProfile(index, &profile);
    printWifiParams(profile);

/**///-----------------------------------------------------------------------------
/*
    int scan_table_size = 10;
    Sl_WlanNetworkEntry_t netEntries[scan_table_size];

    int num_entries = scanWifi(scan_table_size, 0, 3, netEntries);
    int i;
    for(i = 0; i < num_entries; i++)
        printf("SSID: %s\n", netEntries[i].ssid);
    

    rtems_task_wake_after(2000);
/**///-----------------------------------------------------------------------------

/*
    int less_channel = getLessSaturatedChannel();
    printf("Less saturated channel %d\n", less_channel);
    rtems_task_wake_after(5000);
/**///-----------------------------------------------------------------------------


/*
    //0 SL_SEC_TYPE_OPEN, 1 SL_SEC_TYPE_WEP, 2 SL_SEC_TYPE_WPA_WPA2
    generateAP("prueba_dani", "123456789", 2, 7);
    printPrettyIPv4_u32(getOwnIP());

    //setPowerPolicy(SL_LOW_POWER_POLICY);

    _u8 macAddressVal[SL_MAC_ADDR_LEN];
    getOwnMAC(macAddressVal);
    
    printPrettyMAC(macAddressVal);
    
    waitClients();

    pingToConnectedDevice(1000, 20, 3000, 3);
    
    rtems_task_wake_after(4000);


    generateAP("prueba_dani2", "123456789", 2, 7);
    printPrettyIPv4_u32(getOwnIP());

    //setPowerPolicy(SL_LOW_POWER_POLICY);

    getOwnMAC(macAddressVal);
    
    printPrettyMAC(macAddressVal);
    
    waitClients();

    pingToConnectedDevice(1000, 20, 3000, 3);
    
    rtems_task_wake_after(15000);
/**///-----------------------------------------------------------------------------
/*
    generateAP("prueba_dani", "123456789", 2, 7);

    WifiConnectionState wifi_state = getWifiState();

    waitClients();
    printf("Have a client\n");
/*
    int scan_table_size = 10;
    Sl_WlanNetworkEntry_t netEntries[scan_table_size];

    int num_entries = scanWifi(scan_table_size, 0, 5, netEntries);
    int i;
    for(i = 0; i < num_entries; i++)
        printf("SSID: %s\n", netEntries[i].ssid);

    setWifiState(wifi_state);
    waitClients();

    printf("end test\n");
    rtems_task_wake_after(3000);
/**///-----------------------------------------------------------------------------

/*
    generateAP("prueba_dani", "123456789", 2, 7);

    WifiConnectionState wifi_state = getWifiState();
    waitClients();

    int scan_table_size = 10;
    Sl_WlanNetworkEntry_t netEntries[scan_table_size];

    int num_entries = scanWifiRestoreState(scan_table_size, 0, 3, netEntries);
    int i;
    for(i = 0; i < num_entries; i++)
        printf("SSID: %s\n", netEntries[i].ssid);

    waitClients();

    printf("end test\n");
    rtems_task_wake_after(3000);
/**///-----------------------------------------------------------------------------

/*
    printf("Get own IP Address");
    printPrettyIPv4_u32(getOwnIP());

    //_u8 macAddressVal[SL_MAC_ADDR_LEN];
    getOwnMAC(macAddressVal);
    printPrettyMAC(macAddressVal);
*/
/*

    // MAC 11:22:33:44:55:66
    macAddressVal[0] = 0xB;
    macAddressVal[1] = 0x16;
    macAddressVal[2] = 0x21;
    macAddressVal[3] = 0x2C;
    macAddressVal[4] = 0x37;
    macAddressVal[5] = 0x42;    

    setOwnMAC(macAddressVal);
    
    getOwnMAC(macAddressVal);
    printPrettyMAC(macAddressVal);

    rtems_task_wake_after(5000);
*///-----------------------------------------------------------------------------

/*
    connectToAP("visilab", "123456789", SL_SEC_TYPE_WPA_WPA2, 20);

    printf("Own IP\n");
    printPrettyIPv4_u32(getOwnIP());

    printf("Host IP\n");
    printPrettyIPv4_u32(getHostIP());

    rtems_task_wake_after(5000);
/**///-----------------------------------------------------------------------------

/*    
    disconnectFromAP();
    rtems_task_wake_after(5000)
/**///-----------------------------------------------------------------------------

 /*
    connectToAP("visilab", "123456789", SL_SEC_TYPE_WPA_WPA2, 20);

    printf("Own IP\n");
    printPrettyIPv4_u32(getOwnIP());

    printf("Host IP\n");
    printPrettyIPv4_u32(getHostIP());

    rtems_task_wake_after(5000);
/**///-----------------------------------------------------------------------------

    while(1);
    exit(0);
    return;
}


static void Fatal_extension(Internal_errors_Source  the_source,
                            bool                    is_internal,
                            uint32_t                the_error) {

    if (the_source != RTEMS_FATAL_SOURCE_EXIT)
        printk ("\nSource %d Internal %d Error %d\n", the_source, is_internal, the_error);
}
