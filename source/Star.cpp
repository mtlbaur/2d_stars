#include "Star.h"

Star::Star(GenType type) {
    switch (type) {
        case RNG: {
            tips = uniform_int_distribution<int>(cfg.min.tips, cfg.max.tips)(dre);
            xVel = uniform_real_distribution<double>(cfg.min.xVel, cfg.max.xVel)(dre) * randomNegative(dre);
            yVel = uniform_real_distribution<double>(cfg.min.yVel, cfg.max.yVel)(dre) * randomNegative(dre);
            angVel = uniform_real_distribution<double>(cfg.min.angVel, cfg.max.angVel)(dre) * randomNegative(dre);
            iRadius = uniform_real_distribution<double>(cfg.min.iRadius, cfg.max.iRadius)(dre);
            oRadius = iRadius + uniform_real_distribution<double>(cfg.min.oRadius, cfg.max.oRadius)(dre);
            density = uniform_real_distribution<double>(cfg.min.density, cfg.max.density)(dre);

            x = uniform_real_distribution<double>(oRadius, win.main.w - 1.0 - oRadius)(dre);
            y = uniform_real_distribution<double>(oRadius, win.main.h - 1.0 - oRadius)(dre);
            ang = uniform_real_distribution<double>(0.0, 360)(dre);

            color = Color(
                uniform_real_distribution<float>(cfg.min.color.r, cfg.max.color.r)(dre),
                uniform_real_distribution<float>(cfg.min.color.g, cfg.max.color.g)(dre),
                uniform_real_distribution<float>(cfg.min.color.b, cfg.max.color.b)(dre),
                uniform_real_distribution<float>(cfg.min.color.a, cfg.max.color.a)(dre));

            break;
        }
        case MIN: {
            tips = cfg.min.tips;
            xVel = cfg.min.xVel;
            yVel = cfg.min.yVel;
            ang = cfg.min.ang;
            angVel = cfg.min.angVel;
            iRadius = cfg.min.iRadius;
            oRadius = iRadius + cfg.min.oRadius;
            density = cfg.min.density;

            x = uniform_real_distribution<double>(oRadius, win.main.w - 1.0 - oRadius)(dre);
            y = uniform_real_distribution<double>(oRadius, win.main.h - 1.0 - oRadius)(dre);

            color = Color(
                cfg.min.color.r,
                cfg.min.color.g,
                cfg.min.color.b,
                cfg.min.color.a);

            break;
        }
        case MAX: {
            tips = cfg.max.tips;
            xVel = cfg.max.xVel * randomNegative(dre);
            yVel = cfg.max.yVel * randomNegative(dre);
            angVel = cfg.max.angVel * randomNegative(dre);
            iRadius = cfg.max.iRadius;
            oRadius = iRadius + cfg.max.oRadius;
            density = cfg.max.density;

            x = uniform_real_distribution<double>(oRadius, win.main.w - 1.0 - oRadius)(dre);
            y = uniform_real_distribution<double>(oRadius, win.main.h - 1.0 - oRadius)(dre);
            ang = cfg.max.ang;

            color = Color(
                cfg.max.color.r,
                cfg.max.color.g,
                cfg.max.color.b,
                cfg.max.color.a);

            break;
        }
        case AVG: {
            tips = (cfg.min.tips + cfg.max.tips) / 2;
            xVel = (cfg.min.xVel + cfg.max.xVel) / 2.0 * randomNegative(dre);
            yVel = (cfg.min.yVel + cfg.max.yVel) / 2.0 * randomNegative(dre);
            angVel = (cfg.min.angVel + cfg.max.angVel) / 2.0 * randomNegative(dre);
            iRadius = (cfg.min.iRadius + cfg.max.iRadius) / 2.0;
            oRadius = iRadius + (cfg.min.oRadius + cfg.max.oRadius) / 2.0;
            density = (cfg.min.density + cfg.max.density) / 2.0;

            x = uniform_real_distribution<double>(oRadius, win.main.w - 1.0 - oRadius)(dre);
            y = uniform_real_distribution<double>(oRadius, win.main.h - 1.0 - oRadius)(dre);
            ang = (cfg.min.ang + cfg.max.ang) / 2.0;

            color = Color(
                (cfg.min.color.r + cfg.max.color.r) / 2.0,
                (cfg.min.color.g + cfg.max.color.g) / 2.0,
                (cfg.min.color.b + cfg.max.color.b) / 2.0,
                (cfg.min.color.a + cfg.max.color.a) / 2.0);

            break;
        }
    }

    if (cfg.minColor > 0.0f && color.r < cfg.minColor && color.g < cfg.minColor && color.b < cfg.minColor) {
        int component = uniform_int_distribution<int>(0, 2)(dre);

        switch (component) {
            case 0: {
                color.r = cfg.minColor;
                break;
            }
            case 1: {
                color.g = cfg.minColor;
                break;
            }
            case 2: {
                color.b = cfg.minColor;
                break;
            }
        }
    }

    indexRandomRGBColors = uniform_int_distribution<unsigned>(0, RGBColors.size() - 1)(dre);
    indexConsistentRGBColors = 0;
    notUpdated = true;
    notCollided = true;

    computeAverageRadius();
    computeArea();
    computeMass();

    vector<int> coreStyle;
    vector<int> drawStyle;

    cfg.style.getCoreStyle(&coreStyle);
    cfg.style.getDrawStyle(&drawStyle);

    shape = StarShape(tips,
                      iRadius,
                      oRadius,
                      coreStyle[uniform_int_distribution<int>(0, coreStyle.size() - 1)(dre)],
                      drawStyle[uniform_int_distribution<int>(0, drawStyle.size() - 1)(dre)]);

    Shader::create(shaderProperties, shape);
}

