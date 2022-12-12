#ifndef SHADER_H
#define SHADER_H

#include <iostream>

#include "shape.h"

constexpr char vertexSource[] =
    R"(#version 460 core

    layout (location = 0) in vec2 vertex;
    layout (location = 1) uniform mat4 transform;

    void main() {
        gl_Position = transform * vec4(vertex.x, vertex.y, 0.0f, 1.0f);
    };)";

constexpr char fragmentSource[] =
    R"(#version 460 core

    layout (location = 2) uniform vec4 uniColor;
    out vec4 outColor;

    void main() {
        outColor = uniColor;
    };)";

struct Shader {
    unsigned program      = 0;
    unsigned vertexBuffer = 0;
    unsigned indexBuffer  = 0;
    unsigned vertexArray  = 0;

    Shader(Shape&);
    ~Shader();
    void activate() const;
    static void deactivate();
    void createProgram();
};

#endif