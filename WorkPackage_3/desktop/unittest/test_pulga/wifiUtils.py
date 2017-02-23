import dbus
import time

class wifiUtils:
    def __init__(self,interface):
        self.bus = dbus.SystemBus()
        self.manager_bus_object = self.bus.get_object("org.freedesktop.NetworkManager",
                                            "/org/freedesktop/NetworkManager")
        self.manager = dbus.Interface(self.manager_bus_object,
                                 "org.freedesktop.NetworkManager")
        self.device_path = self.manager.GetDeviceByIpIface(interface)
        self.connection_path = ""
        
    def getAPpath(self,SSID):
    # Get path to the 'wlan0' device. If you're uncertain whether your WiFi
    # device is wlan0 or something else, you may utilize manager.GetDevices()
    # method to obtain a list of all devices, and then iterate over these
    # devices to check if DeviceType property equals NM_DEVICE_TYPE_WIFI (2).
    # Connect to the device's Wireless interface and obtain list of access
    # points.
        device = dbus.Interface(self.bus.get_object("org.freedesktop.NetworkManager",
	                               self.device_path),
	                "org.freedesktop.NetworkManager.Device.Wireless")
        accesspoints_paths_list = device.GetAccessPoints()
        our_ap_path = None
        for ap_path in accesspoints_paths_list:
            ap_props = dbus.Interface(
             self.bus.get_object("org.freedesktop.NetworkManager", ap_path),
             "org.freedesktop.DBus.Properties")
            ap_ssid = ap_props.Get("org.freedesktop.NetworkManager.AccessPoint",
	                   "Ssid")
    # Returned SSID is a list of ASCII values. Let's convert it to a proper
    # string.
            str_ap_ssid = "".join(chr(i) for i in ap_ssid)
        #    print ap_path, ": SSID =", str_ap_ssid
            if str_ap_ssid == SSID:
                our_ap_path = ap_path
                break

        if not our_ap_path:
            return "AP not found"
      #  print our_ap_path
        return our_ap_path

    def connectToAP(self, SSID, passphrase):
        apPath = self.getAPpath(SSID)
        connection_params = {
            "802-11-wireless": {
                "security": "802-11-wireless-security",
            },
            "802-11-wireless-security": {
                "key-mgmt": "wpa-psk",
                "psk": passphrase
            },
        }

        # Establish the connection.
        self.settings_path, self.connection_path = self.manager.AddAndActivateConnection(
            connection_params, self.device_path, apPath)
     #   print "settings_path =", settings_path
    #    print "connection_path =", connection_path

        # Wait until connection is established. This may take a few seconds.
        NM_ACTIVE_CONNECTION_STATE_ACTIVATED = 2
        print """Waiting for connection to reach """ \
              """NM_ACTIVE_CONNECTION_STATE_ACTIVATED state ..."""
        connection_props = dbus.Interface(
            self.bus.get_object("org.freedesktop.NetworkManager", self.connection_path),
            "org.freedesktop.DBus.Properties")
        state = 0
        while True:
            # Loop forever until desired state is detected.
            #
            # A timeout should be implemented here, otherwise the program will
            # get stuck if connection fails.
            #
            # IF PASSWORD IS BAD, NETWORK MANAGER WILL DISPLAY A QUERY DIALOG!
            # This is something that should be avoided, but I don't know how, yet.
            #
            # Also, if connection is disconnected at this point, the Get()
            # method will raise an org.freedesktop.DBus.Error.UnknownMethod
            # exception. This should also be anticipated.
            state = connection_props.Get(
                "org.freedesktop.NetworkManager.Connection.Active", "State")
            if state == NM_ACTIVE_CONNECTION_STATE_ACTIVATED:
                break
            time.sleep(0.001)
        print "Connection established!"
        return True  

    def disconnectAP(self):
        NM_ACTIVE_CONNECTION_STATE_ACTIVATED = 2
        connection_props = dbus.Interface(
                self.bus.get_object("org.freedesktop.NetworkManager", self.connection_path),
                "org.freedesktop.DBus.Properties")
        if connection_props.Get("org.freedesktop.NetworkManager.Connection.Active", "State") == NM_ACTIVE_CONNECTION_STATE_ACTIVATED:
           # print "entra " + self.connection_path
            self.manager.DeactivateConnection(self.connection_path)
            #print "cp " + self.connection_path
            settings = dbus.Interface(
                self.bus.get_object("org.freedesktop.NetworkManager", self.settings_path),
                "org.freedesktop.NetworkManager.Settings.Connection")
            settings.Delete() 
