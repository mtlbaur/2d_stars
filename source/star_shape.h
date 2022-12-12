#ifndef STAR_SHAPE_H
#define STAR_SHAPE_H

#include "rng.h"
#include "config.h"
#include "constants.h"
#include "enums.h"
#include "shape.h"

extern struct RNG rng;
extern struct Config cfg;

struct StarShape : Shape {
    using Core = Enum::Star::Shape::Core;
    using Draw = Enum::Star::Shape::Draw;

    int tips;
    double iRadius;
    double oRadius;

    struct Style {
        Core core;
        Draw draw;
    } style;

    StarShape(int&, double&, double&);

    void fullCore();
    void emptyCore();
    void fillDraw();
    void lineDraw();
};

#endif