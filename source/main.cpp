#include <iostream>
#include <memory>
#include <charconv>

// https://www.boost.org/doc/libs/1_79_0/libs/filesystem/doc/tutorial.html
#include <boost/filesystem.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "star.h"

const int SCREEN_SIZE_X       = 1920;
const int SCREEN_SIZE_Y       = 1080;
const double SCREEN_SIZE_MULT = 0.75;

const int GLFW_CONTEXT_VER_MAJOR = 4;
const int GLFW_CONTEXT_VER_MINOR = 6;

const char GLSL_VER[] = "#version 130";

const int MSAA_SAMPLES = 4;

// If FPS goes lower than 30, don't attempt to keep star movement/physics consistent anymore.
// This also prevents long program pauses (== high frameTime values) from causing excessively incorrect behavior (e.g. stars teleporting).
// The drawback is slowdown when FPS falls below 30.
const double FRAME_TIME_MAX = 1.0 / 30.0;
const int MAX_STARS         = 8000;

const std::string PATH_SYSTEM = "./config/system/";
const std::string PATH_USER   = "./config/user/";
const std::string EXT_DEFAULT = ".cfg";

// https://docs.gl/gl4/glBlendFunc
// Used to allow the user to pick whatever blending combination they want.
// Note that a lot of combinations will result in useless blending (invisible stars is one example).
const int glBlendFunc_factor[] = {
    GL_ZERO,
    GL_ONE,
    GL_SRC_COLOR,
    GL_ONE_MINUS_SRC_COLOR,
    GL_DST_COLOR,
    GL_ONE_MINUS_DST_COLOR,
    GL_SRC_ALPHA,
    GL_ONE_MINUS_SRC_ALPHA,
    GL_DST_ALPHA,
    GL_ONE_MINUS_DST_ALPHA,
    GL_CONSTANT_COLOR,
    GL_ONE_MINUS_CONSTANT_COLOR,
    GL_CONSTANT_ALPHA,
    GL_ONE_MINUS_CONSTANT_ALPHA,
    GL_SRC_ALPHA_SATURATE,
    GL_SRC1_COLOR,
    GL_ONE_MINUS_SRC1_COLOR,
    GL_SRC1_ALPHA,
    GL_ONE_MINUS_SRC1_ALPHA};

#define GET_NAME(x) #x

const char* const glBlendFunc_factor_name[] = {
    GET_NAME(GL_ZERO),
    GET_NAME(GL_ONE),
    GET_NAME(GL_SRC_COLOR),
    GET_NAME(GL_ONE_MINUS_SRC_COLOR),
    GET_NAME(GL_DST_COLOR),
    GET_NAME(GL_ONE_MINUS_DST_COLOR),
    GET_NAME(GL_SRC_ALPHA),
    GET_NAME(GL_ONE_MINUS_SRC_ALPHA),
    GET_NAME(GL_DST_ALPHA),
    GET_NAME(GL_ONE_MINUS_DST_ALPHA),
    GET_NAME(GL_CONSTANT_COLOR),
    GET_NAME(GL_ONE_MINUS_CONSTANT_COLOR),
    GET_NAME(GL_CONSTANT_ALPHA),
    GET_NAME(GL_ONE_MINUS_CONSTANT_ALPHA),
    GET_NAME(GL_SRC_ALPHA_SATURATE),
    GET_NAME(GL_SRC1_COLOR),
    GET_NAME(GL_ONE_MINUS_SRC1_COLOR),
    GET_NAME(GL_SRC1_ALPHA),
    GET_NAME(GL_ONE_MINUS_SRC1_ALPHA)};

#undef GET_NAME

// contains all stars that appear on the main window
static std::vector<std::unique_ptr<Star>> stars;

// contains stars used in the preview of the config window
static struct PreviewStars {
    std::unique_ptr<Star> min;
    std::unique_ptr<Star> avg;
    std::unique_ptr<Star> max;

    void forceVisible() const {
        if (min && avg && max) {
            min->color->assign(0.0f, 1.0f, 1.0f, 1.0f);
            avg->color->assign(1.0f, 1.0f, 0.0f, 1.0f);
            max->color->assign(1.0f, 0.0f, 1.0f, 1.0f);
        }
    }
} pre;

// title of the main window
static struct MainWinTitle {
    char data[21] = "    " // space to fill with star count
                    " stars; "
                    "    " // space to fill with FPS count
                    " FPS";
    const int countL = 0;
    const int countR = 3;
    const int fpsL   = countR + 1 + 8;
    const int fpsR   = fpsL + 3;
} mainWinTitle;

