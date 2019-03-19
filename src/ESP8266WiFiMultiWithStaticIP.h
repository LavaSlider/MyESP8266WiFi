/**
 *
 * @file ESP8266WiFiMultiWithStaticIP.h
 * @date 03/18/2019
 * @author LavaSlider
 *
 * Derived from @file ESP8266WiFiMulti.h
 * @date 16.05.2015
 * @author Markus Sattler
 *
 * Copyright (c) 2015 Markus Sattler. All rights reserved.
 * This file is part of the esp8266 core for Arduino environment.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef WIFICLIENTMULTI_H_
#define WIFICLIENTMULTI_H_

#include "ESP8266WiFi.h"
#include <vector>

#ifdef DEBUG_ESP_WIFI
#ifdef DEBUG_ESP_PORT
#define DEBUG_WIFI_MULTI(fmt, ...) DEBUG_ESP_PORT.printf( (PGM_P)PSTR(fmt), ##__VA_ARGS__ )
#endif
#endif

#ifndef DEBUG_WIFI_MULTI
#define DEBUG_WIFI_MULTI(...)
#endif

struct WifiAPEntryWithIP {
    char * ssid;	// The SSID of the network
    char * passphrase;	// The passphrase to use for the network
    IPAddress ip;	// The requested ip address
    IPAddress gateway;	// The gateway's ip
    IPAddress subnet;	// The subnet mask
    uint8 * bssid;	// The discovered BSSID for hidden SSIDs
    bool triedThisOne;
};

typedef std::vector<WifiAPEntryWithIP> WifiAPlist;

class ESP8266WiFiMultiWithStaticIP {
    public:
        ESP8266WiFiMultiWithStaticIP();
        ~ESP8266WiFiMultiWithStaticIP();

        bool addAP(const char* ssid, const char *passphrase = NULL, const char *ip = NULL, const char *gateway = NULL, const char *subnet = NULL);
        bool existsAP(const char* ssid, const char *passphrase = NULL);

        wl_status_t run(void);

    private:
        WifiAPlist APlist;
        bool APlistAdd(const char* ssid, const char *passphrase = NULL, const char *ip = NULL, const char *gateway = NULL, const char *subnet = NULL);
        bool APlistExists(const char* ssid, const char *passphrase = NULL);
        void APlistClean(void);

};

#endif /* WIFICLIENTMULTI_H_ */
