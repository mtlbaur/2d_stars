#include <iostream>

#include "star.h"

// must define static class data members in a .cpp file before main otherwise linking fails
std::vector<Color> Star::RGBColors = Color::getRGBColors();
glm::mat4 Star::projection;
double Star::indexUniformRGBColors = 0.0;

Star::Star(Enum::Star::GenType type) {
    {
        using enum Enum::Star::GenType;

        switch (type) {
            case RNG: {
                tips    = rng.I(cfg.min.tips, cfg.max.tips);
                xVel    = rng.DN(cfg.min.xVel, cfg.max.xVel);
                yVel    = rng.DN(cfg.min.yVel, cfg.max.yVel);
                angVel  = rng.DN(cfg.min.angVel, cfg.max.angVel);
                iRadius = rng.D(cfg.min.iRadius, cfg.max.iRadius);
                oRadius = iRadius + rng.D(cfg.min.oRadius, cfg.max.oRadius);
                density = rng.D(cfg.min.density, cfg.max.density);

                x   = rng.D(oRadius, win.main.w - 1.0 - oRadius);
                y   = rng.D(oRadius, win.main.h - 1.0 - oRadius);
                ang = rng.D(0.0, 360.0);

                color = std::make_unique<Color>(
                    rng.F(cfg.min.color.r, cfg.max.color.r),
                    rng.F(cfg.min.color.g, cfg.max.color.g),
                    rng.F(cfg.min.color.b, cfg.max.color.b),
                    rng.F(cfg.min.color.a, cfg.max.color.a));

                break;
            }
            case MIN: {
                tips    = cfg.min.tips;
                xVel    = cfg.min.xVel;
                yVel    = cfg.min.yVel;
                ang     = cfg.min.ang;
                angVel  = cfg.min.angVel;
                iRadius = cfg.min.iRadius;
                oRadius = iRadius + cfg.min.oRadius;
                density = cfg.min.density;

                x   = rng.D(oRadius, win.main.w - 1.0 - oRadius);
                y   = rng.D(oRadius, win.main.h - 1.0 - oRadius);
                ang = cfg.min.ang;

                color = std::make_unique<Color>(
                    cfg.min.color.r,
                    cfg.min.color.g,
                    cfg.min.color.b,
                    cfg.min.color.a);

                break;
            }
            case MAX: {
                tips    = cfg.max.tips;
                xVel    = rng.N(cfg.max.xVel);
                yVel    = rng.N(cfg.max.yVel);
                angVel  = rng.N(cfg.max.angVel);
                iRadius = cfg.max.iRadius;
                oRadius = iRadius + cfg.max.oRadius;
                density = cfg.max.density;

                x   = rng.D(oRadius, win.main.w - 1.0 - oRadius);
                y   = rng.D(oRadius, win.main.h - 1.0 - oRadius);
                ang = cfg.max.ang;

                color = std::make_unique<Color>(
                    cfg.max.color.r,
                    cfg.max.color.g,
                    cfg.max.color.b,
                    cfg.max.color.a);

                break;
            }
            case AVG: {
                tips    = (cfg.min.tips + cfg.max.tips) / 2;
                xVel    = rng.N((cfg.min.xVel + cfg.max.xVel) / 2.0);
                yVel    = rng.N((cfg.min.yVel + cfg.max.yVel) / 2.0);
                angVel  = rng.N((cfg.min.angVel + cfg.max.angVel) / 2.0);
                iRadius = (cfg.min.iRadius + cfg.max.iRadius) / 2.0;
                oRadius = iRadius + (cfg.min.oRadius + cfg.max.oRadius) / 2.0;
                density = (cfg.min.density + cfg.max.density) / 2.0;

                x   = rng.D(oRadius, win.main.w - 1.0 - oRadius);
                y   = rng.D(oRadius, win.main.h - 1.0 - oRadius);
                ang = (cfg.min.ang + cfg.max.ang) / 2.0;

                color = std::make_unique<Color>(
                    (cfg.min.color.r + cfg.max.color.r) / 2.0,
                    (cfg.min.color.g + cfg.max.color.g) / 2.0,
                    (cfg.min.color.b + cfg.max.color.b) / 2.0,
                    (cfg.min.color.a + cfg.max.color.a) / 2.0);

                break;
            }
        }
    }

    if (cfg.minColor > 0.0f && color->r < cfg.minColor && color->g < cfg.minColor && color->b < cfg.minColor) {
        float c = rng.F(cfg.minColor, 1.0f);

        switch (rng.I(0, 2)) {
            case 0:
                color->r = c;
                break;
            case 1:
                color->g = c;
                break;
            case 2:
                color->b = c;
                break;
        }
    }

    indexRandomRGBColors     = rng.D(0, RGBColors.size() - 1);
    indexConsistentRGBColors = 0;
    notCollided              = true;

    computeAverageRadius();
    computeArea();
    computeMass();

    shape  = std::make_unique<StarShape>(tips, iRadius, oRadius);
    shader = std::make_unique<Shader>(*shape);
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

inline void Star::xUpdate() {
    x += xVel * frameTime;
}

inline void Star::yUpdate() {
    y += yVel * frameTime;
}

inline void Star::angUpdate() {
    ang = fmod(ang + angVel * frameTime, Constants::TWO_PI);
}

inline void Star::gravityUpdate() {
    yVel += cfg.gravityVal * 100 * frameTime;
}

void Star::draw() {
    Color* c;

    switch (cfg.colorMode) {
        using enum Enum::Config::ColorMode;

        case DEFAULT: {
            c = color.get();
            break;
        }
        case RANDOM: {
            c = &RGBColors[(int)indexRandomRGBColors];
            indexRandomRGBColors += cfg.colorShiftMult * frameTime;
            clampIndexRGBColors(indexRandomRGBColors, RGBColors.size());
            break;
        }
        case UNIFORM: {
            c = &RGBColors[(int)indexUniformRGBColors];
            break;
        }
        case CONSISTENT: {
            c = &RGBColors[(int)indexConsistentRGBColors];
            indexConsistentRGBColors += cfg.colorShiftMult * frameTime;
            clampIndexRGBColors(indexConsistentRGBColors, RGBColors.size());
            break;
        }
        default: {
            c = color.get();
            break;
        }
    }

    shader->activate();

    glm::mat4 transform = glm::translate(projection, glm::vec3((float)x, (float)y, 0.0f));
    transform           = glm::rotate(transform, (float)ang, glm::vec3(0.0f, 0.0f, 1.0f)); // this rotates around the origin (0, 0, 0) along z

    // For the following two lines see: https://docs.gl/gl4/glUniform

    // vertexSource: location == 1: uniform mat4 transform
    glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(transform)); // we know the location is 1 in vertexShader
    // fragmentSource: location == 2: uniform vec4 uniColor
    glUniform4f(2, c->r, c->g, c->b, color->a);

    glDrawElements(shape->args_de.mode, shape->args_de.count, shape->args_de.type, shape->args_de.indices);

    Shader::deactivate();
}

