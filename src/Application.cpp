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

    // configure global openGL state
    glEnable(GL_DEPTH_TEST);

    m_window = window;
    this->width = width;
    this->height = height;
    m_monitor = getBestMonitor();
    centerWindow();

    lastX = width / 2.f;
    lastY = height / 2.f;
    deltaTime = 0.f;
    lastFrame = 0.f;

    init(map_size, maze_length, maze_width);
}

void Application::init(int map_size, int maze_length, int maze_width) {
    this->winOrNot = false;

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
    depthShader = new Shader("res/shadow_mapping_depth.vs", "res/shadow_mapping_depth.fs",
                             "res/shadow_mapping_depth.gs");

    // Store the maze
    models = new map<string, Model>;
    for (const string &key : model_list) {
        models->insert(pair<string, Model>(key, Model("res/assets/" + key + ".obj")));
    }
    maze = new Maze(this->maze_len, this->maze_wid, 2.);
//    maze->print_maze();   // just for debugging

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

    // Configure depth map FBO
    glGenFramebuffers(1, &depthMapFBO);
    // Create depth cubemap texture
    glGenTextures(1, &depthCubeMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubeMap);
    for (GLuint i = 0; i < 6; ++i)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0,
                     GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    // Attach cubemap as depth map FBO's color buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubeMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    // Collections
    collection = new Model("res/cube/Cube.obj");

    markWall = new int[3] {-1, -1, -1};
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

    if (camera->isAdventurer && gameState == 0 && reachReg(camera->position, maze->getStartPoint())) {
        // at start point
        gameState = 1;
        preTime = glfwGetTime();
        startTime = glfwGetTime();
//        printf("Start playing now\n");    // just for debugging
    } else if (camera->isAdventurer && gameState == 1 && reachReg(camera->position, maze->getEndPoint())) {
        // at end point
        gameState = 2;
        endTime = glfwGetTime();
        winOrNot = true;
//        printf("Congratulations, you win!\n");    // just for debugging
    }

    if (gameState == 1) {
        if (adventurer_handle) {
            gameTime += glfwGetTime() - preTime;
        } else {
            gameTime += 3 * (glfwGetTime() - preTime);
        }

        preTime = glfwGetTime();
    }

    if (camera->isAdventurer && gameState == 1 && !maze->thingOneCollected &&
        reachReg(camera->position, maze->getThingOne().position)) {
        maze->thingOneCollected = true;
        gameTime -= maze->getThingOne().bonus;
        gameTime = (gameTime < 0) ? 0 : gameTime;
        thingOneCollectedTime = glfwGetTime();
    }
    if (camera->isAdventurer && gameState == 1 && !maze->thingTwoCollected &&
        reachReg(camera->position, maze->getThingTwo().position)) {
        maze->thingTwoCollected = true;
        gameTime -= maze->getThingTwo().bonus;
        gameTime = (gameTime < 0) ? 0 : gameTime;
        thingTwoCollectedTime = glfwGetTime();
    }
    if (camera->isAdventurer && gameState == 1 && !maze->thingThreeCollected &&
        reachReg(camera->position, maze->getThingThree().position)) {
        maze->thingThreeCollected = true;
        gameTime -= maze->getThingThree().bonus;
        gameTime = (gameTime < 0) ? 0 : gameTime;
        thingThreeCollectedTime = glfwGetTime();
    }
}

