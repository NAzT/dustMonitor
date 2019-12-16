/*
 * Copyright Wilyarti Howard - 2019
 */

#include "EEPROMFunctions.h"
#include <Adafruit_ILI9341.h>
#include <TimeLib.h>
#include <Arduino.h>
#include "MHZ19.h"
#include "bitmap.c"
#include "main.h"
#include <EasyButton.h>
#include <cmath>
#include "optionsMenu.h"

MHZ19 myMHZ19;
HardwareSerial mySerial(1);
EasyButton middleButton(BUTTON_A);
EasyButton leftButton(BUTTON_B);
EasyButton rightButton(BUTTON_C);
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

EEPROMFunctions config;


void setup() {
    Serial.begin(9600);
    mySerial.begin(BAUDRATE, SERIAL_8N1, RX_PIN, TX_PIN);
    myMHZ19.begin(mySerial);
    myMHZ19.autoCalibration();
    tft.begin();
    tft.setRotation(0);

    runSetup();
    drawHeader();
    drawScales();

    middleButton.begin();
    leftButton.begin();
    rightButton.begin();
    middleButton.onPressed(cycleGraph);
    leftButton.onPressed(openOptionsMenu);
    rightButton.onPressed(cycleRange);
    drawButtons(mainButtons);
}

void runSetup() {
    drawHeader();
    tft.setTextColor(ILI9341_GREEN, CUSTOM_DARK);
    tft.setCursor(0, 50);
    tft.setTextSize(1);
    tft.println("Reading setup from EEPROM");
    int warmUpTime = optionsMatrix[1][config.conf.warmUpTime];
    if (config.loadConfig()) {
        tft.print("ThingSpeak Channel: ");
        tft.setTextColor(ILI9341_WHITE, CUSTOM_DARK);
        tft.println(config.conf.thingSpeakChannel);
        tft.setTextColor(ILI9341_GREEN, CUSTOM_DARK);
        tft.print("ThingSpeak API Key: ");
        tft.setTextColor(ILI9341_WHITE, CUSTOM_DARK);
        tft.println(config.conf.thingSpeakKey);

        tft.setTextColor(ILI9341_GREEN, CUSTOM_DARK);
        tft.print("publishInterval: ");
        tft.setTextColor(ILI9341_WHITE, CUSTOM_DARK);
        tft.println(config.conf.publishInterval);

        // --> not actual values but options from the optionsMatrix
        tft.setTextColor(ILI9341_GREEN, CUSTOM_DARK);
        tft.print("Graph Interval: ");
        tft.setTextColor(ILI9341_WHITE, CUSTOM_DARK);
        tft.println(optionsMatrix[0][config.conf.graphInterval]);
        currentOptions[0] = config.conf.graphInterval;

        tft.setTextColor(ILI9341_GREEN, CUSTOM_DARK);
        tft.print("warmUpTime: ");
        tft.setTextColor(ILI9341_WHITE, CUSTOM_DARK);
        tft.println(optionsMatrix[1][config.conf.warmUpTime]);
        currentOptions[1] = config.conf.warmUpTime;

        tft.setTextColor(ILI9341_GREEN, CUSTOM_DARK);
        tft.print("debug mode: ");
        tft.setTextColor(ILI9341_WHITE, CUSTOM_DARK);
        tft.println(optionsMatrix[2][config.conf.debugMode]);
        currentOptions[2] = config.conf.debugMode;

        tft.setTextColor(ILI9341_GREEN, CUSTOM_DARK);
        tft.print("language: ");
        tft.setTextColor(ILI9341_WHITE, CUSTOM_DARK);
        tft.println(optionsMatrix[3][config.conf.language]);
        currentOptions[3] = config.conf.language;
        // setup warm up timer
        warmUpTime = optionsMatrix[1][config.conf.warmUpTime];

    } else {
        tft.println("EEPROM could not load.");
    }
    getDataTimer = millis();
    config.printConfig();
    tft.println("\nWarming up...");
    int lastSecond = 0;
    while (true) {
        if ((millis() - getDataTimer) > optionsMatrix[1][config.conf.warmUpTime]) {
            break;
        }

        int cx = 240 / 2;
        int cy = 240;
        int r = 240 / 5;
        float j = 2;
        int lastX, lastY = 0;
        for (float i = 0; i < 2 * PI; i += 0.3) {
            if (lastSecond != ((warmUpTime - (millis() - getDataTimer)) / 1000)) {
                tft.setTextColor(ILI9341_ORANGE, CUSTOM_DARK);
                tft.setCursor(110, 230);
                tft.print((warmUpTime - (millis() - getDataTimer)) / 1000);
                tft.print("s     ");
            }
            lastSecond = ((warmUpTime - (millis() - getDataTimer)) / 1000);
            int x = cx + (r * cos(i));
            int y = cy + (r * sin(i));
            tft.fillCircle(x, y, j, ILI9341_ORANGE);
            Serial.print("i: ");
            Serial.print(i);
            Serial.print("X: ");
            Serial.print(x);
            Serial.print(" Y: ");
            Serial.println(y);
            lastX = x;
            lastY = y;
            delay(50);
        }
        for (float i = 0; i < 2 * PI; i += 0.3) {
            if (lastSecond != ((warmUpTime - (millis() - getDataTimer)) / 1000)) {
                tft.setTextColor(ILI9341_ORANGE, CUSTOM_DARK);
                tft.setCursor(110, 230);
                tft.print((warmUpTime - (millis() - getDataTimer)) / 1000);
                tft.print("s     ");
            }
            lastSecond = ((warmUpTime - (millis() - getDataTimer)) / 1000);
            int x = cx + (r * cos(i));
            int y = cy + (r * sin(i));
            tft.fillCircle(x, y, j, CUSTOM_DARK);
            Serial.print("i: ");
            Serial.print(i);
            Serial.print("X: ");
            Serial.print(x);
            Serial.print(" Y: ");
            Serial.println(y);
            lastX = x;
            lastY = y;
            delay(50);
        }


    }
}

