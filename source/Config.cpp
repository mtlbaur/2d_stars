#include "config.h"

Color::Color(float r, float g, float b, float a)
    : r(r), g(g), b(b), a(a) {}

void Color::assign(float r, float g, float b, float a) {
    this->r = r;
    this->g = g;
    this->b = b;
    this->a = a;
}

// example of what this looks like: https://en.wikipedia.org/wiki/File:RBG_color_wheel.svg
std::vector<Color> Color::getRGBColors() {
    std::vector<Color> colors;
    float alpha = 1.0f, step = 0.001f;

    // red -> yellow
    for (float i = 0.0f; i <= 1.0f; i += step) {
        colors.emplace_back(Color(1.0f, i, 0.0f, alpha));
    }
    // yellow -> green
    for (float i = 1.0f; i >= 0.0f; i -= step) {
        colors.emplace_back(Color(i, 1.0f, 0.0f, alpha));
    }
    // green -> cyan
    for (float i = 0.0f; i <= 1.0f; i += step) {
        colors.emplace_back(Color(0.0f, 1.0f, i, alpha));
    }
    // cyan -> blue
    for (float i = 1.0f; i >= 0.0f; i -= step) {
        colors.emplace_back(Color(0.0f, i, 1.0f, alpha));
    }
    // blue -> magenta
    for (float i = 0.0f; i <= 1.0f; i += step) {
        colors.emplace_back(Color(i, 0.0f, 1.0f, alpha));
    }
    // magenta -> red
    for (float i = 1.0f; i >= 0.0f; i -= step) {
        colors.emplace_back(Color(1.0f, 0.0f, i, alpha));
    }

    return colors;
}

template <class Archive>
void Color::serialize(Archive& a, const unsigned v) {
    a& r;
    a& g;
    a& b;
    a& this->a;
}

template <class Archive>
void StarProperties::serialize(Archive& a, const unsigned v) {
    a& tips;
    a& xVel;
    a& yVel;
    a& ang;
    a& angVel;
    a& iRadius;
    a& oRadius;
    a& density;

    a& color;
}

template <class Archive>
void StarStyle::serialize(Archive& a, const unsigned v) {
    a& core.full;
    a& core.empty;
    a& draw.fill;
    a& draw.line;
}

void Files::load(const std::string& path, const std::string& extension) {
    list.clear();
    names.clear();

    try {
        boost::filesystem::path p(path);

        using namespace boost::filesystem;

        if (exists(p) && is_directory(p)) {
            for (directory_entry& f : directory_iterator(p)) {
                if (f.path().extension() == extension) {
                    list.emplace_back(f);
                    names.emplace_back(f.path().stem().string());
                }
            }
        }

    } catch (std::exception& e) {
        std::cerr << "EXCEPTION: Files::load(): " << e.what() << '\n';
    }
}

Config::Config() {
    limMin.tips    = 3;
    limMin.xVel    = 0.0f;
    limMin.yVel    = 0.0f;
    limMin.ang     = 0.0f;
    limMin.angVel  = 0.0f;
    limMin.iRadius = 1.0f;
    limMin.oRadius = 0.0f;
    limMin.density = 1.0f;
    limMin.color.r = 0.0f;
    limMin.color.g = 0.0f;
    limMin.color.b = 0.0f;
    limMin.color.a = 0.0f;

    limMax.tips    = 100;
    limMax.xVel    = 1000.0f;
    limMax.yVel    = 1000.0f;
    limMax.ang     = Constants::TWO_PI;
    limMax.angVel  = Constants::TWO_PI;
    limMax.iRadius = 100.0f;
    limMax.oRadius = 100.0f;
    limMax.density = 100.0f;
    limMax.color.r = 1.0f;
    limMax.color.g = 1.0f;
    limMax.color.b = 1.0f;
    limMax.color.a = 1.0f;

    min.tips    = 3;
    min.xVel    = 0.0f;
    min.yVel    = 0.0f;
    min.ang     = 0.0f;
    min.angVel  = Constants::PI / 10.0f;
    min.iRadius = 4.0f;
    min.oRadius = 0.0f;
    min.density = 1.0f;
    min.color.r = 0.0f;
    min.color.g = 0.0f;
    min.color.b = 0.0f;
    min.color.a = 1.0f / 3.0f;

    max.tips    = 12;
    max.xVel    = 100.0f;
    max.yVel    = 100.0f;
    max.ang     = Constants::TWO_PI;
    max.angVel  = Constants::TWO_PI;
    max.iRadius = 12.0f;
    max.oRadius = 12.0f;
    max.density = 1.0f;
    max.color.r = 1.0f;
    max.color.g = 1.0f;
    max.color.b = 1.0f;
    max.color.a = 1.0f;
}

template <class Archive>
void Config::serialize(Archive& a, const unsigned v) {
    a& colorMode;
    a& srcBlendMode;
    a& dstBlendMode;
    a& addRemove;
    a& targetFPS;

    a& show;
    a& clear;
    a& collisions;
    a& gravity;
    a& minSpeed;
    a& maxSpeed;
    a& accel;
    a& showGenerationParametersWindow;
    a& forceVisiblePreview;

    a& gravityVal;
    a& colorShiftMult;
    a& minSpeedLimit;
    a& maxSpeedLimit;
    a& accelMult;
    a& minColor;

    a& backgroundColor;

    a& min;
    a& max;

    a& style;
}

void Config::save(const std::string& path, const std::string& name, const std::string& extension) {
    try {
        if (name.empty()) return;
        if (!boost::filesystem::exists(path)) boost::filesystem::create_directories(path);

        std::string fullPath = path + name + extension;
        std::ofstream ofs(fullPath, std::ofstream::out);

        if (ofs.fail()) return;

        boost::archive::text_oarchive oa(ofs);
        oa << *this;
        ofs.close();

        files.last = fullPath;
    } catch (std::exception& e) {
        std::cerr << "EXCEPTION: Config::save(): " << e.what() << '\n';
        reset();
    }
}

void Config::load(const std::string& path, const std::string& name, const std::string& extension) {
    try {
        if (name.empty()) return;

        std::string fullPath = path + name + extension;
        std::ifstream ifs(fullPath, std::ifstream::in);

        if (ifs.fail()) return;

        boost::archive::text_iarchive ia(ifs);
        ia >> *this;
        ifs.close();

        files.last = fullPath;
    } catch (std::exception& e) {
        std::cerr << "EXCEPTION: Config::load(): " << e.what() << '\n';
        reset();
    }
}

void Config::reset() {
    *this            = Config();
    this->files.last = "RESET";
}