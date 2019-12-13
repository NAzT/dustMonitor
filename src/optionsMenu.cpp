//
// Created by undef on 11/12/19.
//

#include <Adafruit_ILI9341.h>
#include <EasyButton.h>
#include "optionsMenu.h"



void optionsMenu::drawOptionsMenu(Adafruit_ILI9341 tft, EasyButton ButtonA, EasyButton ButtonB, EasyButton ButtonC, bool firstRun,
                                  int selected, int lastSelected, int menuSettings[5]) {

    char menuItems[5][16] = {"Graph Range", "Warm Up", "Debug Mode", "Language", "Exit"};
    char menuSettingsFields[5][5][16] {
            {"8hrs", "3hrs", "1hr", "30min", "10min"},
            {"10min", "3mins", "1min", "Off" },
            {"On", "Off"},
            {"English", "Korean"},
            {" "},
    };
    // Lazy update
    if (!firstRun) {
        tft.drawRect(0, selected * 60, 240, 60, ILI9341_YELLOW);
        tft.fillRect(0, lastSelected * 60, 240, 60, CUSTOM_DARK);
        tft.setTextSize(2);
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(20, (selected * 60) + 30);
        tft.print(menuItems[selected]);
        tft.print(": ");
        tft.print(menuSettingsFields[selected][menuSettings[selected]]);
        tft.setCursor(20, (lastSelected * 60) + 30);
        tft.print(menuItems[lastSelected]);
        return;
    }
    for (int i = 0; i < 5; i++) {
        if (selected == i) {
            tft.drawRect(0, i * 60, 240, 60, ILI9341_YELLOW);
        } else {
            tft.fillRect(0, i * 60, 240, 60, CUSTOM_DARK);
        }
        tft.setCursor(20, (i * 60) + 30);
        tft.setTextSize(2);
        tft.setTextColor(ILI9341_WHITE);
        tft.print(menuItems[i]);
    }

}
