#ifndef SHAPE_H_GUARD
#define SHAPE_H_GUARD

#include <memory>

#include <glad/gl.h>

struct Shape {
    unsigned verticesSize;
    std::unique_ptr<float[]> vertices;

    unsigned indicesSize;
    std::unique_ptr<unsigned[]> indices;

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

#endif