//
// Created by undef on 21/12/19.
//

#include "WiFi.h"
#include <Preferences.h>
#include <WiFi.h>
#include <Esp32WifiManager.h>

void WiFi::wiFiManager(){
    WiFiManagerParameter custom_thingspeak_channel("channel", "ThingSpeak Channel", theConfig.thingSpeakChannel.c_str(), 9);
    WiFiManagerParameter custom_thingspeak_key("key", "ThingSpeak Key", theConfig.thingSpeakKey.c_str(), 17);
    WiFiManagerParameter custom_publish_interval("publishInterval", "Publish Interval", String(theConfig.publishInterval).c_str(), 6);

    WiFiManager wifiManager;

    wifiManager.setSaveConfigCallback([]() {
        Serial.println("Should save config");
        shouldSaveConfig = true;
    });

    wifiManager.addParameter(&custom_thingspeak_channel);
    wifiManager.addParameter(&custom_thingspeak_key);
    wifiManager.addParameter(&custom_publish_interval);

    wifiManager.setTimeout(240); // 4 minutes

    //fetches ssid and pass and tries to connect
    //if it does not connect it starts an access point with the specified name
    //and goes into a blocking loop awaiting configuration
    String apName = String("ESP-") + ESP.getChipId();
    if (!wifiManager.autoConnect(apName.c_str())) {
        Serial.println("failed to connect and hit timeout");
        delay(3000);
        //reset and try again, or maybe put it to deep sleep
        ESP.reset();
        delay(5000);
    }

    //if you get here you have connected to the WiFi
    Serial.println("connected...yeey :)");

    //read updated parameters
    theConfig.thingSpeakChannel = String(custom_thingspeak_channel.getValue());
    theConfig.thingSpeakKey = String(custom_thingspeak_key.getValue());
    theConfig.publishInterval = String(custom_publish_interval.getValue()).toInt();

    //save the custom parameters to FS
    if (shouldSaveConfig) {
        Serial.println("saving config");
        saveConfig();
    }

    Serial.println("local ip");
    Serial.println(WiFi.localIP());
}