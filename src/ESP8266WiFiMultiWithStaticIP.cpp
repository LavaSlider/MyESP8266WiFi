/**
 *
 * @file ESP8266WiFiMultiWithStaticIP.cpp
 * @date 03/18/2019
 * @author LavaSlider
 *
 * Derived from @file ESP8266WiFiMulti.cpp
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

#include "ESP8266WiFiMultiWithStaticIP.h"
#include <limits.h>
#include <string.h>

ESP8266WiFiMultiWithStaticIP::ESP8266WiFiMultiWithStaticIP() {
}

ESP8266WiFiMultiWithStaticIP::~ESP8266WiFiMultiWithStaticIP() {
    APlistClean();
}

bool ESP8266WiFiMultiWithStaticIP::addAP(const char* ssid, const char *passphrase, const char *ip, const char *gateway, const char *subnet) {
    if( ip )
	DEBUG_WIFI_MULTI("[WIFI] Just added \"%s\" with static IP \"%s\" to the multi-list\n", ssid, ip );
    else
	DEBUG_WIFI_MULTI("[WIFI] Just added \"%s\" to the multi-list\n", ssid );
    return APlistAdd(ssid, passphrase, ip, gateway, subnet);
}

bool ESP8266WiFiMultiWithStaticIP::existsAP(const char* ssid, const char *passphrase) {
    return APlistExists(ssid, passphrase);
}

wl_status_t ESP8266WiFiMultiWithStaticIP::run(void) {
    DEBUG_WIFI_MULTI("[WIFI] Entering ESP8266WiFiMultiWithStaticIP::run()\n");
    bool showHidden = true;

    wl_status_t status = WiFi.status();
    if(status == WL_DISCONNECTED || status == WL_NO_SSID_AVAIL || status == WL_IDLE_STATUS || status == WL_CONNECT_FAILED) {

        int8_t scanResult = WiFi.scanComplete();

        if(scanResult == WIFI_SCAN_RUNNING) {
            // scan is running, do nothing yet
            status = WL_NO_SSID_AVAIL;
            return status;
        } 

        if(scanResult == 0) {
            // scan done, no ssids found. Start another scan.
            DEBUG_WIFI_MULTI("[WIFI] WiFiMultiWithStaticIP::run scan done\n");
            DEBUG_WIFI_MULTI("[WIFI] no networks found\n");
            WiFi.scanDelete();
            DEBUG_WIFI_MULTI("\n\n");
            delay(0);
            WiFi.disconnect();
            DEBUG_WIFI_MULTI("[WIFI] WiFiMultiWithStaticIP::run starting scan\n");
            // scan wifi async mode
            WiFi.scanNetworks(true, showHidden);
            return status;
        } 

        if(scanResult > 0) {
            // scan done, analyze
            WifiAPEntryWithIP *bestNetworkPtr = NULL;
            int bestNetworkDb = INT_MIN;
	    uint8_t *bestBSSIDptr;
            uint8_t bestBSSID[6];
            int32_t bestChannel;
	    bool hiddenNetworksFound = false;

            DEBUG_WIFI_MULTI("[WIFI] WiFiMultiWithStaticIP::run scan done\n");
            delay(0);

            DEBUG_WIFI_MULTI("[WIFI] %d networks found\n", scanResult);
            for(int8_t i = 0; i < scanResult; ++i) {

                String ssid_scan;
                int32_t rssi_scan;
                uint8_t sec_scan;
                uint8_t* BSSID_scan;
                int32_t chan_scan;
                bool hidden_scan;

                WiFi.getNetworkInfo(i, ssid_scan, sec_scan, rssi_scan, BSSID_scan, chan_scan, hidden_scan);

		// Flag if we encounter any hidden networks
		if( ssid_scan.length() <= 0 || hidden_scan ) {
		    hiddenNetworksFound = true;
		}

                bool known = false;
		// Only look for a match if there is a visible ssid
//		if( ssid_scan.length() > 0 && !hidden_scan ) {
		    // Add '&' before entry to make it a reference and stop
		    // copying every APlist element into entry.
		    for(auto &entry : APlist) {
			// If I store the BSSID in the Access Point list I can
			// check if scan entries with an empty SSID have a
			// matching BSSID and if so check their rssi_scan level.
			if(ssid_scan == entry.ssid || // SSID match
			   (entry.bssid && memcmp((void *)BSSID_scan,(void *)entry.bssid,sizeof(bestBSSID)) == 0) ) {
			    known = true;
			    if(rssi_scan > bestNetworkDb) { // best network
				if(!entry.triedThisOne) { // that has not been tried
				    if(sec_scan == ENC_TYPE_NONE || entry.passphrase) { // check for passphrase if not open wlan
					bestNetworkDb = rssi_scan;
					bestChannel = chan_scan;
					bestNetworkPtr = &entry;
					memcpy((void*) &bestBSSID, (void*) BSSID_scan, sizeof(bestBSSID));
					bestBSSIDptr = &bestBSSID[0];
				    }
				} else {
				    DEBUG_WIFI_MULTI("[WIFI] Already tried %s, although best signal strength, skipping\n",entry.ssid);
				}
			    }
			    break;
			}
		    }
//		} else {
//		    hiddenNetworksFound = true;
//		    DEBUG_WIFI_MULTI("[WIFI] SSID length is Zero on scan identified network\n");
//		    DEBUG_WIFI_MULTI("[WIFI] Its hidden_scan status is %d\n",hidden_scan);
//		    DEBUG_WIFI_MULTI("[WIFI] skipped.\n");
//		}

                if(known) {
		    if(hidden_scan) {
			DEBUG_WIFI_MULTI(" :::> ");
		    } else {
			DEBUG_WIFI_MULTI(" ---> ");
		    }
                } else {
                    DEBUG_WIFI_MULTI("      ");
                }

                DEBUG_WIFI_MULTI(" %d: [%d][%02X:%02X:%02X:%02X:%02X:%02X] %s (%d) %c\n", i, chan_scan, BSSID_scan[0], BSSID_scan[1], BSSID_scan[2], BSSID_scan[3], BSSID_scan[4], BSSID_scan[5], ssid_scan.c_str(), rssi_scan, (sec_scan == ENC_TYPE_NONE) ? ' ' : '*');
                delay(0);
            }

            // clean up ram
            WiFi.scanDelete();

            DEBUG_WIFI_MULTI("\n\n");
            delay(0);

	    // If bestNetworkPtr is null or the ssid is not set then we did
	    // not find one. Now we should try any hidden networks by
	    // blindly sending SSID and passwords to see if one works. If
	    // so I should stick its BSSID into the APlist entry
            if(hiddenNetworksFound && (!bestNetworkPtr || !bestNetworkPtr->ssid)) {
                for(auto &entry : APlist) {
		    // Try any SSID that has not already been tried since
		    // if their SSID is hidden the cannot be found by scan
                    if(!entry.triedThisOne) { // that has not been tried
			//bestNetworkDb = rssi_scan;
			bestNetworkDb = 0;
			//bestChannel = chan_scan;
			bestChannel = 0;
			bestNetworkPtr = &entry;
			// Get the BSSID from the entry
			bestBSSIDptr = (uint8_t *) 0;
			memset((void*) &bestBSSID, '\0', sizeof(bestBSSID));
			DEBUG_WIFI_MULTI("[WIFI] %s did not match any scanned SSID so maybe it is hidden, let's try it.\n",entry.ssid);
                        break;
		    } else {
			DEBUG_WIFI_MULTI("[WIFI] Already tried and failed %s, so it is not a hidden SSID\n",entry.ssid);
		    }
                }
            }

            if(bestNetworkPtr && bestNetworkPtr->ssid) {
                DEBUG_WIFI_MULTI("[WIFI] Connecting BSSID: %02X:%02X:%02X:%02X:%02X:%02X SSID: %s Channel: %d (%d)\n", bestBSSID[0], bestBSSID[1], bestBSSID[2], bestBSSID[3], bestBSSID[4], bestBSSID[5], bestNetworkPtr->ssid, bestChannel, bestNetworkDb);

		// If requested then configure the WiFi
		// Note that two additional unint32_t arguments can be passed
		// with DNS ip addresses.
		if(bestNetworkPtr->ip.isSet()) {
                    DEBUG_WIFI_MULTI("[WIFI] Using static IP address %d.%d.%d.%d\n", bestNetworkPtr->ip[0], bestNetworkPtr->ip[1], bestNetworkPtr->ip[2], bestNetworkPtr->ip[3]);
		    IPAddress gw = bestNetworkPtr->gateway;
		    if(!gw.isSet()) {
			// This is what Arduino library does and it is not really
			// valid but its ok since the user can set what thay really want
			// It should take into account the subnet mask
		    	gw = bestNetworkPtr->ip;
			gw[3] = 1;
#ifdef DEBUG_ESP_WIFI
			DEBUG_WIFI_MULTI("[WIFI] Using default gateway address %d.%d.%d.%d\n", gw[0], gw[1], gw[2], gw[3]);
		    } else {
			DEBUG_WIFI_MULTI("[WIFI] Gateway address %d.%d.%d.%d\n", gw[0], gw[1], gw[2], gw[3]);
#endif
		    }
		    IPAddress sn = bestNetworkPtr->subnet.isSet() ? bestNetworkPtr->subnet : IPAddress(255,255,255,0);
#ifdef DEBUG_ESP_WIFI
		    if( bestNetworkPtr->subnet.isSet() ) {
			DEBUG_WIFI_MULTI("[WIFI] Using subnet mask %d.%d.%d.%d\n", sn[0], sn[1], sn[2], sn[3]);
		    } else {
			DEBUG_WIFI_MULTI("[WIFI] Using default subnet mask %d.%d.%d.%d\n", sn[0], sn[1], sn[2], sn[3]);
		    }
#endif
		    WiFi.config(bestNetworkPtr->ip, gw, sn );
		}
                WiFi.begin(bestNetworkPtr->ssid, bestNetworkPtr->passphrase, bestChannel, bestBSSIDptr);
                status = WiFi.status();

                static const uint32_t connectTimeout = 5000; //5s timeout
                
                auto startTime = millis();
                // wait for connection, fail, or timeout
                while(status != WL_CONNECTED && status != WL_NO_SSID_AVAIL && status != WL_CONNECT_FAILED && (millis() - startTime) <= connectTimeout) {
                    delay(10);
                    status = WiFi.status();
                }
		// When it is not connected because of bad password status is WL_DISCONNECTED
		if( status != WL_CONNECTED ) {
		    DEBUG_WIFI_MULTI("[WIFI] Not connected, marking \"%s\" as not available.\n",bestNetworkPtr->ssid);
		    bestNetworkPtr->triedThisOne = true;
		} else if( !bestBSSIDptr ) {
		    uint8_t *wifiBSSID = WiFi.BSSID();
		    DEBUG_WIFI_MULTI("[WIFI] I think \"%s\" is a hidden network\n",bestNetworkPtr->ssid);
		    DEBUG_WIFI_MULTI("[WIFI] bestBSSIDptr is NULL\n");
		    DEBUG_WIFI_MULTI("[WIFI] bestBSSID is %02X:%02X:%02X:%02X:%02X:%02X\n", bestBSSID[0], bestBSSID[1], bestBSSID[2], bestBSSID[3], bestBSSID[4], bestBSSID[5] );
		    DEBUG_WIFI_MULTI("[WIFI] WiFi.BSSID() is %02X:%02X:%02X:%02X:%02X:%02X\n", wifiBSSID[0], wifiBSSID[1], wifiBSSID[2], wifiBSSID[3], wifiBSSID[4], wifiBSSID[5] );
		    bestNetworkPtr->bssid = (uint8_t *) malloc(6 * sizeof(uint8_t));
		    if( bestNetworkPtr->bssid ) {
			memcpy((void*) bestNetworkPtr->bssid, (void*) WiFi.BSSID(), sizeof(bestBSSID));
		    }
		}
                
#ifdef DEBUG_ESP_WIFI
                IPAddress ip;
                uint8_t * mac;
                switch(status) {
                    case WL_CONNECTED:
                        ip = WiFi.localIP();
                        mac = WiFi.BSSID();
                        DEBUG_WIFI_MULTI("[WIFI] Connecting done.\n");
                        DEBUG_WIFI_MULTI("[WIFI] SSID: %s\n", WiFi.SSID().c_str());
                        DEBUG_WIFI_MULTI("[WIFI] IP: %d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3]);
                        DEBUG_WIFI_MULTI("[WIFI] MAC: %02X:%02X:%02X:%02X:%02X:%02X\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
                        DEBUG_WIFI_MULTI("[WIFI] Channel: %d\n", WiFi.channel());
                        break;
                    case WL_NO_SSID_AVAIL:
                        DEBUG_WIFI_MULTI("[WIFI] Connecting Failed AP not found.\n");
                        break;
                    case WL_CONNECT_FAILED:
                        DEBUG_WIFI_MULTI("[WIFI] Connecting Failed.\n");
                        break;
		    case WL_NO_SHIELD:
                        DEBUG_WIFI_MULTI("[WIFI] Connecting Failed WL_NO_SHIELD?\n");
                        break;
		    case WL_IDLE_STATUS:
                        DEBUG_WIFI_MULTI("[WIFI] Connecting Failed WL_IDLE_STATUS (i.e., still trying).\n");
                        break;
		    case WL_SCAN_COMPLETED:
                        DEBUG_WIFI_MULTI("[WIFI] Connecting Failed WL_SCAN_COMPLETED? This shouldn't be here.\n");
                        break;
		    case WL_CONNECTION_LOST:
                        DEBUG_WIFI_MULTI("[WIFI] Connecting Failed WL_CONNECTION_LOST\n");
                        break;
		    case WL_DISCONNECTED:
                        DEBUG_WIFI_MULTI("[WIFI] Connecting Failed WL_DISCONNECTED.\n");
                        break;
                    default:
                        DEBUG_WIFI_MULTI("[WIFI] Connecting Failed - unrecognized status: %d.\n", status);
                        break;
                }
#endif
            } else {
                DEBUG_WIFI_MULTI("[WIFI] no matching wifi found!\n");
		// I need to clear all the triedThisOne flags here so
		// re-attempts are made
                for(auto &entry : APlist) {
                    DEBUG_WIFI_MULTI("[WIFI] Clearing 'triedThisOne' flag on %s\n", entry.ssid);
		    entry.triedThisOne = false;
                }
            }

            return status;
        }
 
        // scan failed, or some other condition not handled above. Start another scan.
        DEBUG_WIFI_MULTI("[WIFI] WiFiMultiWithStaticIP::run delete old wifi config...\n");
        WiFi.disconnect();

        DEBUG_WIFI_MULTI("[WIFI] WiFiMultiWithStaticIP::run starting scan\n");
        // scan wifi async mode
        WiFi.scanNetworks(true, showHidden);
    }
    return status;
}

// ##################################################################################

bool ESP8266WiFiMultiWithStaticIP::APlistAdd(const char* ssid, const char *passphrase, const char *ipString, const char *gatewayString, const char *subnetString) {

    WifiAPEntryWithIP newAP;

    if(!ssid || *ssid == 0x00 || strlen(ssid) > 32) {
        // fail SSID too long or missing!
        DEBUG_WIFI_MULTI("[WIFI][APlistAdd] no ssid or ssid too long\n");
        return false;
    }

    //for passphrase, max is 63 ascii + null. For psk, 64hex + null.
    if(passphrase && strlen(passphrase) > 64) {
        // fail passphrase too long!
        DEBUG_WIFI_MULTI("[WIFI][APlistAdd] passphrase too long\n");
        return false;
    }

    if(APlistExists(ssid, passphrase)) {
        DEBUG_WIFI_MULTI("[WIFI][APlistAdd] SSID: %s already exists\n", ssid);
        return true;
    }

    if(ipString && !newAP.ip.fromString(ipString)) {
        // fail, not a valid IP address string
        DEBUG_WIFI_MULTI("[WIFI][APlistAdd] invalid IP address \"%s\"\n",ipString);
        return false;
    }

    if(gatewayString && !newAP.gateway.fromString(gatewayString)) {
        // fail, not a valid IP address string
        DEBUG_WIFI_MULTI("[WIFI][APlistAdd] invalid gateway address \"%s\"\n",gatewayString);
        return false;
    }

    if(subnetString && !newAP.subnet.fromString(subnetString)) {
        // fail, not a valid IP address string
        DEBUG_WIFI_MULTI("[WIFI][APlistAdd] invalid subnet mask \"%s\"\n",subnetString);
        return false;
    }

    newAP.ssid = strdup(ssid);

    if(!newAP.ssid) {
        DEBUG_WIFI_MULTI("[WIFI][APlistAdd] fail newAP.ssid == 0\n");
        return false;
    }

    if(passphrase) {
        newAP.passphrase = strdup(passphrase);
    } else {
        newAP.passphrase = strdup("");
    }

    if(!newAP.passphrase) {
        DEBUG_WIFI_MULTI("[WIFI][APlistAdd] fail newAP.passphrase == 0\n");
        free(newAP.ssid);
        return false;
    }

    // Lets just make sure things are initialezed
    newAP.bssid = (uint8_t *) 0;
    newAP.triedThisOne = false;

    APlist.push_back(newAP);
    DEBUG_WIFI_MULTI("[WIFI][APlistAdd] add SSID: %s\n", newAP.ssid);
    return true;
}

bool ESP8266WiFiMultiWithStaticIP::APlistExists(const char* ssid, const char *passphrase) {
    if(!ssid || *ssid == 0x00 || strlen(ssid) > 32) {
        // fail SSID too long or missing!
        DEBUG_WIFI_MULTI("[WIFI][APlistExists] no ssid or ssid too long\n");
        return false;
    }
    for(auto entry : APlist) {
        if(!strcmp(entry.ssid, ssid)) {
            if(!passphrase) {
                if(!strcmp(entry.passphrase, "")) {
                    return true;
                }
            } else {
                if(!strcmp(entry.passphrase, passphrase)) {
                    return true;
                }
            }
        }
    }
    return false;
}

void ESP8266WiFiMultiWithStaticIP::APlistClean(void) {
    for(auto entry : APlist) {
        if(entry.ssid) {
            free(entry.ssid);
        }
        if(entry.passphrase) {
            free(entry.passphrase);
        }
	if(entry.bssid) {
            free(entry.bssid);
	}
    }
    APlist.clear();
}

