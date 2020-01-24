/*
 * Copyright Wilyarti Howard - 2019
 */
#include <TimeLib.h>
#include <Arduino.h>
#include <EasyButton.h>
#include "MHZ19.h"
#include "bitmap.c"
#include "main.h"
#include "optionsMenu.h"
#include "EEPROMFunctions.h"
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <ArduinoJson.h>

// TTGO V13
#define T4_V13

#include "T4_V13.h"
#include <TFT_eSPI.h>
#include <SPI.h>
#include <Wire.h>

TFT_eSPI tft = TFT_eSPI(); // Invoke custom library
SPIClass sdSPI(VSPI);
#define IP5306_ADDR         0X75
#define IP5306_REG_SYS_CTL0 0x00
uint8_t state = 0;
uint8_t g_btns[] = BUTTONS_MAP;
char buff[512];
// TTGO V13

// CCS811
#include "Adafruit_CCS811.h"

Adafruit_CCS811 ccs;
// CCS811

// BLE
BLEServer *pServer = NULL;
BLECharacteristic *tvocCharacteristic = NULL;
BLECharacteristic *co2Characteristic = NULL;
BLECharacteristic *graphCharacteristic = NULL;
float bleTvocValue = 0;
float bleCo2Value = 0;
char bleTemperatureString[16];
char bleCo2String[16];

MHZ19 myMHZ19;
HardwareSerial mySerial(1);
EasyButton middleButton(BUTTON_A);
EasyButton leftButton(BUTTON_B);
EasyButton rightButton(BUTTON_C);

EEPROMFunctions config;

int16_t nTempOut;
int16_t uHumOut;

class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer *pServer) {
        // Rest ble timer on device connect.
        bleGraphTimer = millis() - 60000;
        deviceConnected = true;
        BLEDevice::startAdvertising();
    };

    void onDisconnect(BLEServer *pServer) {
        deviceConnected = false;
    }
};

void setup() {
    esp_log_level_set("*", ESP_LOG_DEBUG);
    Serial.begin(9600);
    mySerial.begin(BAUDRATE, SERIAL_8N1, RX_PIN, TX_PIN);
    myMHZ19.begin(mySerial);
    myMHZ19.autoCalibration();

    if (!ccs.begin()) {
        Serial.println("Failed to start sensor! Please check your wiring.");
    }
    while (!ccs.available());

    initTFT();
    runSetup();
    drawHeader();
    drawScales();

    // Setup buttons
    middleButton.begin();
    leftButton.begin();
    rightButton.begin();
    middleButton.onPressed(cycleGraph);
    leftButton.onPressed(openOptionsMenu);
    rightButton.onPressed(cycleRange);
    drawButtons(mainButtons);

    initBle();

}

