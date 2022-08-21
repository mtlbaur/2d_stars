#pragma once

#include <fstream>
#include <iostream>

// https://www.boost.org/doc/libs/1_79_0/libs/filesystem/doc/tutorial.html
#include <boost/filesystem.hpp>

// https://www.boost.org/doc/libs/1_79_0/libs/serialization/doc/index.html
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "MathConstants.h"

using namespace std;

struct Window {
    GLFWwindow* glfw = NULL;
    ImGuiContext* imgui = NULL;
    ImGuiIO* io = NULL;

    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;

    bool exists = false;

    unsigned int frameBuffer = 0;
    unsigned int texture = 0;
};

struct Windows {
    Window main;
    Window cfg;
};

struct Color {
    float r;
    float g;
    float b;
    float a;

    Color();
    Color(float, float, float, float);

    static vector<Color> getRGBColors();

    // friend class boost::serialization::access; // unnecessary because this is a struct: everything is public
    template <class Archive>
    void serialize(Archive &, const unsigned int);
};

struct StarProperties {
    int tips;
    float xVel;
    float yVel;
    float ang;
    float angVel;
    float iRadius;
    float oRadius;
    float density;
    Color color;

    template <class Archive>
    void serialize(Archive &, const unsigned int);
};

struct StarStyle {
    bool fillCore = true;
    bool hollowCore = true;
    bool fillDraw = true;
    bool lineDraw = true;

    void getCoreStyle(vector<int>*);
    void getDrawStyle(vector<int>*);

    template <class Archive>
    void serialize(Archive &, const unsigned int);
};

struct Config {
    /*
    0: default: each star will have the color it was created with
    1: random: each star will have a random color based on an index position in the rainbowColors vector
    2: uniform: each star will share the same color index in the rainbowColors vector (they will all have the same color)
    3: consistent: batches of stars that spawn at the same time will have the same color
    */
    int colorMode = 0;
    int srcBlendMode = 6;
    int dstBlendMode = 7;
    int addRemove = 10;
    int targetFPS = 500;

    bool show = true;
    bool clear = true;
    bool collisions = true;
    bool gravity = false;
    bool minSpeed = false;
    bool maxSpeed = false;
    bool accel = false;
    bool showGenerationParametersWindow = false;
    bool forceVisiblePreview = false;
    bool menuSave = false;
    bool menuLoad = false;

    float gravityMult = 1000;
    float colorShiftMult = 10;
    float minSpeedLimit = 0;
    float maxSpeedLimit = 1000;
    float accelMult = 1;
    float minColor = 0.0f;

    Color backgroundColor;

    StarProperties limMin;
    StarProperties limMax;
    StarProperties min;
    StarProperties max;

    StarStyle style;

    Config();

    template <class Archive>
    void serialize(Archive &, const unsigned int);
    void save(string, string, string, string* = NULL);
    void load(string, string, string, string* = NULL);
    void reset();
};