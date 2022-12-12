#include "Shader.h"

Shader::Shader(Shape& s) {
    glGenBuffers(1, &vertexBuffer);
    glGenBuffers(1, &indexBuffer);
    glGenVertexArrays(1, &vertexArray);

    createProgram();

    // the following indented lines contain settings that are stored in the VertexArrayObject
    // clang-format off
    glBindVertexArray(vertexArray);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, s.verticesSize * sizeof(float), s.vertices.get(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, s.indicesSize * sizeof(unsigned), s.indices.get(), GL_STATIC_DRAW);

        glVertexAttribPointer(s.args_vap.index, s.args_vap.size, s.args_vap.type, s.args_vap.normalized, s.args_vap.stride, s.args_vap.pointer);
        glEnableVertexAttribArray(s.args_vap.index);
    glBindVertexArray(0);
    // clang-format on

    // unbinding is a redundant: before any star is drawn, its properties are bound
    // glDisableVertexAttribArray(s.index);
    // glBindBuffer(GL_ARRAY_BUFFER, 0);
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

Shader::~Shader() {
    glDeleteProgram(program);

    glDeleteVertexArrays(1, &vertexArray);

    // necessary?
    // glInvalidateBufferData(vertexBuffer);
    // glInvalidateBufferData(indexBuffer);

    glDeleteBuffers(1, &vertexBuffer);
    glDeleteBuffers(1, &indexBuffer);
}

void Shader::activate() const {
    glUseProgram(program);
    glBindVertexArray(vertexArray);
}

void Shader::deactivate() {
    glBindVertexArray(0);
    glUseProgram(0);
}

void Shader::createProgram() {
    unsigned vertexShader, fragmentShader;

    {
        vertexShader = glCreateShader(GL_VERTEX_SHADER);

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

            std::cerr << "ERROR: Shader::createProgram(): VERTEX: COMPILE_FAILURE:\n"
                      << log << '\n';
        };
    }
    {
        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

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

            std::cerr << "ERROR: Shader::createProgram(): FRAGMENT: COMPILE_FAILURE:\n"
                      << log << '\n';
        };
    }
    {
        program = glCreateProgram();

        int linked;

        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);
        glGetProgramiv(program, GL_LINK_STATUS, &linked);

        if (!linked) {
            int logLength;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

            if (logLength < 1) logLength = 1;

            char log[logLength];
            glGetProgramInfoLog(program, logLength, NULL, log);

            std::cerr << "ERROR: Shader::createProgram(): PROGRAM: LINK_FAILURE:\n"
                      << log << '\n';
        }
    }

    // since the shaders are now compiled and linked with the program, we don't need the originals anymore
    glDeleteShader(fragmentShader);
    glDeleteShader(vertexShader);
}