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
#include "optionsMenu.h"
#include <math.h>

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
    rightButton.onPressed(cycleBacklight);
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
        tft.println("Setup successfully loaded.");
        tft.print("ThingSpeak Channel: ");
        tft.println(config.conf.thingSpeakChannel);
        tft.print("ThingSpeak API Key: ");
        tft.println(config.conf.thingSpeakKey);

        tft.print("publishInterval: ");
        tft.println(config.conf.publishInterval);

        // --> not actual values but options from the optionsMatrix
        tft.print("Graph Interval: ");
        tft.println(optionsMatrix[0][config.conf.graphInterval]);
        currentOptions[0] = config.conf.graphInterval;

        tft.print("warmUpTime: ");
        tft.println(optionsMatrix[1][config.conf.warmUpTime]);
        currentOptions[1] = config.conf.warmUpTime;

        tft.print("debug mode: ");
        tft.println(optionsMatrix[2][config.conf.debugMode]);
        currentOptions[2] = config.conf.debugMode;

        tft.print("language: ");
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
        for (float i = 0; i < 2*PI; i += 0.3) {
            if (lastSecond != ((warmUpTime-(millis()-getDataTimer))/1000)) {
                tft.setTextColor(ILI9341_ORANGE, CUSTOM_DARK);
                tft.setCursor(120, 240);
                tft.print((warmUpTime - (millis() - getDataTimer)) / 1000);
                tft.print("s");
            }
            lastSecond = ((warmUpTime-(millis()-getDataTimer))/1000);
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
        for (float i = 0; i < 2*PI; i += 0.3) {
            if (lastSecond != ((warmUpTime-(millis()-getDataTimer))/1000)) {
                tft.setTextColor(ILI9341_ORANGE, CUSTOM_DARK);
                tft.setCursor(120, 240);
                tft.print((warmUpTime - (millis() - getDataTimer)) / 1000);
                tft.print("s");
            }
            lastSecond = ((warmUpTime-(millis()-getDataTimer))/1000);
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

        // Add a graph data point every 8 mins.
        if ((millis() - graphIntervalTimer > optionsMatrix[0][currentOptions[0]]) || graphIntervalTimer == 0) {
            addMeasurement(CO2, Temp, millis());
            drawGraph();
            graphIntervalTimer = millis();
        }

        lastTemperature = Temp;
        lastCO2PPM = CO2;
        lastSecond = curSecond;
        getDataTimer = millis();
    }

}

void addMeasurement(int CO2, int Temp, unsigned long Time) {
    for (auto &graphPoint : graphPoints) {
        for (int i = 0; i < DATASET_LENGTH; i++) {
            graphPoint[i] = graphPoint[i + 1];
        }
    }
    graphPoints[0][DATASET_LENGTH - 1] = CO2;
    graphPoints[1][DATASET_LENGTH - 1] = Temp;

    timePoints[DATASET_LENGTH - 1] = Time;
}

void drawGraph() {
    Serial.println("Clearing graph area.");
    tft.fillRect(28, 120, 240, 170, CUSTOM_DARK);
    tft.drawLine(30, 120, 30, xOffSet + 10, ILI9341_WHITE);
    tft.drawLine(0, xOffSet + 10, 240, xOffSet + 10, ILI9341_WHITE);
    int lastX = 0;
    int lastY = 0;
    int min = 0, max = 0;
    Serial.println("Finding min and max.");
    for (int j = 0; j < DATASET_LENGTH; j++) {
        if (!graphPoints[selectedDataSet][j]) {
            continue;
        }
        if (graphPoints[selectedDataSet][j] < min || min == 0) {
            min = graphPoints[selectedDataSet][j];
        }
        if (graphPoints[selectedDataSet][j] > max || max == 0) {
            max = graphPoints[selectedDataSet][j];
        }
    }
    unsigned long oldScale = scale;
    Serial.println("Calculating scale.");
    calculateScale(min, max);
    if (oldScale != scale) {
        drawScales();
    }
    for (int i = 0; i < DATASET_LENGTH; i++) {
        if (graphPoints[selectedDataSet][i] <= 0) {
            continue;
        }
        // Convert measurement using scaling
        int scaled = (graphPoints[selectedDataSet][i] / scale);
        // Convert output to pixel co-ordinate
        int dotYLocation = xOffSet - scaled;
        // Space out our data points
        int currentX = (i * (240 / DATASET_LENGTH)) + 30;

        int color = ILI9341_WHITE;
        int CO2 = graphPoints[selectedDataSet][i];
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
        Serial.println(graphPoints[selectedDataSet][i]);

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
}

void drawScales() {
    if (scale >= 32) {
        scale = 31;
    }
    tft.setTextSize(1);
    tft.setCursor(0, xOffSet + 20);
    Serial.print("Y Scale: ");
    Serial.println(scale);
    tft.fillRect(0, 115, 240, (xOffSet - 115), CUSTOM_DARK);
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
    if (selectedDataSet == 0) {
        tft.print("CO2 ");
    } else {
        tft.print("Temp ");
    }
    tft.print(menuSettingsFields[0][currentOptions[0]]);
    tft.print(" Trend");
}

void cycleGraph() {
    Serial.println("Button has been pressed!");
    if (inSubMenu) {
        return;
    }
    if (selectedDataSet == 0) {
        selectedDataSet = 1;
    } else {
        selectedDataSet = 0;
    }
    drawGraph();
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

void openOptionsMenu() {
    if (inSubMenu) {
        return;
    }
    inSubMenu = true;
    tft.fillRect(0, 0, 240, 320, CUSTOM_DARK);
    int selected = 0;
    int lastSelected = 0;
    int menuSettings[5] = {0, 0, 0, 0, 0};

    drawButtons(optionsButtons);
    optionsMenu::drawOptionsMenu(tft, leftButton, middleButton, rightButton, true, selected, lastSelected,
                                 menuSettings);
    while (true) {
        leftButton.read();
        middleButton.read();
        rightButton.read();

        if (selected != lastSelected) {
            optionsMenu::drawOptionsMenu(tft, leftButton, middleButton, rightButton, false, selected, lastSelected,
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
                optionsMenu::drawOptionsMenu(tft, leftButton, middleButton, rightButton, false, selected, lastSelected,
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
        delay(100);
    }
    inSubMenu = false;
    // Force lazy update
    lastTemperature = lastCO2PPM = 0;
    drawHeader();
    drawScales();
    drawGraph();
    drawButtons(mainButtons);
}

void cycleBacklight() {
    if (inSubMenu) { return; }
    pinMode(22, OUTPUT);
    digitalWrite(22, LOW);
}