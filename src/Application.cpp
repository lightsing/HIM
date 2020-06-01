//
// Created by light on 5/6/2020.
//

#include "Application.h"

static string model_list[] = {"stone", "dirt", "bedrock"};
static string gamestates[] = {"free", "start", "finish"};
static float font_size = 48;

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
    bindAdventurer = true;
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
    characterBallAdv = new Model("res/ball/ball.obj");
    characterBallUav = new Model("res/UFO/UFO.obj");

    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    freeType = new FreeType("res/assets/fonts/minecrafter/Minecrafter.Reg.ttf");
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

    floor_model = new CubeModel *[maze->get_row_num() + 2 * map_sz];
    for (int i = 0; i < maze->get_row_num() + 2 * map_sz; ++i) {
        floor_model[i] = new CubeModel[maze->get_col_num() + 2 * map_sz];
    }

    for (int i = -map_sz; i < maze->get_row_num() + map_sz; ++i)
        for (int j = -map_sz; j < maze->get_col_num() + map_sz; ++j) {
            // render the loaded model
            floor_model[i + map_sz][j + map_sz].position = glm::vec3(i * 2., -2.f, j * 2.);

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, floor_model[i + map_sz][j + map_sz].position);
            model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));

            floor_model[i + map_sz][j + map_sz].model = model;
            floor_model[i + map_sz][j + map_sz].model_res = glm::mat3(glm::transpose(glm::inverse(model)));
            floor_model[i + map_sz][j + map_sz].type = "dirt";
        }

    wall_model = new CubeModel **[maze->get_row_num()];
    for (int i = 0; i < maze->get_row_num(); ++i) {
        wall_model[i] = new CubeModel *[maze->get_col_num()];
        for (int j = 0; j < maze->get_col_num(); ++j) {
            wall_model[i][j] = new CubeModel[5];
        }
    }

    for (int i = 0; i < maze->get_row_num(); ++i)
        for (int j = 0; j < maze->get_col_num(); ++j) {
            if (!maze->isWall(i, j)) continue;
            for (int _ = 0; _ < 5; ++_) {
                wall_model[i][j][_].type = "stone";
                wall_model[i][j][_].position = glm::vec3(i * 2., _ * 2., j * 2.);

                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, wall_model[i][j][_].position);
                model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));

                wall_model[i][j][_].model = model;
                wall_model[i][j][_].model_res = glm::mat3(glm::transpose(glm::inverse(model)));
            }
        }
}

