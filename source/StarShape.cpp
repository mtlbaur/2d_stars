// #include <iostream>
// #include <iomanip>

#include "StarShape.h"

StarShape::StarShape(int &tips, double &iRadius, double &oRadius, int styleCore, int styleDraw) {
    this->tips = tips;
    this->iRadius = iRadius;
    this->oRadius = oRadius;
    this->styleCore = styleCore;
    this->styleDraw = styleDraw;

    define();
}

void StarShape::define() {
    if (styleCore == 0) {
        fillCore();
    } else {
        hollowCore();
    }

    if (styleDraw == 0) {
        fillDraw();
    } else {
        lineDraw();
    }
}

void StarShape::fillCore() {
    float slice = PI / tips; // note: PI, not TWO_PI

    verticesSize = tips * 2 * 2 + 2; // tip_triangles * slices_per_triangle * <x, y> + origin
    indicesSize = (tips + tips) * 3; // (tip_triangles + inner_triangles) * vertices_per_triangle

    vertices = make_unique<float[]>(verticesSize);
    indices = make_unique<unsigned int[]>(indicesSize);

    {
        // tip_triangles definition

        double a = 0.0;           // angle in radians -- adjusted to find the vertices that compose each tip/triangle of the star
        int b = 0, c = 0, d = -1; // index variables used to define shape

        /*
        The vertices of the star shape are stored in the vertices array.
        The indices array is then used to identify groups of three vertices that each compose one triangle of the star.
        The triangles of a star consist of:
            - the outer "tip" triangles that protrude from the polygon "core" of the star
            - and, if the shape is defined via fillCore(), the "inner" triangles whose bases consist of each of the sides of the polygon and whose common tip/third point is the origin/center of the star shape
        */
        for (int i = 0; i < tips; i++) {
            vertices[b++] = cos(a) * oRadius;
            vertices[b++] = sin(a) * oRadius;

            a += slice;

            vertices[b++] = cos(a) * iRadius;
            vertices[b++] = sin(a) * iRadius;

            a += slice;

            indices[c++] = d++;
            indices[c++] = d++;
            indices[c++] = d;
        }

        indices[0] = indices[--c] = d;

        // inner_triangles definition

        int diff = indicesSize / 2;
        int origin = verticesSize / 2 - 1;

        unsigned int i = 0;
        while (i < indicesSize / 2) {
            indices[i + diff] = indices[i];
            i++;
            indices[i + diff] = origin;
            i++;
            indices[i + diff] = indices[i];
            i++;
        }
    }

    // for (unsigned int i = 0; i < verticesSize; i += 2) {
    //     cout << fixed << i << ": (" << vertices[i] << ", " << vertices[i + 1] << ")" << endl;
    // }
    // for (unsigned int i = 0; i < indicesSize; i += 3) {
    //     cout << fixed << i << ": (" << indices[i] << ", " << indices[i + 1] << ", " << indices[i + 2] << ")" << endl;
    // }
    // cout << "----" << endl;

    // attributes pointer: https://docs.gl/gl4/glVertexAttribPointer

    args_vap.index = 0;       // index == 0 corresponds to "location" in "layout (location = 0) in vec3 vertex;" in star.vs
    args_vap.size = 2;        // we have 2 components per vertex: <x, y>
    args_vap.type = GL_FLOAT; // each vertex has floats as the <x, y> types
    args_vap.normalized = GL_FALSE;
    args_vap.stride = args_vap.size * sizeof(float); // to get to the next vertex, add (3 * sizeof(float)) to the index
    args_vap.pointer = (void*)0;
}

void StarShape::hollowCore() {
    float slice = PI / tips; // note: PI, not TWO_PI

    verticesSize = tips * 2 * 2; // tip_triangles * slices_per_triangle * <x, y>
    indicesSize = tips * 3;      // tip_triangles * vertices_per_triangle

    vertices = make_unique<float[]>(verticesSize);
    indices = make_unique<unsigned int[]>(indicesSize);

    double a = 0.0;           // angle in radians -- adjusted to find the vertices that compose each tip/triangle of the star
    int b = 0, c = 0, d = -1; // index variables used to define shape

    {
        /*
        The vertices of the star shape are stored in the vertices array.
        The indices array is then used to identify groups of three vertices that each compose one triangle of the star.
        The triangles of a star consist of:
            - the outer "tip" triangles that protrude from the polygon "core" of the star
            - and, if the shape is defined via fillCore(), the "inner" triangles whose bases consist of each of the sides of the polygon and whose common tip/third point is the origin/center of the star shape
        */
        for (int i = 0; i < tips; i++) {
            vertices[b++] = cos(a) * oRadius;
            vertices[b++] = sin(a) * oRadius;

            a += slice;

            vertices[b++] = cos(a) * iRadius;
            vertices[b++] = sin(a) * iRadius;

            a += slice;

            indices[c++] = d++;
            indices[c++] = d++;
            indices[c++] = d;
        }

        indices[0] = indices[--c] = d;
    }

    // for (unsigned int i = 0; i < verticesSize; i += 2) {
    //     cout << fixed << i << ": (" << vertices[i] << ", " << vertices[i + 1] << ")" << endl;
    // }
    // for (unsigned int i = 0; i < indicesSize; i += 3) {
    //     cout << fixed << i << ": (" << indices[i] << ", " << indices[i + 1] << ", " << indices[i + 2] << ")" << endl;
    // }
    // cout << "----" << endl;

    // attributes pointer: https://docs.gl/gl4/glVertexAttribPointer

    args_vap.index = 0;       // index == 0 corresponds to "location" in "layout (location = 0) in vec3 vertex;" in star.vs
    args_vap.size = 2;        // we have 2 components per vertex: <x, y>
    args_vap.type = GL_FLOAT; // each vertex has floats as the <x, y> types
    args_vap.normalized = GL_FALSE;
    args_vap.stride = args_vap.size * sizeof(float); // to get to the next vertex, add (3 * sizeof(float)) to the index
    args_vap.pointer = (void*)0;
}

void StarShape::fillDraw() {
    args_de.mode = GL_TRIANGLES;
    args_de.count = tips * 3 * (styleCore == 0 ? 2 : 1);
    args_de.type = GL_UNSIGNED_INT;
    args_de.indices = (void*)0;
}

void StarShape::lineDraw() {
    args_de.mode = GL_LINE_STRIP;
    args_de.count = tips * 3 * (styleCore == 0 ? 2 : 1);
    args_de.type = GL_UNSIGNED_INT;
    args_de.indices = (void*)0;
}