// global struct: used to generate random values
RNG rng;
// global struct: contains various user-controlled settings
Config cfg;
// global struct: contains properties GLFW and ImGui windows
Windows win;
// global var: contains frame time used as a multiplier to make stars appear to move at the same speed regardless of FPS
double frameTime = 0.0;

// main loop

void execute();

// Star

void addRemoveStars(int);
void regenStars();
void updateStars();

// ImGui creation

void createGUI();
bool displayGenerationParameters();
void displayPreview(bool);

// GLFW window create/destruction

void createMainWin();
void destroyMainWin();

void createCfgWin();
void destroyCfgWin();
void createDestroyCfgWin();

// GLFW window utility

void maximizeRestoreWin(GLFWwindow*);
void clearWin(GLFWwindow*);
void setMainWinTitle();

// GLFW

static void errorCallback(int, const char*);

void mainWinKeyCallback(GLFWwindow*, int, int, int, int);
void mainWinPosCallback(GLFWwindow*, int, int);
void mainWinSizeCallback(GLFWwindow*, int, int);

void cfgWinKeyCallback(GLFWwindow*, int, int, int, int);
void cfgWinPosCallback(GLFWwindow*, int, int);
void cfgWinSizeCallback(GLFWwindow*, int, int);
void cfgWinCloseCallback(GLFWwindow*);

// utility

int cfgNameImGuiInputTextFilter(ImGuiInputTextCallbackData*);

int main() {
    stars.reserve(MAX_STARS);

    glfwSetErrorCallback(errorCallback);

    if (!glfwInit()) {
        std::cerr << "glfwInit(): failed\n";
        exit(1);
    }

    cfg.load(PATH_SYSTEM, "data", EXT_DEFAULT);

    createMainWin();
    createCfgWin();

    execute();                     // main program loop
    addRemoveStars(-stars.size()); // correctly free the memory for all the existing stars before shutdown

    destroyCfgWin();
    destroyMainWin();

    glfwTerminate();

    cfg.save(PATH_SYSTEM, "data", EXT_DEFAULT);
}

void execute() {
    std::chrono::steady_clock::time_point updateTime = std::chrono::steady_clock::now();

    while (!glfwWindowShouldClose(win.main.glfw)) {
        std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
        std::chrono::duration<double> deltaTime           = currentTime - updateTime; // seconds
        double targetSecondsPerFrame                      = 1.0 / (double)cfg.targetFPS;

        if (deltaTime.count() >= targetSecondsPerFrame) {
            updateTime = currentTime;
            // Update global frameTime with the last frame's delta time value.
            // A star with a velocity vector of (1, 0) will move 1 pixel per second in the positive x direction.
            frameTime = deltaTime.count();

            glfwPollEvents();

            glfwMakeContextCurrent(NULL);
            glfwMakeContextCurrent(win.main.glfw);

            glfwGetFramebufferSize(win.main.glfw, &win.main.w, &win.main.h);
            glViewport(0, 0, win.main.w, win.main.h);

            if (cfg.clear) {
                glClearColor(cfg.backgroundColor.r, cfg.backgroundColor.g, cfg.backgroundColor.b, cfg.backgroundColor.a);
                glClear(GL_COLOR_BUFFER_BIT);
            }

            setMainWinTitle();

            frameTime = std::min(frameTime, FRAME_TIME_MAX);

            // update and draw in OpenGL
            Star::prepareProjection(win.main.w, win.main.h);
            updateStars();

            glfwSwapBuffers(win.main.glfw);

            if (win.cfg.exists) {
                glfwMakeContextCurrent(NULL);
                glfwMakeContextCurrent(win.cfg.glfw);

                glfwGetFramebufferSize(win.cfg.glfw, &win.cfg.w, &win.cfg.h);
                glViewport(0, 0, win.cfg.w, win.cfg.h);
                glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
                glClear(GL_COLOR_BUFFER_BIT);

                ImGui::SetCurrentContext(win.cfg.imgui);
                ImGui_ImplOpenGL3_NewFrame();
                ImGui_ImplGlfw_NewFrame();
                ImGui::NewFrame();

                // make sure the ImGui UI is filling the GLFW window completely
                win.cfg.io->DisplaySize.x = win.cfg.w;
                win.cfg.io->DisplaySize.y = win.cfg.h;
                ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
                ImGui::SetNextWindowSize(ImVec2(win.cfg.w, win.cfg.h));

                createGUI();

                ImGui::Render();
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

                glfwSwapBuffers(win.cfg.glfw);
            }
        }
    }
}

