#include "Application.h"

int main() {
    Application::init();
    auto application = Application("HIM", 800, 600);

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