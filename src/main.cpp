#include "Application.h"
#include "maze.h"

#define DEBUG false

int main() {

    Application::init();
    auto application = Application("HIM", 1920, 1080, 16, 21, 23, DEBUG);

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