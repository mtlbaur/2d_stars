// #include <iostream>
// #include <iomanip>

// #include <algorithm>
// #include <memory>
// #include <cmath>

#include <chrono>
#include <random>
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

#include "Star.h"

using namespace std;

const int SCREEN_SIZE_X = 1920;
const int SCREEN_SIZE_Y = 1080;
const double SCREEN_SIZE_MULT = 0.75;
const int GLFW_CONTEXT_VER_MAJOR = 4;
const int GLFW_CONTEXT_VER_MINOR = 6;
const char* GLSL_VER = "#version 130";
// If FPS goes lower than 30, don't attempt to keep star movement/physics consistent anymore.
// This also prevents long program pauses (== high frameTime values) from causing excessively incorrect behavior (e.g. stars teleporting).
// The drawback is slowdown when FPS falls below 30.
const double FRAME_TIME_MAX = 1.0 / 30.0;
const int MSAA_SAMPLES = 4;
const unsigned int MAX_STARS = 5000;

// config files path information
static const char CFG_PATH_USER[] = "./config/user/";
static const char CFG_PATH_DEFAULT[] = "./config/default/";
static const char CFG_EXTENSION[] = ".cfg";
static const char CFG_NAME_DEFAULT[] = "parameters";

// https://docs.gl/gl4/glBlendFunc
// Used to allow the user to pick whatever blending combination they want.
// Note that a lot of combinations will result in useless blending (invisible stars is one example).
static const int glBlendFunc_factor[] = {
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

static const char* glBlendFunc_factor_name[] = {
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

// global struct: contains various user-controlled settings
Config cfg;
// global struct: contains properties GLFW and ImGui windows
Windows win;
// global var: contains frame time used as a multiplier to make stars appear to move at the same speed regardless of FPS
double frameTime = 0.0;
// global var: used to generate random values
default_random_engine dre;

// contains all stars that appear on the main window
static vector<unique_ptr<Star>> stars;
// contains stars used in the preview of the config window
static struct {
    unique_ptr<Star> min;
    unique_ptr<Star> avg;
    unique_ptr<Star> max;

    void forceVisible() {
        if (min && avg && max) {
            min->color = Color(0.0f, 1.0f, 1.0f, 1.0f);
            avg->color = Color(1.0f, 1.0f, 0.0f, 1.0f);
            max->color = Color(1.0f, 0.0f, 1.0f, 1.0f);
        }
    }
} pre;

// config files
static vector<boost::filesystem::directory_entry> cfgFiles;
// config file name max length
static const unsigned int cfgFileNameBufferSize = 128;
// config file name characters
static char cfgFileNameBuffer[cfgFileNameBufferSize] = "";
// contains name of last used config (displayed in GUI)
static string cfgLastUsed = "";
// how many consecutive chars of space is available in mainWinTitle (filled with to_char)
static const int mainWinTitleSpace = 4;
// title of the main window
static char mainWinTitle[] = "    " // space to fill with star count
                             " stars; "
                             "    " // space to fill with FPS count
                             " FPS";

// main loop

void execute();

// Star

void addRemoveStars(int);
void regenStars(unsigned int);
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

void getCfgFiles(vector<boost::filesystem::directory_entry> &);
int cfgNameImGuiInputTextFilter(ImGuiInputTextCallbackData*);

int main() {
    dre.seed(std::chrono::system_clock::now().time_since_epoch().count());
    stars.reserve(MAX_STARS);

    glfwSetErrorCallback(errorCallback);

    if (!glfwInit()) {
        fprintf(stderr, "glfwInit() failed");
        exit(1);
    }

    cfg.load(CFG_PATH_DEFAULT, CFG_NAME_DEFAULT, CFG_EXTENSION, &cfgLastUsed);

    createMainWin();
    createCfgWin();

    execute();                     // main program loop
    addRemoveStars(-stars.size()); // correctly free the memory for all the existing stars before shutdown

    destroyCfgWin();
    destroyMainWin();

    glfwTerminate();

    cfg.save(CFG_PATH_DEFAULT, CFG_NAME_DEFAULT, CFG_EXTENSION);

    return 0;
}

void execute() {
    chrono::steady_clock::time_point updateTime = chrono::steady_clock::now();

    while (!glfwWindowShouldClose(win.main.glfw)) {
        chrono::steady_clock::time_point currentTime = chrono::steady_clock::now();
        chrono::duration<double> deltaTime = currentTime - updateTime; // seconds
        double targetSecondsPerFrame = 1.0 / (double)cfg.targetFPS;

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

            frameTime = min(frameTime, FRAME_TIME_MAX);

            // update and draw in OpenGL
            DrawableObject::prepareProjection(win.main.w, win.main.h);
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
        if (stars.size() + n > MAX_STARS) n = MAX_STARS - stars.size();

        for (int i = 0; i < n; i++) {
            stars.push_back(make_unique<Star>(RNG));
        }
    } else if (n < 0) {
        for (unsigned int i = ((unsigned int)-n < stars.size() ? -n : stars.size()); i > 0; i--) {
            stars.pop_back();
        }
    }
}

void regenStars(unsigned int n) {
    if (n == 0) return;

    glfwMakeContextCurrent(NULL);
    glfwMakeContextCurrent(win.main.glfw);

    addRemoveStars(-n);
    addRemoveStars(n);

    glClearColor(cfg.backgroundColor.r, cfg.backgroundColor.g, cfg.backgroundColor.b, cfg.backgroundColor.a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void updateStars() {
    if (cfg.collisions) {
        bool ithStarNotCollided;

        for (unsigned i = 0; i < stars.size(); i++) {
            if (stars[i]->notUpdated) {
                stars[i]->update();
                stars[i]->notUpdated = false;
            }

            ithStarNotCollided = true;

            for (unsigned j = i + 1; j < stars.size(); j++) {
                if (stars[i]->notUpdated) {
                    stars[i]->update();
                    stars[i]->notUpdated = false;
                }

                if (stars[i]->notCollided && stars[i]->notCollided && Star::collision(stars.at(i).get(), stars.at(j).get())) {
                    ithStarNotCollided = false;
                    break;
                }
            }

            stars[i]->notUpdated = true;
            stars[i]->notCollided = ithStarNotCollided;
            stars[i]->draw();
        }
    } else {
        for (unsigned i = 0; i < stars.size(); i++) {
            stars[i]->update();
            stars[i]->draw();
        }
    }

    DrawableObject::updateIndexUniformRGBColors();
}

// ImGui creation

void createGUI() {
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
                             ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoCollapse |
                             ImGuiWindowFlags_HorizontalScrollbar;

    const char* intFormat = "%d";
    const char* floatFormat = "%.3f";

    if (ImGui::Begin("Config", NULL, flags)) {
        if (ImGui::CollapsingHeader("Stars")) {
            ImGui::SliderInt("##addRemoveStars", &cfg.addRemove, -500, 500);

            unsigned int amount = 0;

            ImGui::SameLine();
            if (ImGui::Button("+-#")) amount += cfg.addRemove;

            if (ImGui::Button("+1")) amount += 1;

            ImGui::SameLine();
            if (ImGui::Button("-1")) amount += -1;

            ImGui::SameLine();
            if (ImGui::Button("Clear")) amount += -stars.size();

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

                // cout << glBlendFunc_factor[cfg.srcBlendMode] << ": " << glBlendFunc_factor_name[cfg.srcBlendMode] << endl
                //      << glBlendFunc_factor[cfg.dstBlendMode] << ": " << glBlendFunc_factor_name[cfg.dstBlendMode] << endl
                //      << endl;

                glfwMakeContextCurrent(NULL);
                glfwMakeContextCurrent(win.cfg.glfw);
            }

            ImGui::Separator();
            ImGui::Combo("Color Mode", &cfg.colorMode, "Default\0Random\0Uniform\0Consistent\0");
            ImGui::SliderFloat("Color Shift Mult", &cfg.colorShiftMult, -1000, 1000, floatFormat, ImGuiSliderFlags_NoRoundToFormat);

            ImGui::Separator();
            ImGui::ColorEdit4("Background Color", (float*)&cfg.backgroundColor);
        }
        if (ImGui::CollapsingHeader("Physics")) {
            ImGui::Checkbox("Collisions", &cfg.collisions);

            ImGui::Separator();
            ImGui::SliderFloat("##gravityMult", &cfg.gravityMult, -10000, 10000, floatFormat, ImGuiSliderFlags_NoRoundToFormat);

            ImGui::SameLine();
            ImGui::Checkbox("Gravity", &cfg.gravity);

            ImGui::Separator();
            ImGui::SliderFloat("##accelMult", &cfg.accelMult, 0, 2, "%.6f", ImGuiSliderFlags_NoRoundToFormat | ImGuiSliderFlags_AlwaysClamp);

            ImGui::SameLine();
            ImGui::Checkbox("Accel", &cfg.accel);

            ImGui::Separator();
            ImGui::SliderFloat("##minSpeedLimit", &cfg.minSpeedLimit, 0, cfg.maxSpeedLimit, floatFormat, ImGuiSliderFlags_NoRoundToFormat | ImGuiSliderFlags_AlwaysClamp);

            ImGui::SameLine();
            ImGui::Checkbox("Min Speed", &cfg.minSpeed);

            ImGui::Separator();
            ImGui::SliderFloat("##maxSpeedLimit", &cfg.maxSpeedLimit, cfg.minSpeedLimit, 1000, floatFormat, ImGuiSliderFlags_NoRoundToFormat | ImGuiSliderFlags_AlwaysClamp);

            ImGui::SameLine();
            ImGui::Checkbox("Max Speed", &cfg.maxSpeed);
        }
        if (ImGui::CollapsingHeader("Parameters")) {
            if (ImGui::Button("Apply")) {
                regenStars(stars.size());

                glfwMakeContextCurrent(NULL);
                glfwMakeContextCurrent(win.cfg.glfw);
            }

            ImGui::SameLine();
            if (ImGui::Button("Save")) {
                getCfgFiles(cfgFiles);
                ImGui::OpenPopup("Save Config");
            }

            int count = cfgFiles.size();

            // BEGIN SAVE POPUP
            bool p_open_save = true;

            if (ImGui::BeginPopupModal("Save Config", &p_open_save, ImGuiWindowFlags_AlwaysAutoResize)) {
                static unsigned int cfgSaveSelectedIdx = 0;

                if (ImGui::BeginListBox("##fileNames")) {
                    for (unsigned int i = 0; i < cfgFiles.size(); i++) {
                        count -= 1;

                        const bool selected = (cfgSaveSelectedIdx == i);

                        if (ImGui::Selectable(cfgFiles[i].path().stem().string().c_str(), selected)) {
                            cfgSaveSelectedIdx = i;
                            memcpy(cfgFileNameBuffer, cfgFiles[i].path().stem().string().c_str(), cfgFileNameBufferSize);
                        }

                        if (selected) ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndListBox();
                }

                bool save = ImGui::Button("Save");

                ImGui::SameLine();
                ImGui::InputText("Name", cfgFileNameBuffer, cfgFileNameBufferSize, ImGuiInputTextFlags_CallbackCharFilter, cfgNameImGuiInputTextFilter);

                if (save) {
                    ImGui::CloseCurrentPopup();
                    cfg.save(CFG_PATH_USER, cfgFileNameBuffer, CFG_EXTENSION, &cfgLastUsed);
                    memset(cfgFileNameBuffer, '\0', cfgFileNameBufferSize);
                    getCfgFiles(cfgFiles);
                }

                if (ImGui::Button("Delete") && cfgFiles.size() > 0 && cfgSaveSelectedIdx < cfgFiles.size()) {
                    boost::filesystem::remove(cfgFiles[cfgSaveSelectedIdx].path());
                    cfgSaveSelectedIdx = 0;
                    getCfgFiles(cfgFiles);
                }

                ImGui::EndPopup();
            }
            // END SAVE POPUP

            bool reloadPreview = false;

            ImGui::SameLine();
            if (ImGui::Button("Load")) {
                getCfgFiles(cfgFiles);
                ImGui::OpenPopup("Load Config");
            }

            // BEGIN LOAD POPUP
            bool p_open_load = true;

            if (ImGui::BeginPopupModal("Load Config", &p_open_load, ImGuiWindowFlags_AlwaysAutoResize)) {
                bool load = false;
                static unsigned int cfgLoadSelectedIdx = 0;

                if (ImGui::BeginListBox("##fileNames")) {
                    for (unsigned int i = 0; i < cfgFiles.size(); i++) {
                        count -= 1;

                        const bool selected = (cfgLoadSelectedIdx == i);

                        if (ImGui::Selectable(cfgFiles[i].path().stem().string().c_str(), selected)) {
                            load = true;
                            reloadPreview |= true;
                            cfgLoadSelectedIdx = i;
                            cfg.load(CFG_PATH_USER, cfgFiles[i].path().stem().string(), CFG_EXTENSION, &cfgLastUsed);

                            regenStars(stars.size());

                            glfwMakeContextCurrent(NULL);
                            glfwMakeContextCurrent(win.cfg.glfw);
                        }

                        if (selected) ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndListBox();
                }

                if (load) ImGui::CloseCurrentPopup();

                ImGui::EndPopup();
            }
            // END LOAD POPUP

            ImGui::SameLine();
            if (ImGui::Button("Reset")) {
                reloadPreview |= true;
                cfgLastUsed = "RESET";
                cfg.reset();

                regenStars(stars.size());

                glfwMakeContextCurrent(NULL);
                glfwMakeContextCurrent(win.cfg.glfw);
            }

            ImGui::SameLine();
            ImGui::LabelText("##configOptions", cfgLastUsed.c_str());

            ImGui::Separator();
            ImGui::SliderInt("Target FPS", &cfg.targetFPS, 60, 1000, intFormat, ImGuiSliderFlags_AlwaysClamp);

            displayPreview(displayGenerationParameters() | reloadPreview);
        }
    }

    ImGui::End();
}

bool displayGenerationParameters() {
    const char* intFormat = "%d";
    const char* floatFormat = "%.3f";
    const double steps = 500.0;

    bool adjusted = false;

    ImGui::Separator();
    adjusted |= ImGui::DragIntRange2("Tips", &cfg.min.tips, &cfg.max.tips, cfg.limMax.tips / steps, cfg.limMin.tips, cfg.limMax.tips, intFormat, intFormat, ImGuiSliderFlags_AlwaysClamp);

    ImGui::Separator();
    adjusted |= ImGui::DragFloatRange2("x Velocity", &cfg.min.xVel, &cfg.max.xVel, cfg.limMax.xVel / steps, cfg.limMin.xVel, cfg.limMax.xVel, floatFormat, floatFormat, ImGuiSliderFlags_AlwaysClamp);

    ImGui::Separator();
    adjusted |= ImGui::DragFloatRange2("y Velocity", &cfg.min.yVel, &cfg.max.yVel, cfg.limMax.yVel / steps, cfg.limMin.yVel, cfg.limMax.yVel, floatFormat, floatFormat, ImGuiSliderFlags_AlwaysClamp);

    ImGui::Separator();
    adjusted |= ImGui::DragFloatRange2("Angle", &cfg.min.ang, &cfg.max.ang, cfg.limMax.ang / steps, cfg.limMin.ang, cfg.limMax.ang, floatFormat, floatFormat, ImGuiSliderFlags_AlwaysClamp);

    ImGui::Separator();
    adjusted |= ImGui::DragFloatRange2("Angular Velocity", &cfg.min.angVel, &cfg.max.angVel, cfg.limMax.angVel / steps, cfg.limMin.angVel, cfg.limMax.angVel, floatFormat, floatFormat, ImGuiSliderFlags_AlwaysClamp);

    ImGui::Separator();
    adjusted |= ImGui::DragFloatRange2("Inner Radius", &cfg.min.iRadius, &cfg.max.iRadius, cfg.limMax.iRadius / steps, cfg.limMin.iRadius, cfg.limMax.iRadius, floatFormat, floatFormat, ImGuiSliderFlags_AlwaysClamp);

    ImGui::Separator();
    adjusted |= ImGui::DragFloatRange2("Outer Radius", &cfg.min.oRadius, &cfg.max.oRadius, cfg.limMax.oRadius / steps, cfg.limMin.oRadius, cfg.limMax.oRadius, floatFormat, floatFormat, ImGuiSliderFlags_AlwaysClamp);

    ImGui::Separator();
    adjusted |= ImGui::DragFloatRange2("Density", &cfg.min.density, &cfg.max.density, cfg.limMax.density / steps, cfg.limMin.density, cfg.limMax.density, floatFormat, floatFormat, ImGuiSliderFlags_AlwaysClamp);

    ImGui::Separator();
    adjusted |= ImGui::DragFloatRange2("Red", &cfg.min.color.r, &cfg.max.color.r, cfg.limMax.color.r / steps, cfg.limMin.color.r, cfg.limMax.color.r, floatFormat, floatFormat, ImGuiSliderFlags_AlwaysClamp);

    ImGui::Separator();
    adjusted |= ImGui::DragFloatRange2("Green", &cfg.min.color.g, &cfg.max.color.g, cfg.limMax.color.g / steps, cfg.limMin.color.g, cfg.limMax.color.g, floatFormat, floatFormat, ImGuiSliderFlags_AlwaysClamp);

    ImGui::Separator();
    adjusted |= ImGui::DragFloatRange2("Blue", &cfg.min.color.b, &cfg.max.color.b, cfg.limMax.color.b / steps, cfg.limMin.color.b, cfg.limMax.color.b, floatFormat, floatFormat, ImGuiSliderFlags_AlwaysClamp);

    ImGui::Separator();
    adjusted |= ImGui::DragFloatRange2("Alpha", &cfg.min.color.a, &cfg.max.color.a, cfg.limMax.color.a / steps, cfg.limMin.color.a, cfg.limMax.color.a, floatFormat, floatFormat, ImGuiSliderFlags_AlwaysClamp);

    ImGui::Separator();
    adjusted |= ImGui::SliderFloat("Min Color", &cfg.minColor, 0.0f, 1.0f, floatFormat, ImGuiSliderFlags_AlwaysClamp);

    ImGui::Separator();
    adjusted |= ImGui::Checkbox("Fill Core", &cfg.style.fillCore);
    ImGui::SameLine();
    adjusted |= ImGui::Checkbox("Hollow Core", &cfg.style.hollowCore);

    ImGui::Separator();
    adjusted |= ImGui::Checkbox("Fill Draw", &cfg.style.fillDraw);
    ImGui::SameLine();
    adjusted |= ImGui::Checkbox("Line Draw", &cfg.style.lineDraw);

    ImGui::Separator();
    adjusted |= ImGui::Checkbox("Force Visible Preview", &cfg.forceVisiblePreview);

    return adjusted;
}

void displayPreview(bool adjusted) {
    // if any settings were changed in the last frame, construct new preview stars so that those settings are applied
    if (adjusted) {
        pre.min = make_unique<Star>(MIN);
        pre.avg = make_unique<Star>(AVG);
        pre.max = make_unique<Star>(MAX);

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

        DrawableObject::prepareProjection(w, h);

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
        fprintf(stderr, "gladLoadGL(glfwGetProcAddress) failed");
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

    win.main.glfw = NULL;
    win.main.imgui = NULL;
    win.main.exists = false;
}

void createCfgWin() {
    if (win.cfg.exists) return;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GLFW_CONTEXT_VER_MAJOR);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GLFW_CONTEXT_VER_MINOR);
    // glfwWindowHint(GLFW_SAMPLES, 4);

    win.cfg.w = 800;
    win.cfg.h = 600;
    win.cfg.x = win.main.x + win.main.w - win.cfg.w;
    win.cfg.y = win.main.y;

    win.cfg.glfw = glfwCreateWindow(win.cfg.w, win.cfg.h, "Config", NULL, NULL);
    if (!win.cfg.glfw) exit(1);

    glfwSetWindowPos(win.cfg.glfw, win.cfg.x, win.cfg.y);

    glfwMakeContextCurrent(NULL);
    glfwMakeContextCurrent(win.cfg.glfw);
    if (!gladLoadGL(glfwGetProcAddress)) {
        fprintf(stderr, "gladLoadGL(glfwGetProcAddress) failed");
        exit(1);
    }

    // glEnable(GL_MULTISAMPLE);

    glGenFramebuffers(1, &win.cfg.frameBuffer);
    glGenTextures(1, &win.cfg.texture);

    // clang-format off
    // This texture is used for the preview window.
    // ImGui needs requires a texture to be able to add a custom image to an ImGui window, so that's what is being created here.
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

    win.cfg.io = &ImGui::GetIO();
    win.cfg.io->IniFilename = NULL; // disable useless (for this window) ImGui ini file
    // win.cfg.io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    // win.cfg.io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

    ImGui_ImplGlfw_InitForOpenGL(win.cfg.glfw, false); // second param == false --> do not auto install callbacks because they are already defined above
    ImGui_ImplOpenGL3_Init(GLSL_VER);

    pre.min = make_unique<Star>(MIN);
    pre.avg = make_unique<Star>(AVG);
    pre.max = make_unique<Star>(MAX);

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

    win.cfg.glfw = NULL;
    win.cfg.imgui = NULL;
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
    // old title
    // glfwSetWindowTitle(win.main.glfw,
    //                    (to_string(stars.size()) + " stars; " +
    //                     to_string(1.0 / frameTime) + " FPS; " +
    //                     to_string(frameTime) + " frameTime")
    //                        .c_str());

    // note: mingw64 with C++17 currently doesn't support floating point types for to_chars so the title uses integer types instead

    int i = 0;

    for (int j = i + mainWinTitleSpace; i < j; i++) {
        mainWinTitle[i] = ' ';
    }

    to_chars(mainWinTitle,
             mainWinTitle + mainWinTitleSpace * sizeof(char), stars.size());

    i += 8;

    for (int j = i + mainWinTitleSpace; i < j; i++) {
        mainWinTitle[i] = ' ';
    }

    to_chars(mainWinTitle + (i - mainWinTitleSpace) * sizeof(char),
             mainWinTitle + i * sizeof(char), (unsigned int)(ceil(1.0 / frameTime)));

    glfwSetWindowTitle(win.main.glfw, mainWinTitle);
}

// GLFW callbacks

static void errorCallback(int error, const char* description) {
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

void mainWinKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_A && action == GLFW_PRESS) {
        cfg.clear ^= true;
    } else if (key == GLFW_KEY_S && action == GLFW_PRESS) {
        clearWin(win.main.glfw);
    } else if (key == GLFW_KEY_D && action == GLFW_PRESS) {
        createDestroyCfgWin();
    } else if (key == GLFW_KEY_F && action == GLFW_PRESS) {
        maximizeRestoreWin(window);
    } else if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, 1);
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
        // this exists check might be redundant....
        if (win.cfg.exists) ImGui_ImplGlfw_KeyCallback(win.cfg.glfw, key, scancode, action, mods);
    } else {
        if (key == GLFW_KEY_A && action == GLFW_PRESS) {
            cfg.clear ^= true;
        } else if (key == GLFW_KEY_S && action == GLFW_PRESS) {
            clearWin(win.main.glfw);
        } else if (key == GLFW_KEY_D && action == GLFW_PRESS) {
            createDestroyCfgWin();
        } else if (key == GLFW_KEY_F && action == GLFW_PRESS) {
            maximizeRestoreWin(window);
        } else if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            destroyCfgWin();
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

void getCfgFiles(vector<boost::filesystem::directory_entry> &outNames) {
    using namespace boost::filesystem;

    try {
        outNames.clear();

        path p(CFG_PATH_USER);

        if (exists(p) && is_directory(p)) {

            for (directory_entry &f : directory_iterator(p)) {
                if (f.path().extension() == CFG_EXTENSION) outNames.push_back(f);
            }
        }
    } catch (exception &e) {
        fprintf(stderr, "EXCEPTION: getCfgFiles(): READ: %s\n", e.what());
    }
}

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