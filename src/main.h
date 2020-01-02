/*
 * Copyright Wilyarti Howard - 2019
 */


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

// BLE
bool deviceConnected = false;
bool oldDeviceConnected = false;

uint16_t BLE_MTU = 512;
char BLE_SERVICE_ID[37] = "db101875-d9c4-4c10-b856-fad3a581a6ea";
char BLE_TEMP_CHARACTERISTIC[37] = "06576524-99f9-4dc5-b6ea-c66dc433e6f2";
char BLE_HUMIDITY_CHARACTERISTIC[37] = "4e1fb0da-dc91-43ea-9b6d-77f699ddbbed";
char BLE_GRAPH_CHARACTERISTIC[37] = "900dd909-eb3a-4774-bcdb-b10d8dd2ae28";


// Menu items
// All our options represented in numerical form.
// to match the options in openOptionsMenu.cpp:
char menuSettingsFields[5][5][16] = {
        {"8hrs",    "3hrs", "1hr", "30min", "10min"},
        {"3min",    "1min", "30s", "Off"},
        {"On",      "Off"},
        {"English", "Korean"},
        {" "},
};
char menuItems[5][16] = {"Graph Range", "Warm Up", "WiFi Mode", "Language", "Exit"};
char mainButtons[3][16] = {"OPTIONS", "GRAPH", "RANGE"};
char optionsButtons[3][16] = {"UP", "ENTER", "DOWN"};

// Variables
unsigned long getDataTimer = 0;
unsigned long bleTimer = 0;
unsigned long bleGraphTimer = 0;

unsigned long graphIntervalTimer[5] = {0, 0, 0, 0, 0};
unsigned long uptime = 0;
int lastTemperature = 0;
int lastCO2PPM = 0;
int lastSecond = 0;

volatile bool inSubMenu = false;

// Graphing Stuff
const int DATASET_LENGTH = 22;
const int BLE_DATASET_ROWS = 2;
const int BLE_DATASETLENGTH = 96;

// BLE mobile graph
int bleGraphInterval = 1000; //(24 * 60 *60 * 1000)/ BLE_DATASETLENGTH;
unsigned long bleGraphDatasetTimer = 0;
// 0 = Temp
// 1 = CO2
float bleGraphPoints[BLE_DATASET_ROWS][BLE_DATASETLENGTH]; // 1 data point every minute for 24 hours
unsigned long bleTimePoints[BLE_DATASETLENGTH];

// graphPoints[interval][type][points]
int graphPoints[5][5][DATASET_LENGTH];
unsigned long timePoints[DATASET_LENGTH];
int optionsMatrix[5][6] = {
        {((8 * 60 * 60 * 1000) / DATASET_LENGTH),
                          ((3 * 60 * 60 * 1000) / DATASET_LENGTH),
                                           ((1 * 60 * 60 * 1000) / DATASET_LENGTH),
                                                        ((30 * 60 * 1000) / DATASET_LENGTH),
                                                             ((10 * 60 * 1000) / DATASET_LENGTH), -1
        },
        {(3 * 60 * 1000), (1 * 60 * 1000), (30 * 1000), (0), -1,                                  -1},
        {0,               1,               -1,          -1,  -1,                                  -1},
        {0,               1,               -1,          -1,  -1,                                  -1},
        {0,               1,               -1,          -1,  -1,                                  -1},
};

// Volatile
volatile int currentOptions[5]{0, 0, 0, 0, 0};
volatile int graphDataSet = 0;

// Graphing variables
float scale = 2;
int yMax = 160;
int xOffSet = 280;
int numYLabels = 8;

// Functions
void calculateScale(int, int);

void drawGraph(int, int);

void drawHeader();

void drawScales();

void cycleGraph();

void addMeasurement(int, int, unsigned long, int);

void drawButtons(char[3][16]);

void ticker(int, int);

void openOptionsMenu();

void runSetup();

void cycleRange();

void debug();

void initBle();

void addBleGraphMeasurement(float, float, unsigned long);

#ifndef DUSTMONITOR_MAIN_H
#define DUSTMONITOR_MAIN_H

#endif //DUSTMONITOR_MAIN_H
