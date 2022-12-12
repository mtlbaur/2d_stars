#include "star_shape.h"

StarShape::StarShape(int& tips, double& iRadius, double& oRadius)
    : tips(tips), iRadius(iRadius), oRadius(oRadius) {

    const StarStyle& s = cfg.style;

    if (s.core.full && s.core.empty || !s.core.full && !s.core.empty) {
        switch (rng.I(0, 1)) {
            case 0:
                fullCore();
                break;
            case 1:
                emptyCore();
                break;
        }
    } else if (s.core.full) {
        fullCore();
    } else {
        emptyCore();
    }

    if (s.draw.fill && s.draw.line || !s.draw.fill && !s.draw.line) {
        switch (rng.I(0, 1)) {
            case 0:
                fillDraw();
                break;
            case 1:
                lineDraw();
                break;
        }
    } else if (s.draw.fill) {
        fillDraw();
    } else {
        lineDraw();
    }
}

void StarShape::fullCore() {
    style.core = Core::FULL;

    float slice = Constants::PI / tips; // note: PI, not TWO_PI

    verticesSize = tips * 2 * 2 + 2;  // tip_triangles * slices_per_triangle * <x, y> + origin
    indicesSize  = (tips + tips) * 3; // (tip_triangles + inner_triangles) * vertices_per_triangle

    vertices = std::make_unique<float[]>(verticesSize);
    indices  = std::make_unique<unsigned[]>(indicesSize);

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

        int diff   = indicesSize / 2;
        int origin = verticesSize / 2 - 1;

        unsigned i = 0;

        while (i < indicesSize / 2) {
            indices[i + diff] = indices[i];
            i++;
            indices[i + diff] = origin;
            i++;
            indices[i + diff] = indices[i];
            i++;
        }
    }

    // attributes pointer: https://docs.gl/gl4/glVertexAttribPointer

    args_vap.index      = 0;        // index == 0 corresponds to "location" in "layout (location = 0) in vec3 vertex;" in star.vs
    args_vap.size       = 2;        // we have 2 components per vertex: <x, y>
    args_vap.type       = GL_FLOAT; // each vertex has floats as the <x, y> types
    args_vap.normalized = GL_FALSE;
    args_vap.stride     = args_vap.size * sizeof(float); // to get to the next vertex, add (3 * sizeof(float)) to the index
    args_vap.pointer    = (void*)0;
}

void StarShape::emptyCore() {
    style.core = Core::EMPTY;

    float slice = Constants::PI / tips; // note: PI, not TWO_PI

    verticesSize = tips * 2 * 2; // tip_triangles * slices_per_triangle * <x, y>
    indicesSize  = tips * 3;     // tip_triangles * vertices_per_triangle

    vertices = std::make_unique<float[]>(verticesSize);
    indices  = std::make_unique<unsigned[]>(indicesSize);

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

    // attributes pointer: https://docs.gl/gl4/glVertexAttribPointer

    args_vap.index      = 0;        // index == 0 corresponds to "location" in "layout (location = 0) in vec3 vertex;" in star.vs
    args_vap.size       = 2;        // we have 2 components per vertex: <x, y>
    args_vap.type       = GL_FLOAT; // each vertex has floats as the <x, y> types
    args_vap.normalized = GL_FALSE;
    args_vap.stride     = args_vap.size * sizeof(float); // to get to the next vertex, add (3 * sizeof(float)) to the index
    args_vap.pointer    = (void*)0;
}

void StarShape::fillDraw() {
    style.draw = Draw::FILL;

    args_de.mode    = GL_TRIANGLES;
    args_de.count   = tips * 3 * (style.core == Core::FULL ? 2 : 1);
    args_de.type    = GL_UNSIGNED_INT;
    args_de.indices = (void*)0;
}

void StarShape::lineDraw() {
    style.draw = Draw::LINE;

    args_de.mode    = GL_LINE_STRIP;
    args_de.count   = tips * 3 * (style.core == Core::FULL ? 2 : 1);
    args_de.type    = GL_UNSIGNED_INT;
    args_de.indices = (void*)0;
}