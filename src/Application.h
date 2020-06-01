//
// Created by light on 5/6/2020.
//

#pragma once

#include <iostream>
#include <stb_image.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <learnopengl/camera.h>
#include <learnopengl/shader.h>
#include <learnopengl/model.h>
#include <sstream>

#include "maze.h"
#include "text.h"

struct CubeModel {
    string type;
    glm::vec3 position;
    glm::mat4 model;
    glm::mat3 model_res;
};

class Application {
public:
    Application() = delete;

    Application(const char *title, int width, int height, int map_size, int maze_length, int maze_width,
                bool debug = false);

    static void init() {
        // glfw: initialize and configure
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    }

    void preRender();

    void render();

    void renderObject(Shader *);

    void renderLight(glm::vec3);

    void postRender();

    bool shouldClose() { return glfwWindowShouldClose(m_window); }

    bool alive() { return !shouldClose(); }

    int terminate() {
        glfwDestroyWindow(m_window);
        glfwTerminate();
        return 0;
    }

private:
    CubeModel **floor_model;
    CubeModel ***wall_model;

    GLFWwindow *m_window;
    GLFWmonitor *m_monitor;
    int width, height;
    GLuint SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

    float lastX, lastY;
    float deltaTime, lastFrame;
    bool firstMouse = true;

    int map_sz;

    int maze_len, maze_wid;

    bool adventurer_handle = true;
    bool bindAdventurer = true;
    Camera camera_adventurer = Camera(CAM_POS_DEFAULT, WORLD_UP_DEFAULT, TARGET_POS_DEFAULT, true);
    Camera camera_uav = Camera(CAM_POS_DEFAULT, WORLD_UP_DEFAULT, TARGET_POS_DEFAULT, false);
    Camera *camera;
    Model *characterBallAdv;
    Model *characterBallUav;

    GLuint depthMapFBO;
    GLuint depthCubeMap;

    FreeType *freeType;

    double startTime = 0.0;
    double endTime = 0.0;
    double gameTime = 0.0;
    double preTime = 0.0;
    double notAdvTime = 0.0;

    Shader *lightCubeShader, *objShader, *depthShader;

    map<string, Model> *models;

    Maze *maze;

    int gameState;  // 0: free, 1: playing, 2: finished

    bool debug;
    bool shadows = true;

    bool reachReg(glm::vec3 cen1, glm::vec3 cen2);

    void processInput();

    void framebufferSizeCallback(int width, int height);

    void mouseCallback(double positionX, double positionY);

    void keyboardCallback(int key, int scancode, int action, int mods);

    void scrollCallback(double xoffset, double yoffset);

    void centerWindow();

    GLFWmonitor *getBestMonitor();

    class CallbackWrapper {
    public:
        CallbackWrapper() = delete;

        CallbackWrapper(const CallbackWrapper &) = delete;

        CallbackWrapper(CallbackWrapper &&) = delete;

        ~CallbackWrapper() = delete;

        static void framebufferSizeCallback(GLFWwindow *window, int width, int height);

        static void mouseCallback(GLFWwindow *window, double positionX, double positionY);

        static void keyboardCallback(GLFWwindow *window, int key, int scancode, int action, int mods);

        static void scrollCallback(GLFWwindow *window, double xoffset, double yoffset);

        static void setApplication(Application *application);

    private:
        static Application *s_application;
    };

};
