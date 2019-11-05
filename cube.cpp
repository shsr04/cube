#include "range.hpp"
#include "sdl_wrap.hpp"
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_video.h>

int main() {
    Init _init;
    // SDL_GL_LoadLibrary(nullptr);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8); // ?
    Window window(800, 400, "Testing", SDL_WINDOW_OPENGL);
    GlContext gl(window, window.width_, window.height_);

    // glEnableClientState(GL_VERTEX_ARRAY);
    // glEnableClientState(GL_NORMAL_ARRAY);
    // glVertexPointer(3, GL_DOUBLE, 0, vertices.data());

    gl.matrix(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-1, 1, -1, 1, 0, 10);
    gl.matrix(GL_MODELVIEW);
    gl.setClearColor({0.35f, 0.35f, 0.35f, 1.f});

    glShadeModel(GL_SMOOTH);
    std::array<GLfloat, 4> lightPosition = {0, 2, 1, 1};
    std::array<GLfloat, 4> lightAmbient = {.2, .2, .2, 1};
    std::array<GLfloat, 4> lightDiffuse = {.5, .5, .5, 1};
    std::array<GLfloat, 4> lightSpecular = {.3, .3, .3, 1};
    std::array<GLfloat, 4> materialDiffuse = {.9, 0, .9, 1};
    std::array<GLfloat, 4> materialSpecular = {.1, .1, .1, 1};
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition.data());
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient.data());
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse.data());
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular.data());
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, materialDiffuse.data());
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular.data());
    glMaterialf(GL_FRONT, GL_SHININESS, 0.3);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    int mode = 1;
    while (true) {
        SDL_Event e;
        if (SDL_PollEvent(&e)) {
            if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                case SDLK_1:
                    mode = 1;
                    break;
                case SDLK_2:
                    mode = 2;
                    break;
                case SDLK_ESCAPE:
                    e.type = SDL_QUIT;
                    break;
                }
            }
            if (e.type == SDL_QUIT)
                break;
        }
        gl.clear();
        glColor3f(1, 0, 1);
        glLoadIdentity();
        glScaled(0.5, 0.5, 0.5);
        glRotated(50, 0, 1, 0.3);
        // glTranslated(-0.5, 0.5, 1);
        glDisable(GL_LIGHTING);
        glLineWidth(5.0);
        gl.draw<2, 3>(GL_LINE, {{{0, 2, 1}, {0, 0, 0}}}, {1, 0, 0, 1});
        gl.drawAxes();
        glEnable(GL_LIGHTING);
        gl.draw(GL_QUADS, Cube::v_, {1, 0, 1, 1});
        gl.swapBuffers();
    }
}
