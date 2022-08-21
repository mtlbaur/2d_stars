#include "DrawableObject.h"

// must define an implementation for a pure virtual destructor even though this is an abstract struct (otherwise the program can't be linked)
DrawableObject::~DrawableObject(){};

void DrawableObject::computeMass() {
    mass = density * area; // area not volume because we are in a 2D plane
}

void DrawableObject::computeSpeed() {
    speed = sqrt(pow(xVel, 2) + pow(yVel, 2));
}

void DrawableObject::xUpdate() {
    x += xVel * frameTime;
}

void DrawableObject::yUpdate() {
    y += yVel * frameTime;
}

void DrawableObject::angUpdate() {
    ang = fmod((ang + angVel * frameTime), TWO_PI);
}

void DrawableObject::gravityUpdate() {
    yVel += cfg.gravityMult * frameTime;
}

void DrawableObject::forceMove() {
    xVel += uniform_real_distribution<double>(1, 1000)(dre) * frameTime * randomNegative(dre);
    yVel += uniform_real_distribution<double>(1, 1000)(dre) * frameTime * randomNegative(dre);
}

void DrawableObject::updateIndexUniformRGBColors() {
    DrawableObject::indexUniformRGBColors += cfg.colorShiftMult * frameTime;
    clampIndexRGBColors(DrawableObject::indexUniformRGBColors, RGBColors.size());
}

void DrawableObject::prepareProjection(int width, int height) {
    projection = glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 1.0f, 0.0f)); // before ortho -> NDC

    projection *= glm::ortho(-width / 2.0f,   // left
                             width / 2.0f,    // right
                             height / 2.0f,   // bottom
                             -height / 2.0f); // top
}

int DrawableObject::randomNegative(default_random_engine &dre) {
    if (uniform_int_distribution<int>(0, 1)(dre) == 1) return 1;
    else return -1;
}

void DrawableObject::clampIndexRGBColors(double &index, int size) {
    if (index >= size) index -= size;
    else if (index < 0.0) index += size;
}

// must define static class data members in a .cpp file before main otherwise linking fails
vector<Color> DrawableObject::RGBColors = Color::getRGBColors();
double DrawableObject::indexUniformRGBColors = 0.0;
glm::mat4 DrawableObject::projection;