void drawHeader() {
    tft.fillScreen(CUSTOM_DARK);
    tft.setTextColor(ILI9341_ORANGE);
    tft.setCursor(40, 20);
    tft.setTextSize(2);
    tft.drawBitmap(5, 5, opens3, 28, 32, ILI9341_YELLOW);
    tft.println("Air Monitor");
    tft.drawLine(40, 10, 240, 10, ILI9341_WHITE);
    tft.drawLine(0, 40, 240, 40, ILI9341_WHITE);
}

void drawButtons(char buttons[3][16]) {

    tft.setTextSize(1);
    tft.setTextColor(ILI9341_ORANGE);
    tft.drawRect(0, 305, 240, 15, ILI9341_YELLOW);
    tft.drawRect(0, 305, 80, 15, ILI9341_YELLOW);
    tft.drawRect(80, 305, 80, 15, ILI9341_YELLOW);
    tft.drawRect(160, 305, 80, 15, ILI9341_YELLOW);
    tft.setCursor(10, 310);
    tft.print(buttons[0]);
    tft.setCursor(100, 310);
    tft.print(buttons[1]);
    tft.setCursor(180, 310);
    tft.print(buttons[2]);

}

void loop() {
    middleButton.read();
    leftButton.read();
    rightButton.read();

    if (millis() - getDataTimer >= 50) {
        int curSecond = ((millis() - uptime) / 1000);
        //ticker(lastSecond, curSecond);
        int CO2 = 0;
        CO2 = myMHZ19.getCO2();
        int8_t Temp;
        Temp = myMHZ19.getTemperature();

        // Lazy update the CO2
        if (lastCO2PPM != CO2) {
            // CO2
            int color = ILI9341_CYAN;
            if (CO2 <= 500) {
                color = ILI9341_BLUE;
            } else if (CO2 <= 1000) {
                color = ILI9341_GREEN;
            } else if (CO2 <= 1500) {
                color = ILI9341_YELLOW;
            } else if (CO2 <= 2000) {
                color = ILI9341_ORANGE;
            } else if (CO2 <= 2500) {
                color = ILI9341_RED;
            } else if (CO2 <= 5000) {
                color = ILI9341_PURPLE;
            }
            tft.setTextColor(color);
            tft.fillRect(110, 65, 80, 20, CUSTOM_DARK);
            tft.setCursor(5, 65);
            tft.setTextSize(2);
            tft.print("CO2 PPM: ");
            tft.setCursor(110, 65);
            tft.print(CO2);
            tft.setTextColor(ILI9341_WHITE);
        }
        // Lazy update the Temp
        if (lastTemperature != Temp) {
            // Temp
            tft.fillRect(110, 95, 80, 20, CUSTOM_DARK);
            tft.setCursor(5, 95);
            tft.setTextSize(2);
            tft.print("Temp: ");
            tft.setCursor(110, 95);
            tft.print(Temp);
        }

        /*
         * Add data to each data set. Regardless of if it is the current option.
         * That way when the graph is cycled it displays a graph for each data set.
         * This saves having to calculate time periods from a memory allocated set of points.
         */
        // Cycle through intervals and add measurements if their individual timers are up.
        for (int t = 0; t < 5; t++) {
            int timerCheck = (millis() - graphIntervalTimer[t]);
            if (timerCheck > optionsMatrix[0][t] || graphIntervalTimer[t] == 0) {
                Serial.print("Adding data points for: ");
                Serial.println(t);
                addMeasurement(CO2, Temp, millis(), t);
                graphIntervalTimer[t] = millis();
                // This is the 't' you are looking for...
                if (t == currentOptions[0]) {
                    drawGraph(currentOptions[0], graphDataSet);
                }
            }
        }
        // draw graph if we are in selected graph or we have just started.
        if ((millis() - graphIntervalTimer[currentOptions[0]] > optionsMatrix[0][currentOptions[0]]) || graphIntervalTimer[currentOptions[0]] == 0) {
           // drawScales();
            drawGraph(currentOptions[0], graphDataSet);
        }

        lastTemperature = Temp;
        lastCO2PPM = CO2;
        lastSecond = curSecond;
        getDataTimer = millis();
    }
}

