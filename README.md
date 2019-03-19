# MyESP8266WiFi
Overrides and extensions of the ESP8266WiFi library components.

## ESP8266WiFiMultiWithStaticIP
This builds upon the ESP8266WiFiMulti library component to achieve three goals.
First, it allows static IP address information to be passed to addAP(). This
information is optional to be totaly backward compatible. Three additional
const char * arguments were added for the desired IP address, the router or
gateway IP address and the subnet mask. Each of these are specified as a
typical IP address string such as "192.168.1.56".

        bool addAP(const char* ssid, const char *passphrase = NULL, const char *ip = NULL, const char *gateway = NULL, const char *subnet = NULL);

Second, it will now fail and attempt the next network specified if the
connection fails. So, for example, if the WiFi password is changed so what
is stored in the code is not correct it will try another SSID.

Third, it will now connect to hidden SSID networks. It acheives this by
first attempting all non-hidden SSIDs that have been added. If they are
not found or connections to them fail then any SSID that was added as
an access point but was not found in the list returned by scan will be
attempted as a potentially hidden SSID. Since the identity of the
network is not known until after it is connected to these cannot be
prioritized for signal strenght like non-hidden SSIDs. Once a hidden
SSID is connected to, however, its BSSID is remembered and it can be
prioritized with the other networks by signal strength.
