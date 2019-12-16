/*
 * Copyright Wilyarti Howard - 2019
 */

#ifndef DUSTMONITOR_OPTIONSMENU_H
#define DUSTMONITOR_OPTIONSMENU_H
#define CUSTOM_DARK 0x3186


class optionsMenu {

public:
    static void drawOptionsMenu(Adafruit_ILI9341, EasyButton, EasyButton, EasyButton, bool, int, int, int[5]);
};



#endif //DUSTMONITOR_OPTIONSMENU_H