void addMeasurement(int CO2, int Temp, unsigned long Time, int intervalID) {
    for (int j = 0; j < 5; j++) {
        for (int i = 0; i < DATASET_LENGTH; i++) {
            graphPoints[intervalID][j][i] = graphPoints[intervalID][j][i + 1];
        }
    }
    graphPoints[intervalID][0][DATASET_LENGTH - 1] = CO2;
    graphPoints[intervalID][1][DATASET_LENGTH - 1] = Temp;
    timePoints[DATASET_LENGTH - 1] = Time;
}

void debug() {
    for (int intervalID = 0; intervalID < 5; intervalID++) {
        Serial.print("Interval: ");
        Serial.println(intervalID);
        for (int j = 0; j < 5; j++) {
            Serial.print("type ");
            Serial.print(j);
            Serial.print(": ");
            for (int i = 0; i < DATASET_LENGTH; i++) {
                Serial.print(graphPoints[intervalID][j][i]);
                Serial.print(",");
            }
            Serial.println();
        }
    }
}

void drawGraph(int intervalID, int selectedDataSet) {
    Serial.println("Clearing graph area.");
    tft.fillRect(28, 120, 240, 170, CUSTOM_DARK);
    tft.drawLine(30, 120, 30, xOffSet + 10, ILI9341_WHITE);
    tft.drawLine(0, xOffSet + 10, 240, xOffSet + 10, ILI9341_WHITE);
    int lastX = 0;
    int lastY = 0;
    int min = 0, max = 0;
    Serial.println("Finding min and max.");
    for (int j = 0; j < DATASET_LENGTH; j++) {
        if (!graphPoints[intervalID][selectedDataSet][j]) {
            continue;
        }
        if (graphPoints[intervalID][selectedDataSet][j] < min || min == 0) {
            min = graphPoints[intervalID][selectedDataSet][j];
        }
        if (graphPoints[intervalID][selectedDataSet][j] > max || max == 0) {
            max = graphPoints[intervalID][selectedDataSet][j];
        }
    }
    unsigned long oldScale = scale;
    Serial.println("Calculating scale.");
    calculateScale(min, max);
    if (oldScale != scale) {
        drawScales();
    }
    for (int i = 0; i < DATASET_LENGTH; i++) {
        if (graphPoints[intervalID][selectedDataSet][i] <= 0) {
            continue;
        }
        // Convert measurement using scaling
        int scaled = (graphPoints[intervalID][selectedDataSet][i] / scale);
        // Convert output to pixel co-ordinate
        int dotYLocation = xOffSet - scaled;
        // Space out our data points
        int currentX = (i * (240 / DATASET_LENGTH)) + 30;

        int color = ILI9341_WHITE;
        int CO2 = graphPoints[intervalID][selectedDataSet][i];
        if (CO2 <= 500) {
            color = ILI9341_CYAN;
        } else if (CO2 <= 1000) {
            color = ILI9341_GREEN;
        } else if (CO2 <= 1500) {
            color = ILI9341_YELLOW;
        } else if (CO2 <= 2000) {
            color = ILI9341_ORANGE;
        } else if (CO2 <= 2500) {
            color = ILI9341_RED;
        } else if (CO2 <= 5000) {
            color = ILI9341_PURPLE;
        }
        tft.fillCircle(currentX, dotYLocation, 2, color);
        if (lastX > 0 && lastY > 0) {
            tft.drawLine(currentX, dotYLocation, lastX, lastY, color);
        }
        Serial.print("Scale: ");
        Serial.print(scale);
        Serial.print(" Plotting at (");
        Serial.print(currentX);
        Serial.print(",");
        Serial.print(dotYLocation - 30);
        Serial.print("): ");
        Serial.println(graphPoints[intervalID][selectedDataSet][i]);

        lastX = currentX;
        lastY = dotYLocation;

    }
    for (int i = 1; i < 11; i++) {
        if (i < numYLabels) {
            tft.drawLine(30, (xOffSet - ((i * (yMax / numYLabels)))), 240, (xOffSet - ((i * (yMax / numYLabels)))),
                         0x8C71);
        }
        tft.drawLine((i * 20) + 30, xOffSet + 10, (i * 20) + 30, 120, 0x8C71);
    }

    Serial.println();
    Serial.println("/////////////// DEBUG  ////////////////////");
    debug();
    Serial.println("/////////////// DEBUG  ////////////////////");
}

