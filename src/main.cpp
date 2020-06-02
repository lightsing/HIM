#include "Application.h"
#include "maze.h"

#define DEBUG true

int main() {

    Application::init();

    int width = 1920, height = 1080;
    int map_sz = 20, maze_len = 7, maze_wid = 5;
    if (DEBUG) {
        width = 1600;
        height = 900;
    }
    auto application = Application("Maze Runner", width, height, map_sz, maze_len, maze_wid, DEBUG);

    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render loop
    while (application.alive()) {
        application.preRender();

        application.render();

        application.postRender();
    }

    return application.terminate();
}