//
// Created by undef on 9/12/19.
//


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

float scale = 2;
int yMax = 160;
int xOffSet = 280;
int numYLabels = 8;

void calculateScale(int, int);
void drawGraph(int, int);
void drawHeader();
void drawScales();
void cycleGraph();
void addMeasurement(int, int, unsigned long, int);
void drawButtons(char[3][16]);
void ticker(int,int);
void openOptionsMenu();
void runSetup();
void cycleRange();
void debug();

unsigned long getDataTimer = 0;
unsigned long graphIntervalTimer[5] = {0, 0, 0, 0,0};
unsigned long uptime = millis();
int lastTemperature = 0;
int lastCO2PPM = 0;
int lastSecond = 0;

bool inSubMenu = false;

// Graphing Stuff
const int DATASET_LENGTH = 22;
/*
 * Graph Points is a 3D set of fields.
 * = {
 * {
 *
 * }
 *
 * }
 */
// graphPoints[interval][type][points]
int graphPoints[5][5][DATASET_LENGTH];
unsigned long timePoints[DATASET_LENGTH];


// All our options represented in numerical form.
// to match the options in openOptionsMenu.cpp:
char mainButtons[3][16] = {"OPTIONS", "GRAPH", "RANGE"};
char optionsButtons[3][16] = {"UP", "ENTER", "DOWN"};
char menuSettingsFields[5][5][16]{
        {"8hrs",    "3hrs",  "1hr",  "30min", "10min"},
        {"3min",   "1min", "30s", "Off"},
        {"On",      "Off"},
        {"English", "Korean"},
        {" "},
};
char functionSettinsFields[5][1][16]{
        {"Disable ABC"},
        {"3min"},
        {"On"},
        {"English"},
        {" "},
};

int optionsMatrix[5][6] = {
        {((8 * 60 * 60 * 1000) / DATASET_LENGTH),
                           ((3 * 60 * 60 * 1000) / DATASET_LENGTH),
                                            ((1 * 60 * 60 * 1000) / DATASET_LENGTH),
                                                         ((30 * 60 * 1000) / DATASET_LENGTH),
                                                          ((10 * 60 * 1000) / DATASET_LENGTH), -1
        },
        {(3 * 60 * 1000), (1 * 60 * 1000), (30 * 1000), (0), -1, -1},
        {0,                1,               -1,          -1,  -1, -1},
        {0,                1,               -1,          -1,  -1, -1},
        {0,                1,               -1,          -1,  -1, -1},
};
int currentOptions[5]{0, 0, 0, 0, 0};

volatile int graphDataSet = 1;

#ifndef DUSTMONITOR_MAIN_H
#define DUSTMONITOR_MAIN_H

#endif //DUSTMONITOR_MAIN_H