void loop() {
    middleButton.read();
    leftButton.read();
    rightButton.read();
    // ************************************ BLE ********************************************************************* //
    // Deal with BLE first
    // Push BLE values first
    // notify changed bleTvocValue
    // 5 sec
    if (deviceConnected && (millis() - bleTimer >= 5000)) {
        // update every second
        Serial.println("Device connected. Pushing values.");
        Serial.print("Temperature: ");
        Serial.println(bleTvocValue);
        Serial.print("Co2: ");
        Serial.println(bleCo2Value);

        sprintf(bleTemperatureString, "%4.2f", bleTvocValue);
        sprintf(bleCo2String, "%4.2f", bleCo2Value);
        tvocCharacteristic->setValue(bleTemperatureString);
        co2Characteristic->setValue(bleCo2String);

        tvocCharacteristic->notify();
        co2Characteristic->notify();
        bleTimer = millis();

    }
    // TODO reduce timer
    if (deviceConnected && (millis() - bleGraphTimer >= 30000)) {
        // update every second
        Serial.println("Device connected. Pushing all the values.");
        const int step = 20;
        const int capacity = JSON_ARRAY_SIZE(step * 4) + JSON_OBJECT_SIZE(2 * 4);
        char chunk[512];
        for (int d = 0; d < BLE_DATASET_ROWS; d++) {
            int sequenceID = 0;
            for (int i = 0; i < BLE_DATASETLENGTH; i += step) {
                StaticJsonDocument<capacity> doc;

                bool haveData = false;
                int dataCount = 0;

                JsonArray valueArray = doc.createNestedArray();
                JsonArray timerArray = doc.createNestedArray();
                // TODO fix the below, make it dynamic
                for (int j = i; j < (i + step) && j < BLE_DATASETLENGTH; j++) {
                    // add some values
                    if (bleGraphPoints[d][j] != 0) {
                        valueArray.add(serialized(String(bleGraphPoints[d][j])));
                        timerArray.add(bleTimePoints[j]);
                        haveData = true;
                        dataCount++;
                    }
                }
                // Any data?
                if (!haveData) {
                    Serial.print(".");
                    continue;
                }
                JsonObject metadata = doc.createNestedObject();
                metadata["timenow"] = millis();
                metadata["dataID"] = d;
                metadata["count"] = dataCount;
                metadata["sequenceID"] = sequenceID;
                // serialize the array and send the result to Serial
                serializeJson(doc, chunk, sizeof(chunk));
                Serial.println(chunk);
                graphCharacteristic->setValue(chunk);
                graphCharacteristic->notify();
                delay(10);
                sequenceID++;
            }
        }
        // Send end command
        StaticJsonDocument<capacity> doc;
        JsonArray valueArray = doc.createNestedArray();
        JsonArray timerArray = doc.createNestedArray();
        JsonObject metadata = doc.createNestedObject();
        metadata["timenow"] = millis();
        metadata["dataID"] = 255;
        metadata["count"] = 0;
        metadata["sequenceID"] = 65536;
        // serialize the array and send the result to Serial
        serializeJson(doc, chunk, sizeof(chunk));
        Serial.println(chunk);
        graphCharacteristic->setValue(chunk);
        graphCharacteristic->notify();
        bleGraphTimer = millis();
    }
    // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        Serial.println("start advertising");
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
        // do stuff here on connecting
        bleGraphTimer = millis() - 60000;
        oldDeviceConnected = deviceConnected;
    }
    // ************************************ BLE ********************************************************************* //

    if (millis() - getDataTimer >= 50) {
        int curSecond = ((millis() - uptime) / 1000);
        //ticker(lastSecond, curSecond);
        int CO2 = myMHZ19.getCO2();
        int TVOC = 0;
        int CO2CCS = 0;
        if(ccs.available()){
            if(!ccs.readData()){
                Serial.print("CO2: ");
                Serial.print(ccs.geteCO2());
                CO2CCS = ccs.geteCO2();
                Serial.print("ppm, TVOC: ");
                Serial.print(ccs.getTVOC());
                TVOC =ccs.getTVOC();
            }
        }else{
            TVOC = lastTVOC;
            CO2CCS = lastCO2PPCCCS;
        }

        // BLE conversion
        bleCo2Value = CO2;
        bleTvocValue = TVOC;

        // CO2
        if (lastCO2PPM != CO2 || lastCO2PPCCCS != CO2CCS) {
            int color = TFT_CYAN;
            if (CO2 <= 500) {
                color = TFT_BLUE;
            } else if (CO2 <= 1000) {
                color = TFT_GREEN;
            } else if (CO2 <= 1500) {
                color = TFT_YELLOW;
            } else if (CO2 <= 2000) {
                color = TFT_ORANGE;
            } else if (CO2 <= 2500) {
                color = TFT_RED;
            } else if (CO2 <= 5000) {
                color = TFT_PURPLE;
            }
            tft.setTextColor(color);
            tft.fillRect(110, 65, 120, 20, CUSTOM_DARK);
            tft.setCursor(5, 65);
            tft.setTextSize(2);
            tft.print("CO2 PPM: ");
            Serial.print("CO2 PPM: ");
            Serial.print(CO2);
            tft.setCursor(110, 65);
            tft.print(CO2);
            if (CO2CCS > 0) {
                tft.print("/");
                tft.print(CO2CCS);
            } else {
                tft.print("/");
                tft.print(lastCO2PPCCCS);
            }
            tft.setTextColor(TFT_WHITE);
        }

        // TVOC
        if (lastTVOC != TVOC && TVOC > 0) {
            tft.fillRect(110, 95, 80, 20, CUSTOM_DARK);
            tft.setCursor(5, 95);
            tft.setTextSize(2);
            tft.print("TVOC: ");
            Serial.print("TVOC: ");
            Serial.println(TVOC);
            tft.setCursor(110, 95);
            tft.print(TVOC);
        }

        /*
         * Add data to each data set. Regardless of if it is the current option.
         * That way when the graph is cycled it displays a graph for each data set.
         * This saves having to calculate time periods from a memory allocated set of points.
         */
        // Cycle through intervals and add measurements if their individual timers are up.
        for (int t = 0; t < 5; t++) {
            unsigned long timerCheck = (millis() - graphIntervalTimer[t]);
            if (timerCheck > optionsMatrix[0][t] || graphIntervalTimer[t] == 0) {
                Serial.print("Adding data points for: ");
                Serial.println(t);
                addMeasurement(CO2, TVOC, millis(), t);
                graphIntervalTimer[t] = millis();
                // This is the 't' you are looking for...
                if (t == currentOptions[0]) {
                    drawGraph(currentOptions[0], graphDataSet);
                }
            }
        }
        long bleGraphTimerCheck = (millis() - bleGraphDatasetTimer);
        if (bleGraphTimerCheck > bleGraphInterval || bleGraphDatasetTimer == 0) {
            Serial.println("Adding data to ble graph.");
            addBleGraphMeasurement(bleCo2Value, bleTvocValue, millis());
            bleGraphDatasetTimer = millis(); // reset timer
        }
        // draw graph if we are in selected graph or we have just started.
        if ((millis() - graphIntervalTimer[currentOptions[0]] > optionsMatrix[0][currentOptions[0]]) ||
            graphIntervalTimer[currentOptions[0]] == 0) {
            // drawScales();
            drawGraph(currentOptions[0], graphDataSet);
        }

        lastTVOC = TVOC;
        lastCO2PPM = CO2;
        lastCO2PPCCCS = CO2CCS;
        lastSecond = curSecond;
        getDataTimer = millis();
    }
    if (millis() - backlightTimer >= optionsMatrix[2][currentOptions[2]]) {
        // turn off backlight
        Serial.print("Turning off backlight: ");
        Serial.println(optionsMatrix[2][currentOptions[2] + 1]);
        digitalWrite(TFT_BL, LOW);
        backlight = false;
        backlightTimer = millis();
    }
}

