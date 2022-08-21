#pragma once

#include <memory>
#include <cmath>

#include <glad/gl.h>

#include "MathConstants.h"

using namespace std;

struct Shape {
    unsigned int verticesSize;
    unique_ptr<float[]> vertices;

    unsigned int indicesSize;
    unique_ptr<unsigned int[]> indices;

    // https://docs.gl/gl4/glVertexAttribPointer
    struct args_glVertexAttribPointer {
        GLuint index;
        GLint size;
        GLenum type;
        GLboolean normalized;
        GLsizei stride;
        const void* pointer;
    } args_vap;

    // https://docs.gl/gl4/glDrawElements
    struct args_glDrawElements {
        GLenum mode;
        GLsizei count;
        GLenum type;
        const void* indices;
    } args_de;
};