void Application::render() {

//    // view/projection transformations
    glm::mat4 projection = glm::perspective(glm::radians(camera->fov), (float) width / (float) height,
                                            camera->zNear, camera->zFar);
    glm::mat4 view = camera->getViewMatrix();
    glm::vec3 lightPos(camera_uav.position.x, camera_uav.position.y + 1.0f, camera_uav.position.z);

    // 0. Create depth cubemap transformation matrices
    GLfloat aspect = (GLfloat) SHADOW_WIDTH / (GLfloat) SHADOW_HEIGHT;
    GLfloat near = 1.0f;
    GLfloat far = 10000.0f;
    glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), aspect, near, far);
    std::vector<glm::mat4> shadowTransforms;
    shadowTransforms.push_back(
            shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
    shadowTransforms.push_back(
            shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
    shadowTransforms.push_back(
            shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
    shadowTransforms.push_back(
            shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
    shadowTransforms.push_back(
            shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)));
    shadowTransforms.push_back(
            shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));

    // 1. Render scene to depth cubemap
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    depthShader->use();
    for (GLuint i = 0; i < 6; ++i)
        depthShader->setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
    depthShader->setFloat("far_plane", far);
    depthShader->setVec3("lightPos", lightPos);
    renderObject(depthShader);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // don't forget to enable shader before setting uniforms
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    objShader->use();
    objShader->setInt("material.diffuse", 0);
    objShader->setInt("material.specular", 1);
    objShader->setFloat("material.shininess", 64.0f);

    objShader->setVec3("lights[0].position", lightPos);
    objShader->setVec3("lights[0].ambient", 0.3f, 0.3f, 0.3f);
    objShader->setVec3("lights[0].diffuse", 0.95f, 0.95f, 0.95f);
    objShader->setVec3("lights[0].specular", 1.0f, 1.0f, 1.0f);
    objShader->setFloat("lights[0].constant", 1.0f);
    objShader->setFloat("lights[0].linear", 0.01f);
    objShader->setFloat("lights[0].quadratic", 0.00025);

    objShader->setVec3("viewPos", camera->position);

    objShader->setMat4("projection", projection);
    objShader->setMat4("view", view);

    objShader->setBool("shadows", shadows);
    objShader->setFloat("far_plane", far);

    objShader->setInt("depthMap", 2);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubeMap);
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
    ss_fps << (int) (1.0f / deltaTime) << " FPS";
    freeType->renderText(ss_fps.str(), 25.0f, height - 49.0f, 0.5f, glm::vec3(0.8f, 0.8f, 0.2f));
    freeType->renderText(gamestates[gameState], 25.0f, 25.0f, 1.0f, glm::vec3(0.5f, 0.8f, 0.2f));

    std::stringstream ss_time;
    ss_time << "time " << (int) gameTime;
    freeType->renderText(ss_time.str(), width / 2. - 2 * font_size * 0.8f, 25.0f, 0.8f, glm::vec3(0.2f, 0.8f, 0.8f));

    freeType->renderText("o", width / 2., height / 2., 0.5f, glm::vec3(0.3f, 0.7f, 0.9f));   // render cursor

    freeType->renderText("enter R to replay or level up", width - 383.0f, height - 40.0f, 0.4f, glm::vec3(0.5f, 0.2f, 0.5f));
    freeType->renderText("enter E to mark a wall block", width - 378.0f, height - 70.0f, 0.4f, glm::vec3(0.5f, 0.2f, 0.5f));
    freeType->renderText("enter 1 to adv mode", width - 262.0f, height - 100.0f, 0.4f, glm::vec3(0.5f, 0.2f, 0.5f));
    freeType->renderText("enter 2 to uav mode", width - 266.0f, height - 130.0f, 0.4f, glm::vec3(0.5f, 0.2f, 0.5f));

    if (adventurer_handle) {
        freeType->renderText("a", width - 100.0f, 25.0f, 1.5f, glm::vec3(0.5f, 0.8f, 0.2f));
    } else {
        freeType->renderText("u", width - 100.0f, 25.0f, 1.5f, glm::vec3(0.5f, 0.8f, 0.2f));
    }

    if (gameState == 1 && glfwGetTime() - thingOneCollectedTime <= 3) {
        std::stringstream ss_thing1;
        ss_thing1 << "Item collected with bonus " << (int) maze->getThingOne().bonus;
        freeType->renderText(ss_thing1.str(), width / 2 - 250, height / 2 - 35, 0.6f, glm::vec3(0.95f, 0.29f, 0.49f));
    }
    if (gameState == 1 && glfwGetTime() - thingTwoCollectedTime <= 3) {
        std::stringstream ss_thing2;
        ss_thing2 << "Item collected with bonus " << (int) maze->getThingTwo().bonus;
        freeType->renderText(ss_thing2.str(), width / 2 - 250, height / 2 - 70, 0.6f, glm::vec3(0.95f, 0.29f, 0.49f));
    }
    if (gameState == 1 && glfwGetTime() - thingThreeCollectedTime <= 3) {
        std::stringstream ss_thing3;
        ss_thing3 << "Item collected with bonus " << (int) maze->getThingThree().bonus;
        freeType->renderText(ss_thing3.str(), width / 2 - 250, height / 2 - 105, 0.6f, glm::vec3(0.95f, 0.29f, 0.49f));
    }

    if (gameState == 1 && glfwGetTime() - startTime <= 1) {
        freeType->renderText("go go go", width / 2 - 400, height / 2, 3.0f, glm::vec3(0.95f, 0.29f, 0.49f));
    }
    if (gameState == 2 && glfwGetTime() - endTime <= 3) {
        freeType->renderText("you win", width / 2 - 300, height / 2, 3.0f, glm::vec3(0.95f, 0.29f, 0.49f));
    }

    std::stringstream ss_level;
    ss_level << "level " << (int) gameLevel;
    if (glfwGetTime() - levelTime <= 3) {
        freeType->renderText(ss_level.str(), width / 2 - 300, height / 2, 3.0f, glm::vec3(0.95f, 0.29f, 0.49f));
    }

    if (gameState == 1 && glfwGetTime() - markJitterTime <= 1) {
        if (markWall[0] < 0 || markWall[1] < 0 || markWall[2] < 0) {
            freeType->renderText("wall mark removed", width / 2 - 150, height / 2 + 150, 0.6f, glm::vec3(0.95f, 0.29f, 0.49f));
        } else {
            freeType->renderText("wall marked", width / 2 - 100, height / 2 + 150, 0.6f, glm::vec3(0.95f, 0.29f, 0.49f));
        }
    }
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

    // three collections
    if (!maze->thingOneCollected) {
        shader->setMat4("model", maze->getThingOne().model);
        shader->setMat3("model_res", maze->getThingOne().model_res);
        collection->Draw(*shader);
    }
    if (!maze->thingTwoCollected) {
        shader->setMat4("model", maze->getThingTwo().model);
        shader->setMat3("model_res", maze->getThingTwo().model_res);
        collection->Draw(*shader);
    }
    if (!maze->thingThreeCollected) {
        shader->setMat4("model", maze->getThingThree().model);
        shader->setMat3("model_res", maze->getThingThree().model_res);
        collection->Draw(*shader);
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
                wall_model[i][j][_].type = (gameState == 1 &&
                        ((curPointAt[0] == i && curPointAt[1] == _ && curPointAt[2] == j) ||
                        (markWall[0] == i && markWall[1] == _ && markWall[2] == j)))
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
        if (winOrNot) {
            maze_len += 2;
            maze_wid += 2;
            gameLevel++;
        }

        levelTime = glfwGetTime();
        init(map_sz, maze_len, maze_wid);

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
        gameTime = 0;
        preTime = glfwGetTime();
    }
    // Binding option
    if (glfwGetKey(m_window, GLFW_KEY_B) == GLFW_PRESS) {
        bindAdventurer = !bindAdventurer;
    }
    // possible camera switching
    if (glfwGetKey(m_window, GLFW_KEY_1) == GLFW_PRESS) {
        adventurer_handle = true;
    }
    if (glfwGetKey(m_window, GLFW_KEY_2) == GLFW_PRESS && gameState != 0) {
        camera_uav.locateTarget(camera_adventurer.position);
        if (gameState == 1 && adventurer_handle) {
            notAdvTime = glfwGetTime();
            gameTime += 5;
        }
        adventurer_handle = false;
    }
    // mark an object
    if (glfwGetKey(m_window, GLFW_KEY_E) == GLFW_PRESS) {
        if (gameState == 1 && glfwGetTime() - markJitterTime > 1) {
            int *curPointAt = camera->getPointAt(maze, 2.);
            if (curPointAt[0] == markWall[0] && curPointAt[1] == markWall[1] && curPointAt[2] == markWall[2]) {
                // cancel marking
                markWall[0] = markWall[1] = markWall[2] = -1;
            } else {
                // mark that wall
                markWall[0] = curPointAt[0];
                markWall[1] = curPointAt[1];
                markWall[2] = curPointAt[2];
            }
        }
        markJitterTime = glfwGetTime();
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
    if (glfwGetKey(m_window, GLFW_KEY_P) == GLFW_PRESS)
        shadows = !shadows;
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

/* Center Window Functions */
void Application::centerWindow() {
    // center the window on the best suitable monitor
    if (!m_monitor) {
        return;
    }
    const GLFWvidmode *mode = glfwGetVideoMode(m_monitor);
    if (!mode) {
        return;
    }

    int monitorX, monitorY;
    glfwGetMonitorPos(m_monitor, &monitorX, &monitorY);

    int windowWidth, windowHeight;
    glfwGetWindowSize(m_window, &windowWidth, &windowHeight);

    glfwSetWindowPos(
            m_window,
            monitorX + (mode->width - windowWidth) / 2,
            monitorY + (mode->height - windowHeight) / 2
    );
}

GLFWmonitor *Application::getBestMonitor() {
    int monitorCount;
    GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);

    if (!monitors) {
        return NULL;
    }

    int windowX, windowY, windowWidth, windowHeight;
    glfwGetWindowSize(m_window, &windowWidth, &windowHeight);
    glfwGetWindowPos(m_window, &windowX, &windowY);

    GLFWmonitor *bestMonitor = NULL;
    int bestArea = 0;

    for (int i = 0; i < monitorCount; ++i) {
        GLFWmonitor *monitor = monitors[i];
        int monitorX, monitorY;
        glfwGetMonitorPos(monitor, &monitorX, &monitorY);

        const GLFWvidmode *mode = glfwGetVideoMode(monitor);
        if (!mode) {
            continue;
        }

        int areaMinX = MAX(windowX, monitorX);
        int areaMinY = MAX(windowY, monitorY);
        int areaMaxX = MIN(windowX + windowWidth, monitorX + mode->width);
        int areaMaxY = MIN(windowY + windowHeight, monitorY + mode->height);
        int area = (areaMaxX - areaMinX) * (areaMaxY - areaMinY);

        if (area > bestArea) {
            bestArea = area;
            bestMonitor = monitor;
        }
    }

    return bestMonitor;
}