void initTFT() {
    tft.init();
    tft.setRotation(0);
    tft.fillScreen(TFT_BLACK);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE);
    tft.setCursor(0, 0);

    if (TFT_BL > 0) {
        pinMode(TFT_BL, OUTPUT);
        digitalWrite(TFT_BL, HIGH);
    }

    tft.setTextFont(1);
    tft.setTextSize(1);
}

void initBle() {
    // Create the BLE Device
    BLEDevice::init("Air Quality Monitor");
    BLEDevice::setMTU(BLE_MTU);

    // Create the BLE Server
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    // Create the BLE Service
    BLEService *pService = pServer->createService(BLEUUID(BLE_SERVICE_ID));

    // Create a BLE Characteristic
    tvocCharacteristic = pService->createCharacteristic(
            BLEUUID(BLE_TVOC_CHARACTERISTIC),
            BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE |
            BLECharacteristic::PROPERTY_NOTIFY |
            BLECharacteristic::PROPERTY_INDICATE
    );

    co2Characteristic = pService->createCharacteristic(
            BLEUUID(BLE_HUMIDITY_CHARACTERISTIC),
            BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE |
            BLECharacteristic::PROPERTY_NOTIFY |
            BLECharacteristic::PROPERTY_INDICATE
    );

    graphCharacteristic = pService->createCharacteristic(
            BLEUUID(BLE_GRAPH_CHARACTERISTIC),
            BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE |
            BLECharacteristic::PROPERTY_NOTIFY |
            BLECharacteristic::PROPERTY_INDICATE
    );

    // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
    // Create a BLE Descriptor
    tvocCharacteristic->addDescriptor(new BLE2902());
    co2Characteristic->addDescriptor(new BLE2902());
    graphCharacteristic->addDescriptor(new BLE2902());

    tvocCharacteristic->setValue("0.0");
    co2Characteristic->setValue("0.0");
    graphCharacteristic->setValue("0.0");
    // Start the service
    pService->start();

    // Start advertising
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(BLEUUID(BLE_SERVICE_ID));
    pAdvertising->setScanResponse(false);
    pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
    BLEDevice::startAdvertising();
    Serial.println("Waiting a client connection to notify...");
}

