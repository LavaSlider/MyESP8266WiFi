/*
    This sketch tries to Connect to the best AP based on a given list
    where 'best' is defined as the strongest signal strength

*/
//#define DEBUG_ESP_WIFI

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMultiWithStaticIP.h>

ESP8266WiFiMultiWithStaticIP wifiMulti;

void setup() {
    Serial.begin(115200);
    Serial.println("");

    WiFi.mode(WIFI_STA);
    wifiMulti.addAP( "openNetwork" );
    wifiMulti.addAP( "network1", "password1" );
    wifiMulti.addAP( "network2", "password2", "ip.add.res.s2" );
    wifiMulti.addAP( "network3", "password3", "ip.add.res.s3", "rou.ter.add.ress", "255.255.255.0" );

    Serial.println( "Connecting Wifi..." );
    if( wifiMulti.run() == WL_CONNECTED ) {
        Serial.println( "" );
        Serial.println( "WiFi connected" );
        Serial.println( "IP address: " );
        Serial.println( WiFi.localIP() );
    }
}

void loop() {
    if( wifiMulti.run() != WL_CONNECTED ) {
        Serial.println( "WiFi not connected!" );
        delay( 1000 );
    } else {
        IPAddress ip = WiFi.localIP();
        Serial.printf("Connected to ssid \"%s\" on IP address %d.%d.%d.%d\n", WiFi.SSID().c_str(), ip[0], ip[1], ip[2], ip[3] );
        delay( 10000 );
    }
}
