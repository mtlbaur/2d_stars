#ifndef STAR_H_GUARD
#define STAR_H_GUARD

#include <random>
#include <memory>

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "rng.h"
#include "constants.h"
#include "enums.h"
#include "window.h"
#include "config.h"
#include "shader.h"
#include "star_shape.h"
#include "overlap_correction.h"
#include "elastic_collision_response.h"

extern struct RNG rng;
extern struct Config cfg;
extern struct Windows win;
extern double frameTime;

struct Star {
    double x;
    double y;
    double xVel; // pixels per second
    double yVel;
    double ang;
    double angVel; // radians per second
    double density;

    int tips;
    double iRadius;
    double oRadius;
    double aRadius;

    double area;
    double mass;
    double speed;

    // These are floating-point types to allow smooth, FPS-independent color transitions.
    // They are truncated to int on use.
    double indexRandomRGBColors = 0.0;
    static double indexUniformRGBColors;
    double indexConsistentRGBColors = 0.0;

    bool notCollided = true;

    std::unique_ptr<Color> color;
    std::unique_ptr<Shader> shader;
    std::unique_ptr<Shape> shape;

    static std::vector<Color> RGBColors;
    static glm::mat4 projection;

    Star(Enum::Star::GenType);

    void update();
    void xUpdate();
    void yUpdate();
    void angUpdate();
    void gravityUpdate();

    void draw();
    void forceMove();

    void computeMass();
    void computeArea();
    void computeSpeed();
    void computeAverageRadius();

    void reflectLeft();
    void reflectRight();
    void reflectTop();
    void reflectBottom();

    static void prepareProjection(int, int);

    static void updateIndexUniformRGBColors();
    static void clampIndexRGBColors(double&, int);

    static bool collision(Star&, Star&);
};

#endif