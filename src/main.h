//
// Created by undef on 9/12/19.
//

// Graphing Stuff
const int DATASET_LENGTH = 22;
const int GRAPH_INTERVAL_HOURS = 8;
const int GRAPH_INTERVAL =  (GRAPH_INTERVAL_HOURS * 60 * 60 * 1000)/DATASET_LENGTH;

int graphPoints[5][DATASET_LENGTH];
unsigned long timePoints[DATASET_LENGTH];

volatile int selectedDataSet = 1;

float scale = 2;
int yMax = 160;
int xOffSet = 280;
int numYLabels = 8;

void calculateScale(int, int);
void drawGraph();
void drawScales();
void onPressed();
void addMeasurement(int, int, unsigned long);
void drawButtons();
void ticker(int,int);
void optionsMenu();

#ifndef DUSTMONITOR_MAIN_H
#define DUSTMONITOR_MAIN_H

#endif //DUSTMONITOR_MAIN_H
