/*
 * Copyright Wilyarti Howard - 2019
 */

#include <Adafruit_ILI9341.h>
#include <EasyButton.h>
#include "optionsMenu.h"
#include "main.h"


void optionsMenu::drawOptionsMenu(Adafruit_ILI9341 tft, EasyButton ButtonA, EasyButton ButtonB, EasyButton ButtonC, bool firstRun,
                                  int selected, int lastSelected, int menuSettings[5]) {


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

