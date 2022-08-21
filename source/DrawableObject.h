#pragma once

#include <random>

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Config.h"
#include "Shader.h"

using namespace std;

extern struct Config cfg;
extern struct Windows win;
extern double frameTime;
extern default_random_engine dre;

struct DrawableObject {
    double x;
    double y;
    double xVel; // pixels per second
    double yVel;
    double ang;
    double angVel; // radians per second
    double density;

    Color color;
    static vector<Color> RGBColors;

    double area;
    double mass;
    double speed;

    // These are floating-point types to allow smooth, FPS-independent color transitions.
    // They are truncated to int on use.
    double indexRandomRGBColors;
    static double indexUniformRGBColors;
    double indexConsistentRGBColors;

    bool notUpdated;
    bool notCollided;

    ShaderProperties shaderProperties;
    Shape shape;

    static glm::mat4 projection;

    // pure virtual functions must be implemented in the derived class
    virtual ~DrawableObject() = 0;
    virtual void update() = 0;
    virtual void draw() = 0;
    virtual void computeArea() = 0;

    void computeMass();
    void computeSpeed();
    void xUpdate();
    void yUpdate();
    void angUpdate();
    void gravityUpdate();
    void forceMove();

    static void updateIndexUniformRGBColors();
    static void prepareProjection(int, int);
    static int randomNegative(default_random_engine &);
    static void clampIndexRGBColors(double &, int);
};