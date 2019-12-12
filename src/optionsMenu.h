//
// Created by undef on 11/12/19.
//

#ifndef DUSTMONITOR_OPTIONSMENU_H
#define DUSTMONITOR_OPTIONSMENU_H
#define CUSTOM_DARK 0x3186


class optionsMenu {

public:
    static void drawOptionsButton(Adafruit_ILI9341);
    static void drawOptionsMenu(Adafruit_ILI9341, EasyButton, EasyButton, EasyButton, int);
};



#endif //DUSTMONITOR_OPTIONSMENU_H
