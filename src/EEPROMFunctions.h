/*
 * Copyright Wilyarti Howard - 2019
 */

#ifndef DUSTMONITOR_EEPROMFUNCTIONS_H
#define DUSTMONITOR_EEPROMFUNCTIONS_H

#include <EEPROM.h>

#define INITIALIZED_MARKER 7217 // original was 7216

typedef struct {
    String thingSpeakChannel = "NOT SET";
    String thingSpeakKey = "SET ME";
    int publishInterval = 60;
    int initializedFlag = 0;
    //TODO implement these options/functions
    int graphInterval = 1000 * 60 * 60 * 8;
    int warmUpTime = 60 * 1000 * 3;
    int debugMode = 0; // 0 = false
    int language = 0; // 0 = english - TO BE IMPLEMENTED
} _Config;


class EEPROMFunctions {

public:
    bool loadConfig();

    void saveConfig();

    void printConfig();

    int eepromWriteString(int, String);

    String eepromReadString(int);
    _Config conf;

};


#endif //DUSTMONITOR_EEPROMFUNCTIONS_H
