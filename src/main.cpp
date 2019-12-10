/*
 * Copyright Wilyarti Howard - 2019
 */


#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <TimeLib.h>
#include <Arduino.h>
#include "MHZ19.h"
#include <SoftwareSerial.h>
#include "bitmap.c"
#include "main.h"
#include <EasyButton.h>
#include <math.h>
#include "optionsMenu.h"

MHZ19 myMHZ19;
HardwareSerial mySerial(1);

#define RX_PIN 35
#define TX_PIN 33
#define BAUDRATE 9600
#define CUSTOM_DARK 0x3186
#define TFT_CS   27
#define TFT_DC   26
#define TFT_MOSI 23
#define TFT_CLK  18
#define TFT_RST  5
#define TFT_MISO 12

#define BUTTON_A  37  //          37 CENTRE
#define BUTTON_B  38 //          38 LEFT
#define BUTTON_C  39 //          39 RIGHT

unsigned long getDataTimer = 0;
unsigned long graphIntervalTimer = 0;
unsigned long uptime = millis();
int lastTemperature = 0;
int lastCO2PPM = 0;
int lastSecond = 0;


EasyButton graphToggleButton(BUTTON_A);
EasyButton optionsButton(BUTTON_B);


Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

void setup() {
    Serial.begin(9600);
    mySerial.begin(BAUDRATE, SERIAL_8N1, RX_PIN, TX_PIN);
    myMHZ19.begin(mySerial);
    myMHZ19.autoCalibration();
    tft.begin();
    tft.setRotation(0);
    tft.fillScreen(CUSTOM_DARK);
    tft.setTextColor(ILI9341_ORANGE);
    tft.setCursor(40, 20);
    tft.setTextSize(2);
    tft.drawBitmap(5, 5, opens3, 28, 32, ILI9341_YELLOW);
    tft.println("Air Monitor");
    tft.drawLine(40, 10, 240, 10, ILI9341_WHITE);
    tft.drawLine(0, 40, 240, 40, ILI9341_WHITE);

    drawScales();

    graphToggleButton.begin();
    optionsButton.begin();
    graphToggleButton.onPressed(onPressed);
    optionsButton.onPressed(optionsMenu);
    drawButtons();

}

void drawButtons() {
    tft.setTextSize(1);
    tft.setTextColor(ILI9341_ORANGE);
    tft.drawRect(0, 310, 240, 10, ILI9341_YELLOW);
    tft.drawRect(0, 310, 80, 10, ILI9341_YELLOW);
    tft.drawRect(80, 310, 80, 10, ILI9341_YELLOW);
    tft.drawRect(160, 310, 80, 10, ILI9341_YELLOW);
    tft.setCursor(10, 310);
    tft.print("OPTIONS");
    tft.setCursor(100, 310);
    tft.print("GRAPH");
    tft.setCursor(180, 310);
    tft.print("RANGE");

}

void loop() {
    graphToggleButton.read();
    optionsButton.read();

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
        if ((millis() - graphIntervalTimer > GRAPH_INTERVAL) || graphIntervalTimer == 0) {
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

        int color;
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
    tft.fillRect(90, (xOffSet+ 12), 120, 10, CUSTOM_DARK);
    if (selectedDataSet == 0) {
        tft.print("CO2 ");
    } else {
        tft.print("Temp ");
    }
    tft.print(GRAPH_INTERVAL_HOURS);
    tft.print(" Hour Trend");
}

void onPressed() {
    Serial.println("Button has been pressed!");
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
void ticker(int lastSecond, int curSecond) {
    // Update uptime first.
    tft.setTextColor(ILI9341_WHITE);
    if (lastSecond != curSecond) {
        tft.setTextSize(1);
        tft.fillRect(50, 307, 60, 15, CUSTOM_DARK);
        tft.setCursor(5, 307);
        tft.print("Uptime: ");
        tft.print(curSecond);
        tft.print("s");
    }
}

// TODO implement options menu.
void optionsMenu() {
    Serial.println("Button B has been pressed!");
    tft.fillRect(0, 0, 240, 320, ILI9341_GREENYELLOW);
    while(true) {
        optionsMenu::drawOptionsButton(tft);
        optionsMenu::drawOptionsMenu(tft);
        delay(1000);
    }
}