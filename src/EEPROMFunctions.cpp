//
// Created by undef on 14/12/19.
//

#include "EEPROMFunctions.h"

void EEPROMFunctions::saveConfig() {
    EEPROM.begin(512);
    int addr = 0;
    EEPROM.put(addr, INITIALIZED_MARKER);
    addr += sizeof(INITIALIZED_MARKER);
    addr = eepromWriteString(addr, conf.thingSpeakChannel);
    addr = eepromWriteString(addr, conf.thingSpeakKey);
    EEPROM.put(addr, conf.publishInterval);
    addr += sizeof(conf.publishInterval);
    EEPROM.put(addr, conf.graphInterval);
    addr += sizeof(conf.graphInterval);
    EEPROM.put(addr, conf.warmUpTime);
    addr += sizeof(conf.warmUpTime);
    EEPROM.put(addr, conf.debugMode);
    addr += sizeof(conf.debugMode);
    EEPROM.put(addr, conf.language);
    addr += sizeof(conf.language);
    // update loadConfig() and printConfig() if anything else added here
    EEPROM.commit();
    Serial.println("Config saved:"); printConfig();
}

bool EEPROMFunctions::loadConfig() {
    EEPROM.begin(512);

    int addr = 0;
    EEPROM.get(addr, conf.initializedFlag); addr += sizeof(conf.publishInterval);
    if (conf.initializedFlag != INITIALIZED_MARKER) {
        Serial.println("*** No config!");
        return false;
    }

    conf.thingSpeakChannel = eepromReadString(addr); addr += conf.thingSpeakChannel.length() + 1;
    conf.thingSpeakKey = eepromReadString(addr); addr += conf.thingSpeakKey.length() + 1;
    EEPROM.get(addr, conf.publishInterval); addr += sizeof(conf.publishInterval);
    EEPROM.get(addr, conf.graphInterval); addr += sizeof(conf.graphInterval);
    EEPROM.get(addr, conf.warmUpTime); addr += sizeof(conf.warmUpTime);
    EEPROM.get(addr, conf.debugMode); addr += sizeof(conf.debugMode);
    EEPROM.get(addr, conf.language); addr += sizeof(conf.language);
    EEPROM.commit();

    Serial.println("Config loaded: "); printConfig();
    return true;
}

void EEPROMFunctions::printConfig() {
    Serial.print("ThingSpeak channel: '"); Serial.print(conf.thingSpeakChannel);
    Serial.print("', ThingSpeak Key="); Serial.print(conf.thingSpeakKey);
    Serial.print(", publishInterval="); Serial.print(conf.publishInterval);
    Serial.print(", graphInterval="); Serial.print(conf.graphInterval);
    Serial.print(", warmUpTime="); Serial.print(conf.warmUpTime);
    Serial.print(", debugMode="); Serial.print(conf.debugMode);
    Serial.print(", language="); Serial.print(conf.language);

    Serial.println();
}

// get these eeprom read/write String functions in Arduino code somewhere?
int EEPROMFunctions::eepromWriteString(int addr, String s) {
    int l = s.length();
    for (int i = 0; i < l; i++) {
        EEPROM.write(addr++, s.charAt(i));
    }
    EEPROM.write(addr++, 0x00);
    return addr;
}

String EEPROMFunctions::eepromReadString(int addr) {
    String s;
    char c;
    while ((c = EEPROM.read(addr++)) != 0x00) {
        s += c;
    }
    return s;
}