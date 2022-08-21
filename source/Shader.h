#pragma once

// #include <iostream>
// #include <iomanip>

#include "Shape.h"

using namespace std;

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

struct ShaderProperties {
    unsigned int program = 0;
    unsigned int vertexBuffer = 0;
    unsigned int indexBuffer = 0;
    unsigned int vertexArray = 0;
};

namespace Shader {
    void create(ShaderProperties &, Shape &);
    void destroy(ShaderProperties &);
    void activate(ShaderProperties &);
    void deactivate(ShaderProperties &);
    void createProgram(ShaderProperties &);
}