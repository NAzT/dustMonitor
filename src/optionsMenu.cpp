//
// Created by undef on 11/12/19.
//

#include <Adafruit_ILI9341.h>
#include <EasyButton.h>
#include "optionsMenu.h"

void optionsMenu::drawOptionsButton(Adafruit_ILI9341 tft) {
    tft.setTextSize(2);
    tft.setTextColor(ILI9341_ORANGE);
    tft.drawRect(0, 305, 240, 15, ILI9341_YELLOW);
    tft.drawRect(0, 305, 80, 15, ILI9341_YELLOW);
    tft.drawRect(80, 305, 80, 15, ILI9341_YELLOW);
    tft.drawRect(160, 305, 80, 15, ILI9341_YELLOW);
    tft.setCursor(10, 305);
    tft.print("UP");
    tft.setCursor(100, 305);
    tft.print("ENTER");
    tft.setCursor(180, 305);
    tft.print("DOWN");
}

void optionsMenu::drawOptionsMenu(Adafruit_ILI9341 tft, EasyButton ButtonA, EasyButton ButtonB, EasyButton ButtonC, int selected) {
    char menuItems [5][16] = { "Graph Range", "Warm Up Time", "Debug Mode", "Language", "Exit" };
    for (int i = 0; i < 5; i++) {
        if (selected == i) {
            tft.drawRect(0, i*60, 240, 60, ILI9341_YELLOW);
        } else {
            tft.fillRect(0, i*60, 240, 60, CUSTOM_DARK);
        }
        tft.setCursor(40, (i*60) + 30);
        tft.setTextSize(2);
        tft.setTextColor(ILI9341_WHITE);
        tft.print(menuItems[i]);
    }

}