inline void Star::forceMove() {
    xVel += rng.DN(1, 1000) * frameTime;
    yVel += rng.DN(1, 1000) * frameTime;
}

inline void Star::computeArea() {
    double slice                     = Constants::PI / tips;
    double apothem                   = cos(slice) * iRadius;
    double sideLength                = sin(slice) * iRadius * 2;
    double perimeter                 = sideLength * tips;
    double areaOfTheStarPolygonBase  = 0.5 * perimeter * apothem;
    double areaOfTheStarTipTriangles = 0.5 * sideLength * oRadius * tips;

    area = areaOfTheStarPolygonBase + areaOfTheStarTipTriangles;
}

inline void Star::computeAverageRadius() {
    aRadius = (oRadius + iRadius) / 2;
}

inline void Star::computeMass() {
    mass = density * area; // area not volume because we are in a 2D plane
}

inline void Star::computeSpeed() {
    speed = sqrt(pow(xVel, 2) + pow(yVel, 2));
}

inline void Star::reflectLeft() {
    xVel = -xVel;
    x    = std::max(0.0, oRadius + oRadius - x);
}

inline void Star::reflectRight() {
    double xMax = static_cast<double>(win.main.w - 1) - oRadius;

    xVel = -xVel;
    x    = std::min(xMax, xMax - (xMax - x));
}

inline void Star::reflectTop() {
    yVel = -yVel;
    y    = std::max(0.0, oRadius + oRadius - y);
}

inline void Star::reflectBottom() {
    double yMax = static_cast<double>(win.main.h - 1) - oRadius;

    yVel = -yVel;
    y    = std::min(yMax, yMax - (yMax - y));
}

void Star::prepareProjection(int width, int height) {
    projection = glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 1.0f, 0.0f)); // before ortho -> NDC

    projection *= glm::ortho(-width / 2.0f,   // left
                             width / 2.0f,    // right
                             height / 2.0f,   // bottom
                             -height / 2.0f); // top
}

void Star::updateIndexUniformRGBColors() {
    Star::indexUniformRGBColors += cfg.colorShiftMult * frameTime;
    clampIndexRGBColors(Star::indexUniformRGBColors, RGBColors.size());
}

void Star::clampIndexRGBColors(double& index, int size) {
    if (index >= size) index -= size;
    else if (index < 0.0) index += size;
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
bool Star::collision(Star& a, Star& b) {
    double distance = pow(a.x - b.x, 2.0) + pow(a.y - b.y, 2.0); // distance between center points squared

    if (distance > pow(a.aRadius + b.aRadius, 2.0)) return false; // avoid calling sqrt() until after this check for extra performance

    distance = sqrt(distance); // compute the actual distance now that we know there is a collision

    // defined in overlap_correction.h
    overlapCorrection(a.x, a.y, b.x, b.y,
                      a.aRadius + b.aRadius, distance);

    // defined in elastic_collision_response.h
    elasticCollisionResponse(a.x, a.y, b.x, b.y,
                             a.xVel, a.yVel, b.xVel, b.yVel,
                             a.mass, b.mass);

    return true;
}