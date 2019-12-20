/*
 * Copyright Wilyarti Howard - 2019
 */

#ifndef DUSTMONITOR_OPTIONSMENU_H
#define DUSTMONITOR_OPTIONSMENU_H
#endif //DUSTMONITOR_OPTIONSMENU_H

#define CUSTOM_DARK 0x3186

#include <Adafruit_ILI9341.h>

class optionsMenu {
public:
    static void drawOptionsMenu(Adafruit_ILI9341, char[5][16], char[5][5][16], bool, int, int, const int[5]);
};



