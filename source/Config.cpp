#include "Config.h"

Color::Color() {
    r = 0.0f;
    g = 0.0f;
    b = 0.0f;
    a = 1.0f;
}

Color::Color(float r, float g, float b, float a) {
    this->r = r;
    this->g = g;
    this->b = b;
    this->a = a;
}

// visual example of what this looks like: https://en.wikipedia.org/wiki/File:RBG_color_wheel.svg
vector<Color> Color::getRGBColors() {
    vector<Color> colors;
    float alpha = 1.0f, step = 0.001f;

    // red -> yellow
    for (float i = 0.0f; i <= 1.0f; i += step) {
        colors.push_back(Color(1.0f, i, 0.0f, alpha));
    }
    // yellow -> green
    for (float i = 1.0f; i >= 0.0f; i -= step) {
        colors.push_back(Color(i, 1.0f, 0.0f, alpha));
    }
    // green -> cyan
    for (float i = 0.0f; i <= 1.0f; i += step) {
        colors.push_back(Color(0.0f, 1.0f, i, alpha));
    }
    // cyan -> blue
    for (float i = 1.0f; i >= 0.0f; i -= step) {
        colors.push_back(Color(0.0f, i, 1.0f, alpha));
    }
    // blue -> magenta
    for (float i = 0.0f; i <= 1.0f; i += step) {
        colors.push_back(Color(i, 0.0f, 1.0f, alpha));
    }
    // magenta -> red
    for (float i = 1.0f; i >= 0.0f; i -= step) {
        colors.push_back(Color(1.0f, 0.0f, i, alpha));
    }

    return colors;
}

template <class Archive>
void Color::serialize(Archive &a, const unsigned int v) {
    a &r;
    a &g;
    a &b;
    a &this->a;
}

template <class Archive>
void StarProperties::serialize(Archive &a, const unsigned int v) {
    a &tips;
    a &xVel;
    a &yVel;
    a &ang;
    a &angVel;
    a &iRadius;
    a &oRadius;
    a &density;

    a &color;
}

void StarStyle::getCoreStyle(vector<int>* s) {
    if (fillCore) s->push_back(0);
    if (hollowCore) s->push_back(1);

    if (s->size() == 0) {
        s->push_back(0);
        s->push_back(1);
    }
}

void StarStyle::getDrawStyle(vector<int>* s) {
    if (fillDraw) s->push_back(0);
    if (lineDraw) s->push_back(1);

    if (s->size() == 0) {
        s->push_back(0);
        s->push_back(1);
    }
}

template <class Archive>
void StarStyle::serialize(Archive &a, const unsigned int v) {
    a &fillCore;
    a &hollowCore;
    a &fillDraw;
    a &lineDraw;
}

Config::Config() {
    limMin.tips = 3;
    limMin.xVel = 0.0f;
    limMin.yVel = 0.0f;
    limMin.ang = 0.0f;
    limMin.angVel = 0.0f;
    limMin.iRadius = 4.0f;
    limMin.oRadius = 0.0f;
    limMin.density = 1.0f;
    limMin.color.r = 0.0f;
    limMin.color.g = 0.0f;
    limMin.color.b = 0.0f;
    limMin.color.a = 0.0f;

    limMax.tips = 100;
    limMax.xVel = 1000.0f;
    limMax.yVel = 1000.0f;
    limMax.ang = TWO_PI;
    limMax.angVel = TWO_PI;
    limMax.iRadius = 100.0f;
    limMax.oRadius = 100.0f;
    limMax.density = 100.0f;
    limMax.color.r = 1.0f;
    limMax.color.g = 1.0f;
    limMax.color.b = 1.0f;
    limMax.color.a = 1.0f;

    min.tips = 3;
    min.xVel = 0.0f;
    min.yVel = 0.0f;
    min.ang = 0.0f;
    min.angVel = PI / 10.0f;
    min.iRadius = 4.0f;
    min.oRadius = 0.0f;
    min.density = 1.0f;
    min.color.r = 0.0f;
    min.color.g = 0.0f;
    min.color.b = 0.0f;
    min.color.a = 1.0f / 3.0f;

    max.tips = 12;
    max.xVel = 100.0f;
    max.yVel = 100.0f;
    max.ang = TWO_PI;
    max.angVel = TWO_PI;
    max.iRadius = 12.0f;
    max.oRadius = 12.0f;
    max.density = 1.0f;
    max.color.r = 1.0f;
    max.color.g = 1.0f;
    max.color.b = 1.0f;
    max.color.a = 1.0f;
}

template <class Archive>
void Config::serialize(Archive &a, const unsigned int v) {
    a &colorMode;
    a &srcBlendMode;
    a &dstBlendMode;
    a &addRemove;
    a &targetFPS;

    a &show;
    a &clear;
    a &collisions;
    a &gravity;
    a &minSpeed;
    a &maxSpeed;
    a &accel;
    a &showGenerationParametersWindow;
    a &forceVisiblePreview;

    a &gravityMult;
    a &colorShiftMult;
    a &minSpeedLimit;
    a &maxSpeedLimit;
    a &accelMult;
    a &minColor;

    a &backgroundColor;

    a &min;
    a &max;

    a &style;
}

void Config::save(string path, string name, string extension, string* last) {
    try {
        if (name == "") return;

        // cout << "SAVE: " << path + name + extension << endl;

        if (!boost::filesystem::exists(path)) boost::filesystem::create_directories(path);

        ofstream ofs(path + name + extension, ofstream::out);
        boost::archive::text_oarchive oa(ofs);
        oa << *this;
        ofs.close();

        if (last) *last = path + name + extension;
    } catch (exception &e) {
        fprintf(stderr, "EXCEPTION: Config::save(): %s\n", e.what());
        reset();
    }
}

void Config::load(string path, string name, string extension, string* last) {
    try {
        if (name == "") return;

        // cout << "LOAD: " << path + name + extension << endl;

        ifstream ifs(path + name + extension, ifstream::in);
        if (ifs.fail()) return;
        boost::archive::text_iarchive ia(ifs);
        ia >> *this;
        ifs.close();

        if (last) *last = path + name + extension;
    } catch (exception &e) {
        fprintf(stderr, "EXCEPTION: Config::load(): %s\n", e.what());
        reset();
    }
}

void Config::reset() {
    *this = Config();
}