Star::~Star() {
    Shader::destroy(shaderProperties);
}

void Star::update() {
    if (cfg.gravity) gravityUpdate();

    if (cfg.accel) {
        if (xVel == 0.0 && yVel == 0.0) forceMove();

        double mult = 0.0;

        if (cfg.accelMult > 1.0) mult = 1.0 + cfg.accelMult * frameTime;
        else mult = 1.0 - (1.0 - cfg.accelMult) * frameTime;

        xVel *= mult;
        yVel *= mult;
    }

    if (cfg.minSpeed || cfg.maxSpeed) {
        computeSpeed();

        if (cfg.minSpeed && speed < cfg.minSpeedLimit) {
            if (xVel == 0.0 && yVel == 0.0) forceMove();

            double speedCapMult = cfg.minSpeedLimit / speed;
            xVel *= speedCapMult;
            yVel *= speedCapMult;
        }

        if (cfg.maxSpeed && speed > cfg.maxSpeedLimit) {
            double speedCapMult = cfg.maxSpeedLimit / speed;
            xVel *= speedCapMult;
            yVel *= speedCapMult;
        }
    }

    xUpdate();
    yUpdate();
    angUpdate();

    if (x <= oRadius && xVel < 0.0) reflectLeft();
    else if (x >= (double)(win.main.w - 1) - oRadius && xVel > 0.0) reflectRight();

    if (y <= oRadius && yVel < 0.0) reflectTop();
    else if (y >= (double)(win.main.h - 1) - oRadius && yVel > 0.0) reflectBottom();
}

void Star::draw() {
    Color* c;

    switch (cfg.colorMode) {
        case 0: {
            c = &color;

            break;
        }
        case 1: {
            c = &RGBColors[(int)indexRandomRGBColors];

            indexRandomRGBColors += cfg.colorShiftMult * frameTime;
            clampIndexRGBColors(indexRandomRGBColors, RGBColors.size());

            break;
        }
        case 2: {
            c = &RGBColors[(int)indexUniformRGBColors];

            break;
        }
        case 3: {
            c = &RGBColors[(int)indexConsistentRGBColors];

            indexConsistentRGBColors += cfg.colorShiftMult * frameTime;
            clampIndexRGBColors(indexConsistentRGBColors, RGBColors.size());

            break;
        }
        default: {
            c = &color;
        }
    }

    Shader::activate(shaderProperties);

    glm::mat4 transform = glm::translate(projection, glm::vec3((float)x, (float)y, 0.0f));
    transform = glm::rotate(transform, (float)ang, glm::vec3(0.0f, 0.0f, 1.0f)); // this rotates around the origin (0, 0, 0) along z

    // For the following two lines see: https://docs.gl/gl4/glUniform

    // vertexSource: location == 1: uniform mat4 transform
    glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(transform)); // we know the location is 1 in vertexShader
    // fragmentSource: location == 2: uniform vec4 uniColor
    glUniform4f(2, c->r, c->g, c->b, color.a);

    glDrawElements(shape.args_de.mode, shape.args_de.count, shape.args_de.type, shape.args_de.indices);

    Shader::deactivate(shaderProperties);
}

void Star::computeArea() {
    double slice = PI / tips;
    double apothem = cos(slice) * iRadius;
    double sideLength = sin(slice) * iRadius * 2;
    double perimeter = sideLength * tips;
    double areaOfTheStarPolygonBase = 0.5 * perimeter * apothem;
    double areaOfTheStarTipTriangles = 0.5 * sideLength * oRadius * tips;

    area = areaOfTheStarPolygonBase + areaOfTheStarTipTriangles;
}

void Star::computeAverageRadius() {
    aRadius = (oRadius + iRadius) / 2;
}

void Star::reflectLeft() {
    xVel = -xVel;
    x = max(0.0, oRadius + oRadius - x);
}

void Star::reflectRight() {
    double xMax = (double)(win.main.w - 1) - oRadius;

    xVel = -xVel;
    x = min(xMax, xMax - (xMax - x));
}

void Star::reflectTop() {
    yVel = -yVel;
    y = max(0.0, oRadius + oRadius - y);
}

void Star::reflectBottom() {
    double yMax = (double)(win.main.h - 1) - oRadius;

    yVel = -yVel;
    y = min(yMax, yMax - (yMax - y));
}

/*
This is a circle-based collision response function.

For the purposes of collision handling, each star is treated as a circle where its radius is equal to the average of:
    inner radius: the distance from the star center point to the nearest point that lies on the edge of the star
    outer radius: the distance from the star center point to the furthest point that lies on the edge of the star

This is a compromise to minimize these issues:
    - If we use inner radius as the radius of the colliding circle, then we will have more stars overlapping before colliding (where the overlap is purely visual/graphical).
    - If we use the outer radius, then we will more frequently see stars that, from visual standpoint, do not appear to touch each other but will still collide due to the star's collision circle extending into the empty space between the star's tips.
*/
bool Star::collision(Star* a, Star* b) {
    double distance = pow(a->x - b->x, 2) + pow(a->y - b->y, 2); // distance between center points squared
    double radii = a->aRadius + b->aRadius;                      // sum of the average radii

    if (distance > pow(radii, 2)) return false; // avoid calling sqrt() until after this check for extra performance

    distance = sqrt(distance); // compute the actual distance now that we know there is a collision

    // defined in OverlapCorrection.h
    overlapCorrection(a->x, a->y, b->x, b->y,
                      radii, distance);

    // defined in ElasticCollisionResponse.h
    elasticCollisionResponse(a->x, a->y, b->x, b->y,
                             a->xVel, a->yVel, b->xVel, b->yVel,
                             a->mass, b->mass);

    return true;
}