void drawScales() {
    if(inSubMenu == true) {
        return;
    }
    inSubMenu = true;
    if (scale >= 32) {
        scale = 31;
    }
    tft.setTextSize(1);
    tft.setCursor(0, xOffSet + 20);
    Serial.print("Y Scale: ");
    Serial.println(scale);
    tft.fillRect(0, 115, 30, (xOffSet - 115), CUSTOM_DARK);
    tft.drawLine(30, 120, 30, xOffSet + 10, ILI9341_WHITE);
    tft.drawLine(0, xOffSet + 10, 240, xOffSet + 10, ILI9341_WHITE);
    for (int i = 0; i < numYLabels; i++) {
        int color = ILI9341_GREEN;
        int label = (i * (yMax / numYLabels) * scale);
        if (label <= 500) {
            color = ILI9341_CYAN;
        } else if (label <= 1000) {
            color = ILI9341_GREEN;
        } else if (label <= 1500) {
            color = ILI9341_YELLOW;
        } else if (label <= 2000) {
            color = ILI9341_ORANGE;
        } else if (label <= 2500) {
            color = ILI9341_RED;
        } else if (label <= 5000) {
            color = ILI9341_PURPLE;
        }
        tft.setTextColor(color);
        tft.setCursor(0, (xOffSet - ((i * (yMax / numYLabels)))));
        tft.print(i * (yMax / numYLabels) * scale);
    }
    tft.setTextColor(ILI9341_ORANGE);
    tft.setCursor(90, (xOffSet + 12));
    tft.fillRect(90, (xOffSet + 12), 120, 10, CUSTOM_DARK);
    if (graphDataSet == 0) {
        tft.print("CO2 ");
    } else {
        tft.print("Temp ");
    }
    tft.print(menuSettingsFields[0][currentOptions[0]]);
    tft.print(" Trend");
    inSubMenu = false;
}

void cycleGraph() {
    if(inSubMenu) { return; }
    if (graphDataSet == 0) {
        graphDataSet = 1;
    } else {
        graphDataSet = 0;
    }
    drawScales();
    drawGraph(currentOptions[0], graphDataSet);
}

void calculateScale(int min, int max) {
    // Scales below 160 are less than 1. Deal with them first.
    Serial.print("Min :");
    Serial.println(min);
    Serial.print("Max: ");
    Serial.println(max);

    if (min < 50) {
        Serial.println("Scale set to 0.3");
        scale = 0.3;
    } else if (min < 100) {
        Serial.println("Scale set to 0.6");
        scale = 0.6;
    } else if (min < 160) {
        Serial.println("Scale set to 1");
        scale = 1;
    } else if (min > 160) {
        Serial.println("Scale set to rounded.");
        int roundedScale = (min / 160);
        scale = roundedScale;
    }

    if (max) {
        unsigned long maxLong = max;
        unsigned long scaleCheck = 0;
        if (std::isless(scale, 1.0)) {
            Serial.println("Scale less than 1.");
            scaleCheck = (maxLong * scale);
        } else {
            Serial.println("Scale larger than 1.");
            scaleCheck = (maxLong / scale);
        }
        if (scaleCheck > 120) {
            Serial.println("Scale is too big. Decreasing.");
            if (scale < 1) {
                while ((maxLong * scale) > 160) {
                    Serial.println(scale);
                    scale += 0.01;
                }
            } else {
                while ((maxLong / scale) > 160) {
                    Serial.println(scale);
                    scale++;
                }
            }
        }
        Serial.print("Setting scale: ");
        Serial.println(scale);
    }
}

