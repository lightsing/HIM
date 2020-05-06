//
// Created by light on 5/5/2020.
//

#pragma once

#include <GLFW/glfw3.h>
#include <learnopengl/camera.h>

// Proxy methods to GLFW
class Window {
public:
    Window(const char *title, int width, int height);

    void postRender(); // swap buffers and poll IO events
    bool shouldClose(); // glfwWindowShouldClose
    void swapBuffers(); // glfwSwapBuffers
    static void pollEvents(); // glfwPollEvents
    static int terminate(); // glfwTerminate
    GLFWwindow *getWindow() const {
        return window;
    }

private:
    GLFWwindow *window;

    static void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
        glViewport(0, 0, width, height);
    }
};