void Application::preRender() {
    // per-frame time logic
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    //printf("FPS: %.2f\n", 1.0f / deltaTime);    // just for debugging

    processInput(); // input

    glClearColor(.0f, .0f, .0f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // update uav's position with the adventurer
    if (adventurer_handle && bindAdventurer) {
        camera_uav.position = camera_adventurer.position + glm::vec3(-1., 12., -1.);
    }
    camera = adventurer_handle ? &camera_adventurer : &camera_uav;

    if (camera->isAdventurer && gameState == 0 && reachReg(camera->position, maze->getStartPoint(2.))) {
        // at start point
        gameState = 1;
        startTime = glfwGetTime();
        printf("Start playing now\n");
    } else if (camera->isAdventurer && gameState == 1 && reachReg(camera->position, maze->getEndPoint(2.))) {
        // at end point
        gameState = 2;
        printf("Congratulations, you win!\n");
    }

    if (gameState == 1) {
        gameTime = glfwGetTime() - startTime;
    }
}

void Application::render() {
//    // view/projection transformations
    glm::mat4 projection = glm::perspective(glm::radians(camera->fov), (float) width / (float) height,
                                            camera->zNear, camera->zFar);
    glm::mat4 view = camera->getViewMatrix();
    glm::vec3 lightPos(camera_uav.position.x, camera_uav.position.y + 1.0f, camera_uav.position.z);

    // don't forget to enable shader before setting uniforms
    objShader->use();
    objShader->setInt("material.diffuse", 0);
    objShader->setInt("material.specular", 1);

    objShader->setVec3("lights[0].position", lightPos);
    objShader->setVec3("lights[0].ambient", 0.3f, 0.3f, 0.3f);
    objShader->setVec3("lights[0].diffuse", 0.95f, 0.95f, 0.95f);
    objShader->setVec3("lights[0].specular", 1.0f, 1.0f, 1.0f);
    objShader->setFloat("lights[0].constant", 1.0f);
    objShader->setFloat("lights[0].linear", 0.01f);
    objShader->setFloat("lights[0].quadratic", 0.00025);

    objShader->setFloat("material.shininess", 64.0f);
    objShader->setVec3("viewPos", camera->position);

    objShader->setMat4("projection", projection);
    objShader->setMat4("view", view);

    renderObject(objShader);

    lightCubeShader->use();
    lightCubeShader->setMat4("projection", projection);
    lightCubeShader->setMat4("view", view);

    renderLight(lightPos);

    // Render Messages
    freeType->use();
    projection = glm::ortho(0.0f, static_cast<GLfloat>(width), 0.0f, static_cast<GLfloat>(height));
    freeType->setMat4("projection", projection);
    std::stringstream ss_fps;
    ss_fps << (int)(1.0f / deltaTime) << " FPS";
    freeType->renderText(ss_fps.str(), 25.0f, height - 49.0f, 0.5f, glm::vec3(0.8f, 0.8f, 0.2f));
    freeType->renderText(gamestates[gameState], 25.0f, 25.0f, 1.0f, glm::vec3(0.5f, 0.8f, 0.2f));

    std::stringstream ss_time;
    ss_time << "time " << (int)gameTime;
    freeType->renderText(ss_time.str(), width / 2. - 2 * font_size * 0.8f, 25.0f, 0.8f, glm::vec3(0.2f, 0.8f, 0.8f));

    freeType->renderText("o", width / 2., height / 2., 0.5f, glm::vec3(0.3f, 0.7f, 0.9f));   // render cursor
}

void Application::renderObject(Shader *shader) {
    // render
    // ------
    // Render adventurer
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(camera_adventurer.position.x, -0.7, camera_adventurer.position.z));
    model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f));
    shader->setMat4("model", model);
    shader->setMat3("model_res", glm::mat3(glm::transpose(glm::inverse(model))));
    characterBallAdv->Draw(*shader);

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

    for (int i = 0; i < maze->get_row_num() + 2 * map_sz; ++i)
        for (int j = 0; j < maze->get_col_num() + 2 * map_sz; ++j) {
            // render the loaded model
            shader->setMat4("model", floor_model[i][j].model);
            shader->setMat3("model_res", floor_model[i][j].model_res);
            floor_model[i][j].type = ((gameState == 1 && maze->start.x == i - map_sz && maze->start.y == j - map_sz) ||
                                      (gameState == 2 && maze->end.x == i - map_sz && maze->end.y == j - map_sz))
                                     ? "bedrock" : "dirt";
            models->at(floor_model[i][j].type).Draw(*shader);
        }

    int *curPointAt = camera->getPointAt(maze, 2.);

    for (int i = 0; i < maze->get_row_num(); ++i)
        for (int j = 0; j < maze->get_col_num(); ++j) {
            if (!maze->isWall(i, j)) continue;
            for (int _ = 0; _ < 5; ++_) {
                shader->setMat4("model", wall_model[i][j][_].model);
                shader->setMat3("model_res", wall_model[i][j][_].model_res);
                wall_model[i][j][_].type = (curPointAt[1] >= 0 && curPointAt[0] == i && curPointAt[1] == _ &&
                                            curPointAt[2] == j)
                                           ? "bedrock" : "stone";
                models->at(wall_model[i][j][_].type).Draw(*shader);
            }
        }
}

void Application::renderLight(glm::vec3 lightPos) {
    // Render uav
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model,
                           lightPos);
    model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
    lightCubeShader->setMat4("model", model);
    characterBallUav->Draw(*lightCubeShader);
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
        startTime = 0;
        gameTime = 0;
    }
    // Binding option
    if (glfwGetKey(m_window, GLFW_KEY_B) == GLFW_PRESS) {
        bindAdventurer = (bindAdventurer) ? false : true;
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