#include "Shader.h"

void Shader::create(ShaderProperties &p, Shape &s) {
    glGenBuffers(1, &p.vertexBuffer);
    glGenBuffers(1, &p.indexBuffer);
    glGenVertexArrays(1, &p.vertexArray);

    // for (unsigned int i = 0; i < s.verticesSize; i += 2)
    //     cout << fixed << "(" << s.vertices[i] << ", " << s.vertices[i + 1] << ")" << endl;
    // for (unsigned int i = 0; i < s.indicesSize; i += 3)
    //     cout << fixed << "(" << s.indices[i] << ", " << s.indices[i + 1] << ", " << s.indices[i + 2] << ")" << endl;
    // cout << "----" << endl;

    Shader::createProgram(p);

    // the following indented lines contain settings that are stored in the VertexArrayObject
    // clang-format off
    glBindVertexArray(p.vertexArray);
        glBindBuffer(GL_ARRAY_BUFFER, p.vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, s.verticesSize * sizeof(float), s.vertices.get(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, p.indexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, s.indicesSize * sizeof(unsigned int), s.indices.get(), GL_STATIC_DRAW);

        glVertexAttribPointer(s.args_vap.index, s.args_vap.size, s.args_vap.type, s.args_vap.normalized, s.args_vap.stride, s.args_vap.pointer);
        glEnableVertexAttribArray(s.args_vap.index);
    glBindVertexArray(0);
    // clang-format on

    // unbinding is a redundant: before any star is drawn, its properties are bound
    // glDisableVertexAttribArray(s.index);
    // glBindBuffer(GL_ARRAY_BUFFER, 0);
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Shader::destroy(ShaderProperties &p) {
    glDeleteProgram(p.program);

    glDeleteVertexArrays(1, &p.vertexArray);

    // necessary?
    glInvalidateBufferData(p.vertexBuffer);
    glInvalidateBufferData(p.indexBuffer);

    glDeleteBuffers(1, &p.vertexBuffer);
    glDeleteBuffers(1, &p.indexBuffer);
}

void Shader::activate(ShaderProperties &p) {
    glUseProgram(p.program);
    glBindVertexArray(p.vertexArray);
}

void Shader::deactivate(ShaderProperties &p) {
    glBindVertexArray(0);
    glUseProgram(0);
}

void Shader::createProgram(ShaderProperties &p) {
    unsigned int vertexShader, fragmentShader;

    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    {
        int compiled;
        const char* sourcePtr = vertexSource;

        glShaderSource(vertexShader, 1, &sourcePtr, NULL);
        glCompileShader(vertexShader);
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &compiled);

        if (!compiled) {
            int logLength;
            glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &logLength);

            if (logLength < 1) logLength = 1;

            char log[logLength];
            glGetShaderInfoLog(vertexShader, logLength, NULL, log);

            fprintf(stderr, "ERROR: Shader::createProgram(): VERTEX: COMPILE_FAILURE:\n%s\n", log);
        };
    }

    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    {
        int compiled;
        const char* sourcePtr = fragmentSource;

        glShaderSource(fragmentShader, 1, &sourcePtr, NULL);
        glCompileShader(fragmentShader);
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &compiled);

        if (!compiled) {
            int logLength;
            glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &logLength);

            if (logLength < 1) logLength = 1;

            char log[logLength];
            glGetShaderInfoLog(fragmentShader, logLength, NULL, log);

            fprintf(stderr, "ERROR: Shader::createProgram(): FRAGMENT: COMPILE_FAILURE:\n%s\n", log);
        };
    }

    p.program = glCreateProgram();
    {
        int linked;

        glAttachShader(p.program, vertexShader);
        glAttachShader(p.program, fragmentShader);
        glLinkProgram(p.program);
        glGetProgramiv(p.program, GL_LINK_STATUS, &linked);

        if (!linked) {
            int logLength;
            glGetProgramiv(p.program, GL_INFO_LOG_LENGTH, &logLength);

            if (logLength < 1) logLength = 1;

            char log[logLength];
            glGetProgramInfoLog(p.program, logLength, NULL, log);

            fprintf(stderr, "ERROR: Shader::createProgram(): PROGRAM: LINK_FAILURE:\n%s\n", log);
        }
    }

    // since the shaders are now compiled and linked with the program, we don't need the originals anymore
    glDeleteShader(fragmentShader);
    glDeleteShader(vertexShader);
}