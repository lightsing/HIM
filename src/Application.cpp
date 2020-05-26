//
// Created by light on 5/6/2020.
//

#include "Application.h"

static string model_list[] = { "stone", "dirt", "bedrock" };

Application::Application(const char *title, int width, int height) {
    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (window == nullptr) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(-1);
    }
    glfwMakeContextCurrent(window);
    CallbackWrapper::setApplication(this);
    glfwSetFramebufferSizeCallback(window, CallbackWrapper::framebufferSizeCallback);
    glfwSetCursorPosCallback(window, CallbackWrapper::mouseCallback);
    glfwSetKeyCallback(window, CallbackWrapper::keyboardCallback);
    glfwSetScrollCallback(window, CallbackWrapper::scrollCallback);
    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        exit(-1);
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
//    stbi_set_flip_vertically_on_load(true);

    // configure global openGL state
    glEnable(GL_DEPTH_TEST);

    m_window = window;
    this->width = width;
    this->height = height;
    lastX = width / 2.f;
    lastY = height / 2.f;
    deltaTime = 0.f;
    lastFrame = 0.f;
    camera = Camera(
            glm::vec3(4.0f, 3.0f, 3.0f),
            glm::vec3(0.0f, 1.0f, 0.0f),
            glm::vec3(0.0f, 0.0f, 0.0f)
            );
    ourShader = new Shader("res/shader.vs", "res/shader.fs");
    models = new map<string, Model>;
    for(const string& key : model_list) {
        models->insert(pair<string, Model>(key, Model("res/assets/" + key + ".obj")));
    }
    maze = new Maze(17, 19);
    maze->print_maze();

}

void Application::preRender() {
    // per-frame time logic
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    processInput(); // input

    glClearColor(1.f, 1.f, 1.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Application::render() {
    // render
    // ------
    // don't forget to enable shader before setting uniforms
    ourShader->use();

    // view/projection transformations
    glm::mat4 projection = glm::perspective(glm::radians(camera.fov), (float) width / (float) height,
                                            camera.zNear, camera.zFar);
    glm::mat4 view = camera.getViewMatrix();
    ourShader->setMat4("projection", projection);
    ourShader->setMat4("view", view);

    for(int i = -10; i < maze->get_col_num() + 10; ++i)
        for(int j = -10; j < maze->get_row_num() + 10; ++j) {
            // render the loaded model
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model,
                                   glm::vec3(i * 2., .0f, j * 2.)); // translate it down so it's at the center of the scene
            model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));    // it's a bit too big for our scene, so scale it down
            ourShader->setMat4("model", model);
            models->at("bedrock").Draw(*ourShader);
        }
    for(int i = 0; i < maze->get_col_num(); ++i)
        for(int j = 0; j < maze->get_row_num(); ++j) {
            if (!maze->isWall(i, j)) continue;
            for(int _ = 1; _ < 6; ++_) {
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model,
                                       glm::vec3(i * 2., _ * 2., j * 2.)); // translate it down so it's at the center of the scene
                model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));    // it's a bit too big for our scene, so scale it down
                ourShader->setMat4("model", model);
                models->at("stone").Draw(*ourShader);
            }
        }

}

void Application::postRender() {
    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    glfwSwapBuffers(m_window);
    glfwPollEvents();
}

void Application::processInput() {
    if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(m_window, true);
    // keyboard movement
    if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS)
        camera.moveAround(CameraMovement::FORWARD, deltaTime);
    if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS)
        camera.moveAround(CameraMovement::BACKWARD, deltaTime);
    if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS)
        camera.moveAround(CameraMovement::LEFT, deltaTime);
    if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS)
        camera.moveAround(CameraMovement::RIGHT, deltaTime);
    if (glfwGetKey(m_window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.moveAround(CameraMovement::UP, deltaTime);
    if (glfwGetKey(m_window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        camera.moveAround(CameraMovement::DOWN, deltaTime);
    // change moving speed
    if (glfwGetKey(m_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.changeSpeed(SPEED_FAST_DEFAULT);
    else {
        camera.changeSpeed(SPEED_NORMAL_DEFAULT);
    }
}

void Application::mouseCallback(double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.lookAround(xoffset, yoffset);
}

void Application::keyboardCallback(int key, int scancode, int action, int mods) {

}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
void Application::scrollCallback(double xoffset, double yoffset) {
    camera.zoom(yoffset);
}

void Application::framebufferSizeCallback(int width, int height) {
    this->width = width;
    this->height = height;
    lastX = width / 2.f;
    lastY = height / 2.f;
    glViewport(0, 0, width, height);
}

void Application::CallbackWrapper::mouseCallback(GLFWwindow *window, double positionX, double positionY) {
    s_application->mouseCallback(positionX, positionY);
}

void Application::CallbackWrapper::keyboardCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    s_application->keyboardCallback(key, scancode, action, mods);
}

void Application::CallbackWrapper::scrollCallback(GLFWwindow *window, double xoffset, double yoffset) {
    s_application->scrollCallback(xoffset, yoffset);
}

void Application::CallbackWrapper::framebufferSizeCallback(GLFWwindow *window, int width, int height) {
    s_application->framebufferSizeCallback(width, height);
}


void Application::CallbackWrapper::setApplication(Application *application) {
    CallbackWrapper::s_application = application;
}

Application *Application::CallbackWrapper::s_application = nullptr;