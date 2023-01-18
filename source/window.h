#ifndef WINDOW_H_GUARD
#define WINDOW_H_GUARD

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <glad/gl.h>
#include <GLFW/glfw3.h>

struct Window {
    GLFWwindow* glfw    = NULL;
    ImGuiContext* imgui = NULL;
    ImGuiIO* io         = NULL;

    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;

    bool exists = false;

    unsigned frameBuffer = 0;
    unsigned texture     = 0;
};

struct Windows {
    Window main;
    Window cfg;
};

#endif