void addRemoveStars(int n) {
    if (n == 0) return;

    glfwMakeContextCurrent(NULL);
    glfwMakeContextCurrent(win.main.glfw);

    if (n > 0) {
        n = std::min(n, MAX_STARS - static_cast<int>(stars.size()));

        for (; n > 0; n--) {
            stars.emplace_back(std::make_unique<Star>(Enum::Star::GenType::RNG));
        }
    } else if (n < 0) {
        for (; !stars.empty() && n < 0; n++) {
            stars.pop_back();
        }
    }
}

void regenStars() {
    if (stars.empty()) return;

    glfwMakeContextCurrent(NULL);
    glfwMakeContextCurrent(win.main.glfw);

    int n = stars.size();

    addRemoveStars(-n);
    addRemoveStars(n);

    glClearColor(cfg.backgroundColor.r, cfg.backgroundColor.g, cfg.backgroundColor.b, cfg.backgroundColor.a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void updateStars() {
    if (cfg.collisions) {
        for (const std::unique_ptr<Star>& s : stars) {
            s->update();
            s->notCollided = true;
        }
        for (unsigned i = 0; i < stars.size(); i++) {
            if (stars[i]->notCollided) {
                for (unsigned j = i + 1; j < stars.size(); j++) {
                    if (stars[j]->notCollided && Star::collision(*stars[i], *stars[j])) {
                        stars[j]->notCollided = false;
                        break;
                    }
                }
            }
            stars[i]->draw();
        }
    } else {
        for (const std::unique_ptr<Star>& s : stars) {
            s->update();
            s->draw();
        }
    }
    Star::updateIndexUniformRGBColors();
}

// ImGui creation

void createGUI() {
    static const ImGuiWindowFlags WF = ImGuiWindowFlags_NoTitleBar |
                                       ImGuiWindowFlags_NoMove |
                                       ImGuiWindowFlags_NoResize |
                                       ImGuiWindowFlags_NoCollapse |
                                       ImGuiWindowFlags_HorizontalScrollbar;
    static const ImGuiSliderFlags SF = ImGuiSliderFlags_NoRoundToFormat |
                                       ImGuiSliderFlags_AlwaysClamp;
    static const char IF[] = "%d";   // int format
    static const char FF[] = "%.3f"; // float format
    static const double S  = 1000.0; // steps

    if (ImGui::Begin("Config", NULL, WF)) {
        if (ImGui::CollapsingHeader("Stars")) {
            ImGui::DragInt("##addRemoveStars", &cfg.addRemove, 500.0 / S, -500, 500, IF, SF);

            int amount = 0;

            ImGui::SameLine();
            if (ImGui::Button("+-"))
                amount += cfg.addRemove;

            if (ImGui::Button("+1"))
                amount++;

            ImGui::SameLine();
            if (ImGui::Button("-1"))
                amount--;

            ImGui::SameLine();
            if (ImGui::Button("Clear"))
                amount += -stars.size();

            if (amount != 0) {
                addRemoveStars(amount);

                glfwMakeContextCurrent(NULL);
                glfwMakeContextCurrent(win.cfg.glfw);
            }
        }
        if (ImGui::CollapsingHeader("Effects")) {
            ImGui::Checkbox("Clear Screen", &cfg.clear);

            bool blendModeAdjusted = false;

            // https://docs.gl/gl4/glBlendFunc
            ImGui::Separator();
            blendModeAdjusted |= ImGui::Combo("Src Blend Mode", &cfg.srcBlendMode, glBlendFunc_factor_name, IM_ARRAYSIZE(glBlendFunc_factor_name));

            ImGui::Separator();
            blendModeAdjusted |= ImGui::Combo("Dst Blend Mode", &cfg.dstBlendMode, glBlendFunc_factor_name, IM_ARRAYSIZE(glBlendFunc_factor_name));

            if (blendModeAdjusted) {
                glfwMakeContextCurrent(NULL);
                glfwMakeContextCurrent(win.main.glfw);

                glBlendFunc(glBlendFunc_factor[cfg.srcBlendMode], glBlendFunc_factor[cfg.dstBlendMode]);

                glfwMakeContextCurrent(NULL);
                glfwMakeContextCurrent(win.cfg.glfw);
            }

            ImGui::Separator();
            ImGui::Combo("Color Mode", &cfg.colorMode, "Default\0Random\0Uniform\0Consistent\0");
            ImGui::DragFloat("Color Shift Mult", &cfg.colorShiftMult, 1000.0f / S, -1000.0f, 1000.0f, FF, SF);

            ImGui::Separator();
            ImGui::ColorEdit4("Background Color", (float*)&cfg.backgroundColor);
        }
        if (ImGui::CollapsingHeader("Physics")) {
            ImGui::Checkbox("Collisions", &cfg.collisions);

            ImGui::Separator();
            ImGui::DragFloat("##gravityVal", &cfg.gravityVal, 100.0f / S, -100.0f, 100.0f, FF, SF);

            ImGui::SameLine();
            ImGui::Checkbox("Gravity", &cfg.gravity);

            ImGui::Separator();
            ImGui::DragFloat("##accelMult", &cfg.accelMult, 2.0f / S, 0.0f, 2.0f, FF, SF);

            ImGui::SameLine();
            ImGui::Checkbox("Accel", &cfg.accel);

            ImGui::DragFloatRange2("Speed Limit", &cfg.minSpeedLimit, &cfg.maxSpeedLimit, 1000.0f / S, 0.0f, 1000.0f, FF, FF, SF);

            ImGui::SameLine();
            ImGui::Checkbox("Min", &cfg.minSpeed);

            ImGui::SameLine();
            ImGui::Checkbox("Max", &cfg.maxSpeed);
        }
        if (ImGui::CollapsingHeader("Parameters")) {
            if (ImGui::Button("Apply")) {
                regenStars();

                glfwMakeContextCurrent(NULL);
                glfwMakeContextCurrent(win.cfg.glfw);
            }

            ImGui::SameLine();
            if (ImGui::Button("Save/Load")) {
                cfg.files.load(PATH_USER, EXT_DEFAULT);
                ImGui::OpenPopup("Save/Load Config");
            }

            bool reloadPreview = false;

            // BEGIN SAVE/LOAD POPUP

            bool p_open_save_load = true;

            if (ImGui::BeginPopupModal("Save/Load Config", &p_open_save_load, ImGuiWindowFlags_AlwaysAutoResize)) {
                static int i                         = 0;
                static const int FILE_NAME_SIZE      = 128;
                static char fileName[FILE_NAME_SIZE] = "";

                int bytesToCopy = 0;

                if (ImGui::BeginListBox("##fileNames", ImVec2(400, 400))) {
                    for (int j = 0; j < cfg.files.list.size(); j++) {
                        bool selected = i == j;

                        if (ImGui::Selectable(cfg.files.names[j].data(), selected)) {
                            i = j;

                            bytesToCopy = sizeof(char) * std::min(FILE_NAME_SIZE - 1, static_cast<int>(cfg.files.names[i].size()));

                            memcpy(fileName, cfg.files.names[i].data(), bytesToCopy);
                            fileName[bytesToCopy] = '\0';
                        }
                        if (selected) ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndListBox();
                }

                ImGui::Separator();

                ImGui::InputText("Name", fileName, FILE_NAME_SIZE, ImGuiInputTextFlags_CallbackCharFilter, cfgNameImGuiInputTextFilter);

                ImGui::SameLine();

                if (ImGui::Button("Clear")) {
                    memset(fileName, '\0', 1);
                }

                ImGui::Separator();

                if (ImGui::Button("Save")) {
                    ImGui::CloseCurrentPopup();
                    cfg.save(PATH_USER, fileName, EXT_DEFAULT);
                }

                if (!cfg.files.list.empty() && i >= 0 && i < cfg.files.list.size()) {
                    ImGui::SameLine(0.0f, 32.0f);

                    if (ImGui::Button("Load")) {
                        ImGui::CloseCurrentPopup();

                        cfg.load(PATH_USER, cfg.files.names[i], EXT_DEFAULT);

                        memcpy(fileName, cfg.files.names[i].data(), bytesToCopy);
                        fileName[bytesToCopy] = '\0';

                        regenStars();
                        reloadPreview |= true;

                        glfwMakeContextCurrent(NULL);
                        glfwMakeContextCurrent(win.cfg.glfw);
                    }

                    ImGui::SameLine(0.0f, 32.0f);

                    if (ImGui::Button("Delete")) {
                        boost::filesystem::remove(cfg.files.list[i].path());
                        i = std::max(i - 1, 0);
                        cfg.files.load(PATH_USER, EXT_DEFAULT);
                    }
                }

                ImGui::EndPopup();
            }

            // END SAVE/LOAD POPUP

            ImGui::SameLine();
            if (ImGui::Button("Reset")) {
                cfg.reset();

                regenStars();
                reloadPreview |= true;

                glfwMakeContextCurrent(NULL);
                glfwMakeContextCurrent(win.cfg.glfw);
            }

            ImGui::SameLine();
            ImGui::LabelText("##configOptions", cfg.files.last.data());

            ImGui::Separator();
            ImGui::DragInt("Target FPS", &cfg.targetFPS, 1000.0f / S, 60, 1000, IF, SF);

            displayPreview(displayGenerationParameters() | reloadPreview);
        }
    }

    ImGui::End();
}

bool displayGenerationParameters() {
    static const ImGuiSliderFlags SF = ImGuiSliderFlags_NoRoundToFormat |
                                       ImGuiSliderFlags_AlwaysClamp;
    static const char IF[] = "%d";   // int format
    static const char FF[] = "%.3f"; // float format
    static const double S  = 1000.0; // steps

    static StarProperties& min  = cfg.min;
    static StarProperties& max  = cfg.max;
    static StarProperties& lmin = cfg.limMin;
    static StarProperties& lmax = cfg.limMax;

    bool adjusted = false;

    ImGui::Separator();
    adjusted |= ImGui::DragIntRange2("Tips", &min.tips, &max.tips, lmax.tips / S, lmin.tips, lmax.tips, IF, IF, SF);

    ImGui::Separator();
    adjusted |= ImGui::DragFloatRange2("x Velocity", &min.xVel, &max.xVel, lmax.xVel / S, lmin.xVel, lmax.xVel, FF, FF, SF);

    ImGui::Separator();
    adjusted |= ImGui::DragFloatRange2("y Velocity", &min.yVel, &max.yVel, lmax.yVel / S, lmin.yVel, lmax.yVel, FF, FF, SF);

    ImGui::Separator();
    adjusted |= ImGui::DragFloatRange2("Angle", &min.ang, &max.ang, lmax.ang / S, lmin.ang, lmax.ang, FF, FF, SF);

    ImGui::Separator();
    adjusted |= ImGui::DragFloatRange2("Angular Velocity", &min.angVel, &max.angVel, lmax.angVel / S, lmin.angVel, lmax.angVel, FF, FF, SF);

    ImGui::Separator();
    adjusted |= ImGui::DragFloatRange2("Inner Radius", &min.iRadius, &max.iRadius, lmax.iRadius / S, lmin.iRadius, lmax.iRadius, FF, FF, SF);

    ImGui::Separator();
    adjusted |= ImGui::DragFloatRange2("Outer Radius", &min.oRadius, &max.oRadius, lmax.oRadius / S, lmin.oRadius, lmax.oRadius, FF, FF, SF);

    ImGui::Separator();
    adjusted |= ImGui::DragFloatRange2("Density", &min.density, &max.density, lmax.density / S, lmin.density, lmax.density, FF, FF, SF);

    ImGui::Separator();
    adjusted |= ImGui::DragFloatRange2("Red", &min.color.r, &max.color.r, lmax.color.r / S, lmin.color.r, lmax.color.r, FF, FF, SF);

    ImGui::Separator();
    adjusted |= ImGui::DragFloatRange2("Green", &min.color.g, &max.color.g, lmax.color.g / S, lmin.color.g, lmax.color.g, FF, FF, SF);

    ImGui::Separator();
    adjusted |= ImGui::DragFloatRange2("Blue", &min.color.b, &max.color.b, lmax.color.b / S, lmin.color.b, lmax.color.b, FF, FF, SF);

    ImGui::Separator();
    adjusted |= ImGui::DragFloatRange2("Alpha", &min.color.a, &max.color.a, lmax.color.a / S, lmin.color.a, lmax.color.a, FF, FF, SF);

    ImGui::Separator();
    ImGui::DragFloat("Min Color", &cfg.minColor, 1.0f / S, 0.0f, 1.0f, FF, SF);

    ImGui::Separator();
    adjusted |= ImGui::Checkbox("Full Core", &cfg.style.core.full);
    ImGui::SameLine();
    adjusted |= ImGui::Checkbox("Empty Core", &cfg.style.core.empty);

    ImGui::Separator();
    adjusted |= ImGui::Checkbox("Fill Draw", &cfg.style.draw.fill);
    ImGui::SameLine();
    adjusted |= ImGui::Checkbox("Line Draw", &cfg.style.draw.line);

    ImGui::Separator();
    adjusted |= ImGui::Checkbox("Force Visible Preview", &cfg.forceVisiblePreview);

    return adjusted;
}

void displayPreview(bool adjusted) {
    // if any settings were changed in the last frame, construct new preview stars so that those settings are applied
    if (adjusted) {
        pre.min = std::make_unique<Star>(Enum::Star::GenType::MIN);
        pre.avg = std::make_unique<Star>(Enum::Star::GenType::AVG);
        pre.max = std::make_unique<Star>(Enum::Star::GenType::MAX);

        if (cfg.forceVisiblePreview) pre.forceVisible();
    }

    // spacing, width, height
    int s = 10, w = 0, h = 0;

    // expand/contract the preview size based on the size of the stars (if the preview window exceeds the size of the config window, then scrolling can be used)

    w += s;

    w += pre.min->iRadius + pre.min->oRadius;
    pre.min->x = w;
    w += pre.min->iRadius + pre.min->oRadius;

    w += s;

    w += pre.avg->iRadius + pre.avg->oRadius;
    pre.avg->x = w;
    w += pre.avg->iRadius + pre.avg->oRadius;

    w += s;

    w += pre.max->iRadius + pre.max->oRadius;
    pre.max->x = w;
    w += pre.max->iRadius + pre.max->oRadius;

    w += s;

    h = s + 2 * pre.max->iRadius + 2 * pre.max->oRadius + s;

    pre.min->y = pre.avg->y = pre.max->y = h / 2;

    // clang-format off
    glBindTexture(GL_TEXTURE_2D, win.cfg.texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL); // adjust width/height of texture based on star sizes
    glBindTexture(GL_TEXTURE_2D, 0);

    // antialiased preview doesn't work with ImGui's AddImage() -- would need some sort of workaround
    // glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, win.cfg.texture);
    //     glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, MSAA_SAMPLES, GL_RGBA, w, h, GL_TRUE);
    // glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, win.cfg.frameBuffer);
        glViewport(0, 0, w, h); // adjust frame buffer width/height based on star sizes
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        Star::prepareProjection(w, h);

        // following draws into the frame buffer's data which then gets displayed in ImGui as a texture
        pre.min->draw();
        pre.avg->draw();
        pre.max->draw();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // clang-format on

    ImGui::Separator();
    ImGui::GetWindowDrawList()->AddImage((void*)win.cfg.texture,
                                         ImVec2(ImGui::GetCursorPosX() - ImGui::GetScrollX(), ImGui::GetCursorPosY() - ImGui::GetScrollY()),
                                         ImVec2(ImGui::GetCursorPosX() + w - ImGui::GetScrollX(), ImGui::GetCursorPosY() + h - ImGui::GetScrollY()),
                                         ImVec2(0, 1),
                                         ImVec2(1, 0));

    // Above image doesn't behave like an ImGui element, so create a dummy rectangular element that is the dimensions of the image.
    // This fixes scrolling: without the dummy, you would not be able to scroll the image on/off screen.
    ImGui::Dummy(ImVec2(w, h));
}

// window creation/destruction

void createMainWin() {
    if (win.main.exists) return;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GLFW_CONTEXT_VER_MAJOR);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GLFW_CONTEXT_VER_MINOR);
    glfwWindowHint(GLFW_SAMPLES, MSAA_SAMPLES); // NOTE: if you disable GLFW_DOUBLEBUFFER, this will no longer work

    win.main.x = (SCREEN_SIZE_X / 2) - (SCREEN_SIZE_X * SCREEN_SIZE_MULT / 2);
    win.main.y = (SCREEN_SIZE_Y / 2) - (SCREEN_SIZE_Y * SCREEN_SIZE_MULT / 2);
    win.main.w = SCREEN_SIZE_X * SCREEN_SIZE_MULT;
    win.main.h = SCREEN_SIZE_Y * SCREEN_SIZE_MULT;

    win.main.glfw = glfwCreateWindow(win.main.w, win.main.h, "Main", NULL, NULL);
    if (!win.main.glfw) exit(1);

    glfwSetWindowPos(win.main.glfw, win.main.x, win.main.y);

    glfwMakeContextCurrent(win.main.glfw);
    if (!gladLoadGL(glfwGetProcAddress)) {
        std::cerr << "gladLoadGL(glfwGetProcAddress): failed\n";
        exit(1);
    }

    glEnable(GL_BLEND);
    glBlendFunc(glBlendFunc_factor[cfg.srcBlendMode], glBlendFunc_factor[cfg.dstBlendMode]);

    glEnable(GL_MULTISAMPLE);

    glfwSetKeyCallback(win.main.glfw, mainWinKeyCallback);
    glfwSetWindowPosCallback(win.main.glfw, mainWinPosCallback);
    glfwSetWindowSizeCallback(win.main.glfw, mainWinSizeCallback);

    win.main.exists = true;
}

void destroyMainWin() {
    if (!win.main.exists) return;

    glfwDestroyWindow(win.main.glfw);

    win.main.glfw   = NULL;
    win.main.imgui  = NULL;
    win.main.exists = false;
}

void createCfgWin() {
    if (win.cfg.exists) return;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GLFW_CONTEXT_VER_MAJOR);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GLFW_CONTEXT_VER_MINOR);
    // glfwWindowHint(GLFW_SAMPLES, 4);

    win.cfg.w = 800;
    win.cfg.h = 800;
    win.cfg.x = win.main.x + win.main.w - win.cfg.w;
    win.cfg.y = win.main.y;

    win.cfg.glfw = glfwCreateWindow(win.cfg.w, win.cfg.h, "Config", NULL, NULL);
    if (!win.cfg.glfw) exit(1);

    glfwSetWindowPos(win.cfg.glfw, win.cfg.x, win.cfg.y);

    glfwMakeContextCurrent(NULL);
    glfwMakeContextCurrent(win.cfg.glfw);
    if (!gladLoadGL(glfwGetProcAddress)) {
        std::cerr << "gladLoadGL(glfwGetProcAddress) failed\n";
        exit(1);
    }

    // glEnable(GL_MULTISAMPLE);

    glGenFramebuffers(1, &win.cfg.frameBuffer);
    glGenTextures(1, &win.cfg.texture);

    // clang-format off
    // This texture is used for the preview window.
    // ImGui requires a texture to be able to add a custom image to an ImGui window, so that's what is being created here.
    glBindTexture(GL_TEXTURE_2D, win.cfg.texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, win.cfg.w, win.cfg.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    // An antialiased texture doesn't work with ImGui's AddImage() function, so a workaround would be necessary for this.
    // glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, win.cfg.texture);
    //     glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, MSAA_SAMPLES, GL_RGBA, win.cfg.w, win.cfg.h, GL_TRUE);
    //     glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //     glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, win.cfg.frameBuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, win.cfg.texture, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // glBindFramebuffer(GL_FRAMEBUFFER, win.cfg.frameBuffer);
    //     glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, win.cfg.texture, 0);
    // glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // clang-format on

    // Set customized GLFW callbacks.
    // NOTE: set GLFW callbacks before ImGui creation (otherwise you can will have missing events if you call ImGui_ImplGlfw_InitForOpenGL(window, true) first).
    glfwSetKeyCallback(win.cfg.glfw, cfgWinKeyCallback);
    glfwSetWindowCloseCallback(win.cfg.glfw, cfgWinCloseCallback);
    glfwSetWindowPosCallback(win.cfg.glfw, cfgWinPosCallback);
    glfwSetWindowSizeCallback(win.cfg.glfw, cfgWinSizeCallback);

    // forward all other events directly to available ImGui implementation callbacks
    glfwSetWindowFocusCallback(win.cfg.glfw, ImGui_ImplGlfw_WindowFocusCallback);
    glfwSetCursorEnterCallback(win.cfg.glfw, ImGui_ImplGlfw_CursorEnterCallback);
    glfwSetCursorPosCallback(win.cfg.glfw, ImGui_ImplGlfw_CursorPosCallback);
    glfwSetMouseButtonCallback(win.cfg.glfw, ImGui_ImplGlfw_MouseButtonCallback);
    glfwSetScrollCallback(win.cfg.glfw, ImGui_ImplGlfw_ScrollCallback);
    glfwSetCharCallback(win.cfg.glfw, ImGui_ImplGlfw_CharCallback);

    IMGUI_CHECKVERSION();
    win.cfg.imgui = ImGui::CreateContext();
    ImGui::SetCurrentContext(win.cfg.imgui);
    ImGui::StyleColorsDark();

    win.cfg.io              = &ImGui::GetIO();
    win.cfg.io->IniFilename = NULL; // disable useless (for this window) ImGui ini file
    // win.cfg.io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    // win.cfg.io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

    ImGui_ImplGlfw_InitForOpenGL(win.cfg.glfw, false); // second param == false --> do not auto install callbacks because they are already defined above
    ImGui_ImplOpenGL3_Init(GLSL_VER);

    pre.min = std::make_unique<Star>(Enum::Star::GenType::MIN);
    pre.avg = std::make_unique<Star>(Enum::Star::GenType::AVG);
    pre.max = std::make_unique<Star>(Enum::Star::GenType::MAX);

    if (cfg.forceVisiblePreview) pre.forceVisible();

    win.cfg.exists = true;
}

