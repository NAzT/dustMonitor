//
// Created by undef on 21/12/19.
//

#ifndef DUSTMONITOR_WIFI_H
#define DUSTMONITOR_WIFI_H

#include <WiFi.h>
#include <WiFiManager.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <Ticker.h>
#include <WiFiClient.h>

class WiFi {
private:
    WiFiClient client;


public:
    void wiFiManager();

};


#endif //DUSTMONITOR_WIFI_H
