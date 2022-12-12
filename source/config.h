#ifndef CONFIG_H
#define CONFIG_H

#include <fstream>
#include <iostream>
#include <vector>

// https://www.boost.org/doc/libs/1_79_0/libs/filesystem/doc/tutorial.html
#include <boost/filesystem.hpp>

// https://www.boost.org/doc/libs/1_79_0/libs/serialization/doc/index.html
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include "constants.h"
#include "enums.h"

struct Color {
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 1.0f;

    Color() = default;
    Color(float, float, float, float);
    void assign(float, float, float, float);

    static std::vector<Color> getRGBColors();

    template <class Archive>
    void serialize(Archive&, const unsigned);
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
    void serialize(Archive&, const unsigned);
};

struct StarStyle {
    struct Core {
        bool full  = true;
        bool empty = true;
    } core;

    struct Draw {
        bool fill = true;
        bool line = true;
    } draw;

    template <class Archive>
    void serialize(Archive&, const unsigned);
};

struct Files {
    // config file list
    std::vector<boost::filesystem::directory_entry> list;
    // config file names
    std::vector<std::string> names;
    // last used config file name
    std::string last;

    void load(const std::string&, const std::string&);
};

struct Config {
    using ColorMode = Enum::Config::ColorMode;

    // DEFAULT: each star will have the color it was created with
    // RANDOM: each star will have a random color based on an index position in the rainbowColors vector
    // UNIFORM: each star will share the same color index in the rainbowColors vector (they will all have the same color)
    // CONSISTENT: batches of stars that spawn at the same time will have the same color
    int colorMode = ColorMode::DEFAULT;

    int srcBlendMode = 6;
    int dstBlendMode = 7;
    int addRemove    = 25;
    int targetFPS    = 500;

    bool show                           = true;
    bool clear                          = true;
    bool collisions                     = true;
    bool gravity                        = false;
    bool minSpeed                       = false;
    bool maxSpeed                       = false;
    bool accel                          = false;
    bool showGenerationParametersWindow = false;
    bool forceVisiblePreview            = false;
    bool menuSave                       = false;
    bool menuLoad                       = false;

    float gravityVal     = Constants::GRAVITY;
    float colorShiftMult = 10;
    float minSpeedLimit  = 0;
    float maxSpeedLimit  = 1000;
    float accelMult      = 1;
    float minColor       = 0.0f;

    Color backgroundColor;

    StarProperties limMin;
    StarProperties limMax;
    StarProperties min;
    StarProperties max;

    StarStyle style;

    Files files;

    Config();

    template <class Archive>
    void serialize(Archive&, const unsigned);
    void save(const std::string&, const std::string&, const std::string&);
    void load(const std::string&, const std::string&, const std::string&);
    void reset();
};

#endif