void destroyCfgWin() {
    if (!win.cfg.exists) return;

    glfwMakeContextCurrent(NULL);
    glfwMakeContextCurrent(win.cfg.glfw);

    ImGui::SetCurrentContext(win.cfg.imgui);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glDeleteTextures(1, &win.cfg.texture);
    glDeleteFramebuffers(1, &win.cfg.frameBuffer);

    glfwDestroyWindow(win.cfg.glfw);

    pre.min.reset();
    pre.avg.reset();
    pre.max.reset();

    win.cfg.glfw   = NULL;
    win.cfg.imgui  = NULL;
    win.cfg.exists = false;

    if (win.main.exists) glfwFocusWindow(win.main.glfw);
}

void createDestroyCfgWin() {
    if (win.cfg.exists) destroyCfgWin();
    else createCfgWin();
}

void maximizeRestoreWin(GLFWwindow* window) {
    if (glfwGetWindowAttrib(window, GLFW_MAXIMIZED)) glfwRestoreWindow(window);
    else glfwMaximizeWindow(window);
}

void clearWin(GLFWwindow* window) {
    glfwMakeContextCurrent(NULL);
    glfwMakeContextCurrent(window);

    glClearColor(cfg.backgroundColor.r, cfg.backgroundColor.g, cfg.backgroundColor.b, cfg.backgroundColor.a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void setMainWinTitle() {
    int r = mainWinTitle.countR;
    int v = stars.size();

    // r should never be less than countL here if v is never more than 4 digits
    for (; v > 0; r--) {
        mainWinTitle.data[r] = v % 10 + '0';
        v /= 10;
    }
    for (; r >= mainWinTitle.countL; r--) {
        mainWinTitle.data[r] = ' ';
    }

    r = mainWinTitle.fpsR;
    v = ceil(1.0 / frameTime);

    // r should never be less than fpsL here if v is never more than 4 digits
    for (; v > 0; r--) {
        mainWinTitle.data[r] = v % 10 + '0';
        v /= 10;
    }
    for (; r >= mainWinTitle.fpsL; r--) {
        mainWinTitle.data[r] = ' ';
    }

    glfwSetWindowTitle(win.main.glfw, mainWinTitle.data);
}

// GLFW callbacks

static void errorCallback(int error, const char* description) {
    std::cerr << "GLFW error " << error << ": " << description << '\n';
}

void mainWinKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_A:
                clearWin(win.main.glfw);
                break;
            case GLFW_KEY_S:
                cfg.clear ^= true;
                break;
            case GLFW_KEY_D:
                createDestroyCfgWin();
                break;
            case GLFW_KEY_F:
                maximizeRestoreWin(window);
                break;
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(window, 1);
                break;
        }
    }
}

