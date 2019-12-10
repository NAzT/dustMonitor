//
// Created by undef on 11/12/19.
//

#include <Adafruit_ILI9341.h>
#include "optionsMenu.h"

void optionsMenu::drawOptionsButton(Adafruit_ILI9341 tft) {
    tft.setTextSize(1);
    tft.setTextColor(ILI9341_ORANGE);
    tft.drawRect(0, 310, 240, 10, ILI9341_YELLOW);
    tft.drawRect(0, 310, 80, 10, ILI9341_YELLOW);
    tft.drawRect(80, 310, 80, 10, ILI9341_YELLOW);
    tft.drawRect(160, 310, 80, 10, ILI9341_YELLOW);
    tft.setCursor(10, 310);
    tft.print("UP");
    tft.setCursor(100, 310);
    tft.print("ENTER");
    tft.setCursor(180, 310);
    tft.print("DOWN");
}

void optionsMenu::drawOptionsMenu(Adafruit_ILI9341 tft) {
    char menuItems [4][16] = { "Graph Range", "Warm Up Time", "Debug Mode", "Language" };
    for (int i = 0; i < 4; i++) {
        tft.setCursor(40, i*60);
        tft.setTextSize(4);
        tft.print(menuItems[i]);
    }

}

