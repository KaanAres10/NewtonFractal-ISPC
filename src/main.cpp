#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <chrono>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// ISPC
#include "newton_ispc.h"

struct FractalParams {
    int width = 1024;
    int height = 1024;
    int n = 5;
    int max_iter = 60;
};

struct GLImage {
    GLuint tex = 0;
    int w = 0, h = 0;
};


static void checkGL(bool cond, const char* msg) {
    if (!cond) { fprintf(stderr, "GL error: %s\n", msg); std::exit(1); }
}

static void createOrResizeTexture(GLImage& img, int w, int h) {
    if (img.tex == 0) glGenTextures(1, &img.tex);
    if (img.w == w && img.h == h) return;

    img.w = w; img.h = h;
    glBindTexture(GL_TEXTURE_2D, img.tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);
}

static void uploadImage(GLImage& img, const std::vector<uint32_t>& rgba) {
    glBindTexture(GL_TEXTURE_2D, img.tex);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, img.w, img.h,
        GL_RGBA, GL_UNSIGNED_BYTE, rgba.data());
    glBindTexture(GL_TEXTURE_2D, 0);
}

static void renderFractalCpu(const FractalParams& p, std::vector<uint32_t>& rgba) {
    rgba.resize(size_t(p.width) * size_t(p.height));
    ispc::render(p.width, p.height, p.n, p.max_iter, rgba.data());
}

static GLFWwindow* initWindow(int w, int h, const char* title) {
    if (!glfwInit()) { std::fprintf(stderr, "glfwInit failed\n"); return nullptr; }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    GLFWwindow* win = glfwCreateWindow(w, h, title, nullptr, nullptr);
    if (!win) { std::fprintf(stderr, "glfwCreateWindow failed\n"); glfwTerminate(); return nullptr; }
    glfwMakeContextCurrent(win);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::fprintf(stderr, "gladLoadGLLoader failed\n");
        glfwDestroyWindow(win);
        glfwTerminate();
        return nullptr;
    }
    return win;
}


int main() {
    // Window
    if (!glfwInit()) { fprintf(stderr, "glfwInit failed\n"); return 1; }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    GLFWwindow* win = glfwCreateWindow(1280, 900, "Newton Fractal (ISPC + ImGui)", nullptr, nullptr);
    if (!win) { fprintf(stderr, "glfwCreateWindow failed\n"); glfwTerminate(); return 1; }
    glfwMakeContextCurrent(win);
    glfwSwapInterval(1); 

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        fprintf(stderr, "gladLoadGLLoader failed\n"); return 1;
    }

    // ImGui setup
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(win, true);
    ImGui_ImplOpenGL3_Init("#version 330");


    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(3.50f);

    ImGuiIO& io = ImGui::GetIO();
    io.FontGlobalScale = 1.4f;

    // state
    FractalParams params;
    GLImage glimg{};
    std::vector<uint32_t> rgba;
    bool needs_render = true;
    bool auto_render = false;         
    bool dragging = false;
    ImVec2 lastMouse{};

    // initial texture
    createOrResizeTexture(glimg, params.width, params.height);

    while (!glfwWindowShouldClose(win)) {
        glfwPollEvents();

        // start imgui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        const float rightPaneWidth = 700.0f;

        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Once);
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize, ImGuiCond_Always);
        ImGuiWindowFlags winFlags = ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoSavedSettings;
        ImGui::Begin("Newton Fractal", nullptr, winFlags);

        // Compute sizes
        ImVec2 avail = ImGui::GetContentRegionAvail();
        float leftWidth = std::max(50.0f, avail.x - rightPaneWidth);
        float leftHeight = avail.y;

        ImGui::BeginChild("ViewPane", ImVec2(leftWidth, leftHeight), true);

        // Re-render when needed
        if (auto_render) {
            createOrResizeTexture(glimg, params.width, params.height);
            auto t0 = std::chrono::high_resolution_clock::now();
            renderFractalCpu(params, rgba);
            auto t1 = std::chrono::high_resolution_clock::now();
            double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
            uploadImage(glimg, rgba);
            char title[128];
            std::snprintf(title, sizeof(title), "Newton Fractal [%.2f ms]", ms);
            glfwSetWindowTitle(win, title);
        }
        else if (needs_render) {
            createOrResizeTexture(glimg, params.width, params.height);
            auto t0 = std::chrono::high_resolution_clock::now();
            renderFractalCpu(params, rgba);
            auto t1 = std::chrono::high_resolution_clock::now();
            double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
            uploadImage(glimg, rgba);
            needs_render = false;
            char title[128];
            std::snprintf(title, sizeof(title), "Newton Fractal [%.2f ms]", ms);
            glfwSetWindowTitle(win, title);
        }

        // Draw image
        ImVec2 leftAvail = ImGui::GetContentRegionAvail();
        float imgSz = std::min(leftAvail.x, leftAvail.y);
        ImVec2 imgSize(imgSz, imgSz);
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (leftAvail.x - imgSz) * 0.5f);
        ImGui::Image((ImTextureID)(intptr_t)glimg.tex, imgSize, ImVec2(0, 1), ImVec2(1, 0)); // flip V

        ImGui::EndChild();

        // Controls
        ImGui::SameLine();
        ImGui::BeginChild("ControlsPane", ImVec2(0, leftHeight), true); 

        ImGui::TextUnformatted("Controls");
        ImGui::Separator();

        ImGui::Checkbox("Auto render (each frame)", &auto_render);

        // Resolution
        if (ImGui::SliderInt("Resolution", &params.width, 256, 4096)) {
            params.height = params.width;
            if (auto_render && ImGui::IsItemDeactivatedAfterEdit());
        }

        // n (power)
        if (ImGui::SliderInt("n (power)", &params.n, 2, 30)) {
            if (auto_render && ImGui::IsItemDeactivatedAfterEdit());
        }

        // max_iter
        if (ImGui::SliderInt("max_iter", &params.max_iter, 3, 200)) {
            if (auto_render && ImGui::IsItemDeactivatedAfterEdit());
        }

        // When Auto is OFF, render only on button press:
        if (!auto_render) {
            if (ImGui::Button("Render Now")) needs_render = true;
        }

        ImGui::Separator();
        ImGui::TextUnformatted("Note");
        ImGui::BulletText("Click Above the Render Now (When Auto Render Disabled)");

        ImGui::EndChild();

        ImGui::End(); 


        // render imgui
        ImGui::Render();
        int fbw, fbh;
        glfwGetFramebufferSize(win, &fbw, &fbh);
        glViewport(0, 0, fbw, fbh);
        glClearColor(0.08f, 0.08f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(win);
    }

    // cleanup
    if (glimg.tex) glDeleteTextures(1, &glimg.tex);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(win);
    glfwTerminate();
    return 0;
}
