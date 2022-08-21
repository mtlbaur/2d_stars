#pragma once

// #include <iostream>
// #include <iomanip>

#include "DrawableObject.h"
#include "StarShape.h"
#include "OverlapCorrection.h"
#include "ElasticCollisionResponse.h"

using namespace std;

enum GenType {
    RNG = 0,
    MIN = 1,
    AVG = 2,
    MAX = 3
};

struct Star : DrawableObject {
    int tips;
    double iRadius;
    double oRadius;
    double aRadius;

    Star(GenType);
    ~Star();

    void update();
    void draw();
    void computeArea();
    void computeAverageRadius();
    void reflectLeft();
    void reflectRight();
    void reflectTop();
    void reflectBottom();

    static bool collision(Star*, Star*);
};