void runSetup() {
    drawHeader();
    tft.setTextColor(TFT_GREEN, CUSTOM_DARK);
    tft.setCursor(0, 50);
    tft.setTextSize(1);
    tft.println("Reading setup from EEPROM");
    int warmUpTime = optionsMatrix[1][config.conf.warmUpTime];
    if (config.loadConfig()) {
        tft.print("ThingSpeak Channel: ");
        tft.setTextColor(TFT_WHITE, CUSTOM_DARK);
        tft.println(config.conf.thingSpeakChannel);
        tft.setTextColor(TFT_GREEN, CUSTOM_DARK);
        tft.print("ThingSpeak API Key: ");
        tft.setTextColor(TFT_WHITE, CUSTOM_DARK);
        tft.println(config.conf.thingSpeakKey);

        tft.setTextColor(TFT_GREEN, CUSTOM_DARK);
        tft.print("publishInterval: ");
        tft.setTextColor(TFT_WHITE, CUSTOM_DARK);
        tft.println(config.conf.publishInterval);

        // --> not actual values but options from the optionsMatrix
        tft.setTextColor(TFT_GREEN, CUSTOM_DARK);
        tft.print("Graph Interval: ");
        tft.setTextColor(TFT_WHITE, CUSTOM_DARK);
        tft.println(optionsMatrix[0][config.conf.graphInterval]);
        currentOptions[0] = config.conf.graphInterval;

        tft.setTextColor(TFT_GREEN, CUSTOM_DARK);
        tft.print("warmUpTime: ");
        tft.setTextColor(TFT_WHITE, CUSTOM_DARK);
        tft.println(optionsMatrix[1][config.conf.warmUpTime]);
        currentOptions[1] = config.conf.warmUpTime;

        tft.setTextColor(TFT_GREEN, CUSTOM_DARK);
        tft.print("backlight timeout: ");
        tft.setTextColor(TFT_WHITE, CUSTOM_DARK);
        tft.println(optionsMatrix[2][config.conf.debugMode]);
        currentOptions[2] = config.conf.debugMode;

        tft.setTextColor(TFT_GREEN, CUSTOM_DARK);
        tft.print("language: ");
        tft.setTextColor(TFT_WHITE, CUSTOM_DARK);
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
        Serial.print(".");
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
                tft.setTextColor(TFT_ORANGE, CUSTOM_DARK);
                tft.setCursor(110, 230);
                tft.print((warmUpTime - (millis() - getDataTimer)) / 1000);
                tft.print("s     ");
            }
            lastSecond = ((warmUpTime - (millis() - getDataTimer)) / 1000);
            int x = cx + (r * cos(i));
            int y = cy + (r * sin(i));
            tft.fillCircle(x, y, j, TFT_ORANGE);
            lastX = x;
            lastY = y;
            delay(50);
        }
        for (float i = 0; i < 2 * PI; i += 0.3) {
            if (lastSecond != ((warmUpTime - (millis() - getDataTimer)) / 1000)) {
                tft.setTextColor(TFT_ORANGE, CUSTOM_DARK);
                tft.setCursor(110, 230);
                tft.print((warmUpTime - (millis() - getDataTimer)) / 1000);
                tft.print("s     ");
            }
            lastSecond = ((warmUpTime - (millis() - getDataTimer)) / 1000);
            int x = cx + (r * cos(i));
            int y = cy + (r * sin(i));
            tft.fillCircle(x, y, j, CUSTOM_DARK);
            lastX = x;
            lastY = y;
            delay(50);
        }


    }
}

void drawHeader() {
    tft.fillScreen(CUSTOM_DARK);
    tft.setTextColor(TFT_ORANGE);
    tft.setCursor(40, 20);
    tft.setTextSize(2);
    tft.drawBitmap(5, 5, opens3, 28, 32, TFT_YELLOW);
    tft.println("Air Monitor");
    tft.drawLine(40, 10, 240, 10, TFT_WHITE);
    tft.drawLine(0, 40, 240, 40, TFT_WHITE);
}

