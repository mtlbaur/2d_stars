#pragma once

#include "Shape.h"

using namespace std;

struct StarShape : Shape {
    int tips;
    double iRadius;
    double oRadius;
    int styleCore;
    int styleDraw;

    StarShape(int &, double &, double &, int, int);
    void define();
    void fillCore();
    void hollowCore();
    void fillDraw();
    void lineDraw();
};