// TODO find space on the screen.
void ticker(int lastSec, int curSec) {
    // Update uptime first.
    tft.setTextColor(ILI9341_WHITE);
    if (lastSec != curSec) {
        tft.setTextSize(1);
        tft.fillRect(50, 307, 60, 15, CUSTOM_DARK);
        tft.setCursor(5, 307);
        tft.print("Uptime: ");
        tft.print(curSec);
        tft.print("s");
    }
}

void cycleRange() {
    if(inSubMenu) { return; }
    optionsMatrix[0][currentOptions[0] + 1] != -1 ? currentOptions[0]++
                                                  : currentOptions[0] = 0;
    drawScales();
    drawGraph(currentOptions[0], graphDataSet);
    config.conf.graphInterval = currentOptions[0];
    config.saveConfig();
}

void openOptionsMenu() {
    if (inSubMenu == true) {
        return;
    }
    inSubMenu = true;
    tft.fillRect(0, 0, 240, 320, CUSTOM_DARK);
    int selected = 0;
    int lastSelected = 0;
    int menuSettings[5] = {0, 0, 0, 0, 0};

    drawButtons(optionsButtons);
    optionsMenu::drawOptionsMenu(tft, leftButton, middleButton, rightButton, menuItems, menuSettingsFields,true, selected, lastSelected,
                                 menuSettings);
    Serial.print("In options menu");
    int menuCounter = millis();
    while (true) {
        leftButton.read();
        middleButton.read();
        rightButton.read();

        // leave menu after 60s
        if (millis() - menuCounter > (60*1000)){
            break;
        }

        if (selected != lastSelected) {
            optionsMenu::drawOptionsMenu(tft, leftButton, middleButton, rightButton,menuItems, menuSettingsFields, false, selected, lastSelected,
                                         menuSettings);
            lastSelected = selected;
        }
        if (middleButton.wasPressed()) {
            Serial.println("Middle button pressed");
            if (selected == 4) {
                // exit
                Serial.println("Saving to EEPROM and exiting.");
                config.saveConfig();
                break;
            } else {
                optionsMatrix[selected][currentOptions[selected] + 1] != -1 ? currentOptions[selected]++
                                                                            : currentOptions[selected] = 0;
                menuSettings[selected] = currentOptions[selected];
                optionsMenu::drawOptionsMenu(tft, leftButton, middleButton, rightButton, menuItems, menuSettingsFields,false, selected, lastSelected,
                                             menuSettings);
                switch (selected) {
                    case 0:
                        config.conf.graphInterval = currentOptions[selected]; // optionsMatrix[selected][currentOptions[selected]] --> store setting code rather than actual setting...
                        break;
                    case 1:
                        config.conf.warmUpTime = currentOptions[selected];
                        break;
                    case 2:
                        config.conf.debugMode = currentOptions[selected];
                        break;
                    case 3:
                        config.conf.language = currentOptions[selected];
                        break;
                    default:
                        Serial.println("No options selected.");
                        config.saveConfig();
                }
            }
            continue;
        }
        if (leftButton.wasPressed()) {
            selected > 0 ? selected-- : selected;
            Serial.print("Left button pressed: ");
            Serial.println(selected);
            continue;
        }
        if (rightButton.wasPressed()) {
            selected < 4 ? selected++ : selected;
            Serial.print("Right button pressed: ");
            Serial.println(selected);
            continue;
        }
        Serial.print(".");
        delay(100);
    }
    inSubMenu = false;
    // Force lazy update
    lastTemperature = lastCO2PPM = 0;
    drawHeader();
    drawScales();
    drawGraph(currentOptions[0], graphDataSet);
    drawButtons(mainButtons);
}
