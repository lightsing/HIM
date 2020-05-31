//
// Created by light on 5/6/2020.
//

#include "Application.h"

static string model_list[] = {"stone", "dirt", "bedrock"};

Application::Application(const char *title, int width, int height, int map_size, int maze_length, int maze_width,
                         bool debug) {
    this->debug = debug;

    // glfw window creation
    // --------------------
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    if (debug) {
        monitor = nullptr;
    }
    GLFWwindow *window = glfwCreateWindow(width, height, title, monitor, nullptr);
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

    map_sz = map_size;

    maze_len = maze_length;
    maze_wid = maze_width;

    // initial camera positions
    adventurer_handle = true;
    camera_adventurer = Camera(
            glm::vec3(2.0f, 1.85f, -20.0f),
            glm::vec3(0.0f, 1.0f, 0.0f),
            glm::vec3(0.0f, 0.0f, 0.0f),
            true
    );
    camera_uav = Camera(
            camera_adventurer.position + glm::vec3(0, 12., 0),
            camera_adventurer.worldUp,
            camera_adventurer.position,
            false
    );
    camera = &camera_adventurer;

//    ourShader = new Shader("res/shader.vs", "res/shader.fs");
    lightCubeShader = new Shader("res/lightShader.vs", "res/lightShader.fs");
    objShader = new Shader("res/objShader.vs", "res/objShader.fs");

    // Store the maze
    models = new map<string, Model>;
    for (const string &key : model_list) {
        models->insert(pair<string, Model>(key, Model("res/assets/" + key + ".obj")));
    }
    maze = new Maze(this->maze_len, this->maze_wid);
    maze->print_maze();

    gameState = 0;
}

void Application::preRender() {
    // per-frame time logic
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    processInput(); // input

    glClearColor(.0f, .0f, .0f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Application::render() {
    // render
    // ------
    // update uav's position with the adventurer
    if (!debug && adventurer_handle) {
        camera_uav.position = camera_adventurer.position + glm::vec3(-1., 12., -1.);
    }

    // don't forget to enable shader before setting uniforms
    objShader->use();
    objShader->setInt("material.diffuse", 0);
    objShader->setInt("material.specular", 1);

    // view/projection transformations
    camera = adventurer_handle ? &camera_adventurer : &camera_uav;
    glm::mat4 projection = glm::perspective(glm::radians(camera->fov), (float) width / (float) height,
                                            camera->zNear, camera->zFar);
    glm::mat4 view = camera->getViewMatrix();
    objShader->setVec3("light.position",
                       glm::vec3(camera_uav.position.x, camera_uav.position.y + 1.0f, camera_uav.position.z));
    objShader->setVec3("viewPos", camera->position);

    objShader->setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
    objShader->setVec3("light.diffuse", 0.8f, 0.8f, 0.8f);
    objShader->setVec3("light.specular", 1.0f, 1.0f, 1.0f);

    objShader->setFloat("material.shininess", 64.0f);

    objShader->setMat4("projection", projection);
    objShader->setMat4("view", view);

    // Render adventurer
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(camera_adventurer.position.x, -0.7, camera_adventurer.position.z));
    model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f));
    objShader->setMat4("model", model);
    objShader->setMat3("model_res", glm::mat3(glm::transpose(glm::inverse(model))));
    Model characterBallAdv("res/ball.obj");
    characterBallAdv.Draw(*objShader);

    if (camera->isAdventurer && gameState == 0 && reachReg(camera->position, maze->getStartPoint(2.))) {
        // at start point
        gameState = 1;
        printf("Start playing now\n");
    } else if (camera->isAdventurer && gameState == 1 && reachReg(camera->position, maze->getEndPoint(2.))) {
        // at end point
        gameState = 2;
        printf("Congratulations, you win!\n");
    }

