/*
 * Copyright Wilyarti Howard - 2019
 */
#include "optionsMenu.h"

void optionsMenu::drawOptionsMenu(Adafruit_ILI9341 tft, char menuItems[5][16], char menuSettingsFields[5][5][16],
                                  bool firstRun,
                                  int selected, int lastSelected, const int menuSettings[5]) {

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