void drawButtons(char buttons[3][16]) {

    tft.setTextSize(1);
    tft.setTextColor(TFT_ORANGE);
    tft.drawRect(0, 305, 240, 15, TFT_YELLOW);
    tft.drawRect(0, 305, 80, 15, TFT_YELLOW);
    tft.drawRect(80, 305, 80, 15, TFT_YELLOW);
    tft.drawRect(160, 305, 80, 15, TFT_YELLOW);
    tft.setCursor(10, 310);
    tft.print(buttons[0]);
    tft.setCursor(100, 310);
    tft.print(buttons[1]);
    tft.setCursor(180, 310);
    tft.print(buttons[2]);

}


void addMeasurement(int CO2, int Tvoc, unsigned long Time, int intervalID) {
    for (int j = 0; j < 5; j++) {
        for (int i = 0; i < DATASET_LENGTH; i++) {
            graphPoints[intervalID][j][i] = graphPoints[intervalID][j][i + 1];
        }
    }
    Serial.println("Added to graph: ");
    Serial.println("CO2: ");
    Serial.print(CO2);
    Serial.print("Tvoc: ");
    Serial.print(Tvoc);
    graphPoints[intervalID][0][DATASET_LENGTH - 1] = CO2;
    graphPoints[intervalID][1][DATASET_LENGTH - 1] = Tvoc;
    timePoints[DATASET_LENGTH - 1] = Time;
}

void addBleGraphMeasurement(float CO2, float Temp, unsigned long Time) {
    for (int j = 0; j < BLE_DATASET_ROWS; j++) {
        for (int i = 0; i < BLE_DATASETLENGTH; i++) {
            bleGraphPoints[j][i] = bleGraphPoints[j][i + 1];
        }
    }
    bleGraphPoints[0][BLE_DATASETLENGTH - 1] = CO2;
    bleGraphPoints[1][BLE_DATASETLENGTH - 1] = Temp;
    // TODO dust ppm etc
    for (int i = 0; i < BLE_DATASETLENGTH; i++) {
        bleTimePoints[i] = bleTimePoints[i + 1];
    }
    bleTimePoints[BLE_DATASETLENGTH - 1] = Time;
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
    Serial.print("BLE Graph Points");
    for (int row = 0; row < BLE_DATASET_ROWS; row++) {
        Serial.print("Row: ");
        Serial.println(row);
        Serial.print("Data: ");
        for (int k = 0; k < BLE_DATASETLENGTH; k++) {
            Serial.print(bleGraphPoints[row][k]);
            Serial.print(",");
        }
        Serial.println("");
    }
}