void mainWinPosCallback(GLFWwindow* window, int x, int y) {
    win.main.x = x;
    win.main.y = y;
}

void mainWinSizeCallback(GLFWwindow* window, int w, int h) {
    win.main.w = w;
    win.main.h = h;
}

void cfgWinKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (win.cfg.io->WantCaptureKeyboard) {
        if (win.cfg.exists) {
            ImGui_ImplGlfw_KeyCallback(win.cfg.glfw, key, scancode, action, mods);
        }
    } else if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_A:
                clearWin(win.main.glfw);
                break;
            case GLFW_KEY_S:
                cfg.clear ^= true;
                break;
            case GLFW_KEY_D:
                createDestroyCfgWin();
                break;
            case GLFW_KEY_F:
                maximizeRestoreWin(window);
                break;
            case GLFW_KEY_ESCAPE:
                destroyCfgWin();
                break;
        }
    }
}

void cfgWinPosCallback(GLFWwindow* window, int x, int y) {
    win.cfg.x = x;
    win.cfg.y = y;
}

void cfgWinSizeCallback(GLFWwindow* window, int w, int h) {
    win.cfg.w = w;
    win.cfg.h = h;
}

void cfgWinCloseCallback(GLFWwindow* window) {
    destroyCfgWin();
}

// utility

// defines legal characters for the config save file names entered via an ImGui InputText
int cfgNameImGuiInputTextFilter(ImGuiInputTextCallbackData* data) {
    ImWchar c = data->EventChar;

    return ((c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') ||
            (c == '.') ||
            (c == '_') ||
            (c == '-')) == 0;
}