//    for(int i = -map_sz; i < maze->get_row_num() + map_sz; ++i)
//        for(int j = -map_sz; j < maze->get_col_num() + map_sz; ++j) {
//            // render the loaded model
//            glm::mat4 model = glm::mat4(1.0f);
//            model = glm::translate(model,
//                                   glm::vec3(i * 2., -4.f, j * 2.)); // translate it down so it's at the center of the scene
//            model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));    // it's a bit too big for our scene, so scale it down
//            ourShader->setMat4("model", model);
//            models->at("bedrock").Draw(*ourShader);
//        }
    for (int i = -map_sz; i < maze->get_row_num() + map_sz; ++i)
        for (int j = -map_sz; j < maze->get_col_num() + map_sz; ++j) {
            // render the loaded model
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model,
                                   glm::vec3(i * 2., -2.f, j * 2.));
            model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
            objShader->setMat4("model", model);
            objShader->setMat3("model_res", glm::mat3(glm::transpose(glm::inverse(model))));
            models->at(((gameState == 1 && maze->start.x == i && maze->start.y == j) ||
                        (gameState == 2 && maze->end.x == i && maze->end.y == j))
                       ? "bedrock" : "dirt").Draw(*objShader);
        }

    int *curPointAt = camera->getPointAt(maze, 2.);

    for (int i = 0; i < maze->get_row_num(); ++i)
        for (int j = 0; j < maze->get_col_num(); ++j) {
            if (!maze->isWall(i, j)) continue;
            for (int _ = 0; _ < 5; ++_) {
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model,
                                       glm::vec3(i * 2., _ * 2., j * 2.));
                model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
                objShader->setMat4("model", model);
                objShader->setMat3("model_res", glm::mat3(glm::transpose(glm::inverse(model))));
                models->at((curPointAt[1] >= 0 && curPointAt[0] == i && curPointAt[1] == _ && curPointAt[2] == j)
                           ? "bedrock" : "stone").Draw(*objShader);
            }
        }

    lightCubeShader->use();
    lightCubeShader->setMat4("projection", projection);
    lightCubeShader->setMat4("view", view);

//    model = glm::mat4(1.0f);
//    model = glm::translate(model, lightPos);
//    model = glm::scale(model, glm::vec3(0.2f));
//    lightCubeShader->setMat4("model", model);
//
//    models->at("bedrock").Draw(*lightCubeShader);

    // Render uav
    model = glm::mat4(1.0f);
    model = glm::translate(model,
                           glm::vec3(camera_uav.position.x, camera_uav.position.y + 1.0f, camera_uav.position.z));
    model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
    lightCubeShader->setMat4("model", model);
    Model characterBallUav("res/ball.obj");
    characterBallUav.Draw(*lightCubeShader);

}

void Application::postRender() {
    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    glfwSwapBuffers(m_window);
    glfwPollEvents();
}

bool Application::reachReg(glm::vec3 cen1, glm::vec3 cen2) {
    return abs(cen1.x - cen2.x) < 1. &&
           abs(cen1.z - cen2.z) < 1.;
}

void Application::processInput() {
    // close
    if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(m_window, true);
    }
    // replay
    if (glfwGetKey(m_window, GLFW_KEY_R) == GLFW_PRESS) {
        camera_adventurer = Camera(
                glm::vec3(2.0f, 1.85f, -20.0f),
                glm::vec3(0.0f, 1.0f, 0.0f),
                glm::vec3(0.0f, 0.0f, 0.0f),
                true
        );
        camera_uav = Camera(
                camera_adventurer.position + glm::vec3(0, 12., 0),
                camera_adventurer.worldUp,
                camera_adventurer.position,
                false
        );
        gameState = 0;
    }
    // possible camera switching
    if (glfwGetKey(m_window, GLFW_KEY_1) == GLFW_PRESS) {
        adventurer_handle = true;
    }
    if (glfwGetKey(m_window, GLFW_KEY_2) == GLFW_PRESS) {
        camera_uav.locateTarget(camera_adventurer.position);
        adventurer_handle = false;
    }
    // keyboard movement
    if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS)
        camera->moveAround(CameraMovement::FORWARD, deltaTime, maze, 2.);
    if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS)
        camera->moveAround(CameraMovement::BACKWARD, deltaTime, maze, 2.);
    if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS)
        camera->moveAround(CameraMovement::LEFT, deltaTime, maze, 2.);
    if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS)
        camera->moveAround(CameraMovement::RIGHT, deltaTime, maze, 2.);
    if (glfwGetKey(m_window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera->moveAround(CameraMovement::UP, deltaTime, maze, 2.);
    if (glfwGetKey(m_window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        camera->moveAround(CameraMovement::DOWN, deltaTime, maze, 2.);
    // change moving speed
    if (glfwGetKey(m_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera->changeSpeed(SPEED_FAST_DEFAULT);
    else {
        camera->changeSpeed(SPEED_NORMAL_DEFAULT);
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

    this->camera->lookAround(xoffset, yoffset);
}

void Application::keyboardCallback(int key, int scancode, int action, int mods) {

}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
void Application::scrollCallback(double xoffset, double yoffset) {
    camera->zoom(yoffset);
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