void drawGraph(int intervalID, int selectedDataSet) {
    Serial.println("Clearing graph area.");
    tft.fillRect(28, 120, 240, 170, CUSTOM_DARK);
    tft.drawLine(30, 120, 30, xOffSet + 10, TFT_WHITE);
    tft.drawLine(0, xOffSet + 10, 240, xOffSet + 10, TFT_WHITE);
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

        int color = TFT_WHITE;
        int CO2 = graphPoints[intervalID][selectedDataSet][i];
        if (CO2 <= 500) {
            color = TFT_CYAN;
        } else if (CO2 <= 1000) {
            color = TFT_GREEN;
        } else if (CO2 <= 1500) {
            color = TFT_YELLOW;
        } else if (CO2 <= 2000) {
            color = TFT_ORANGE;
        } else if (CO2 <= 2500) {
            color = TFT_RED;
        } else if (CO2 <= 5000) {
            color = TFT_PURPLE;
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
    //Serial.println("/////////////// DEBUG  ////////////////////");
    //debug();
    //Serial.println("/////////////// DEBUG  ////////////////////");
}

void drawScales() {
    if (inSubMenu != 0) {
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
    tft.drawLine(30, 120, 30, xOffSet + 10, TFT_WHITE);
    tft.drawLine(0, xOffSet + 10, 240, xOffSet + 10, TFT_WHITE);
    for (int i = 0; i < numYLabels; i++) {
        int color = TFT_GREEN;
        int label = (i * (yMax / numYLabels) * scale);
        if (label <= 500) {
            color = TFT_CYAN;
        } else if (label <= 1000) {
            color = TFT_GREEN;
        } else if (label <= 1500) {
            color = TFT_YELLOW;
        } else if (label <= 2000) {
            color = TFT_ORANGE;
        } else if (label <= 2500) {
            color = TFT_RED;
        } else if (label <= 5000) {
            color = TFT_PURPLE;
        }
        tft.setTextColor(color);
        tft.setCursor(0, (xOffSet - ((i * (yMax / numYLabels)))));
        tft.print(i * (yMax / numYLabels) * scale);
    }
    tft.setTextColor(TFT_ORANGE);
    tft.setCursor(90, (xOffSet + 12));
    tft.fillRect(90, (xOffSet + 12), 120, 10, CUSTOM_DARK);
    if (graphDataSet == 0) {
        tft.print("CO2 ");
    } else {
        tft.print("TVOC ");
    }
    tft.print(menuSettingsFields[0][currentOptions[0]]);
    tft.print(" Trend");
    inSubMenu = false;
}

void cycleGraph() {
    if (!backlight) {
        digitalWrite(TFT_BL, HIGH);
        backlight = true;
        backlightTimer = millis();
    }
    if (inSubMenu) { return; }
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
    Serial.print("Min: ");
    Serial.println(min);
    Serial.print("Max: ");
    Serial.println(max);

    if (max < 50) {
        Serial.println("Scale set to 0.3");
        scale = 0.3;
    } else if (max < 100) {
        Serial.println("Scale set to 0.6");
        scale = 0.5;
    } else if (max < 160) {
        Serial.println("Scale set to 1");
        scale = 1;
    } else if (max > 160) {
        Serial.println("Scale set to rounded.");
        int roundedScale = (max / 160);
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
                    scale += 0.1;
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
    tft.setTextColor(TFT_WHITE);
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
    if (!backlight) {
        digitalWrite(TFT_BL, HIGH);
        backlight = true;
        backlightTimer = millis();
    }
    if (inSubMenu) { return; }
    optionsMatrix[0][currentOptions[0] + 1] != -1 ? currentOptions[0]++
                                                  : currentOptions[0] = 0;
    drawScales();
    drawGraph(currentOptions[0], graphDataSet);
    config.conf.graphInterval = currentOptions[0];
    config.saveConfig();
}

void openOptionsMenu() {
    if (!backlight) {
        digitalWrite(TFT_BL, HIGH);
        backlight = true;
        backlightTimer = millis();
    }
    if (inSubMenu != 0) {
        return;
    }
    inSubMenu = true;
    tft.fillRect(0, 0, 240, 320, CUSTOM_DARK);
    int selected = 0;
    int lastSelected = 0;
    int menuSettings[5] = {0, 0, 0, 0, 0};

    drawButtons(optionsButtons);
    optionsMenu::drawOptionsMenu(tft, menuItems, menuSettingsFields, true, selected, lastSelected,
                                 menuSettings);
    Serial.print("In options menu");
    int menuCounter = millis();
    while (true) {
        leftButton.read();
        middleButton.read();
        rightButton.read();

        // leave menu after 60s
        if (millis() - menuCounter > (60 * 1000)) {
            break;
        }

        if (selected != lastSelected) {
            optionsMenu::drawOptionsMenu(tft, menuItems, menuSettingsFields, false, selected, lastSelected,
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
                optionsMenu::drawOptionsMenu(tft, menuItems, menuSettingsFields, false, selected, lastSelected,
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
    lastTVOC = lastCO2PPM = 0;
    drawHeader();
    drawScales();
    drawGraph(currentOptions[0], graphDataSet);
    drawButtons(mainButtons);
}

bool setPowerBoostKeepOn(int en)
{
    Wire.beginTransmission(IP5306_ADDR);
    Wire.write(IP5306_REG_SYS_CTL0);
    if (en)
        Wire.write(0x37); // Set bit1: 1 enable 0 disable boost keep on
    else
        Wire.write(0x35); // 0x37 is default reg value
    return Wire.endTransmission() == 0;
}