#include "range.hpp"
#include "sdl_wrap.hpp"
#include "sdl_wrap_gl.hpp"
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_video.h>
#include <array>

void handleEvents(bool &quit, int &mode, Cube &c) {
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
            case SDLK_w:
                c.rot_[1] += 3.0;
                break;
            case SDLK_s:
                c.rot_[1] -= 3.0;
                break;
            case SDLK_a:
                c.rot_[0] += 3.0;
                break;
            case SDLK_d:
                c.rot_[0] -= 3.0;
                break;
            case SDLK_ESCAPE:
                e.type = SDL_QUIT;
                break;
            }
        }
        if (e.type == SDL_QUIT)
            quit = true;
    }
}

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
    const std::array<GLfloat, 4> lightPos = {0, 5, 2, 0};
    gl.lighting(lightPos, {.9, 0, .9, 1}, 0.9);
    Cube c({-0.5, 0.5, 1}, {0, 50, 20}, {0.5, 0.5, 0.5});

    bool quit = false;
    int mode = 1;
    while (!quit) {
        handleEvents(quit, mode, c);
        gl.clear();
        newMatrix([&] {
            c.scale();
            c.rotate();
            c.translate();
            newIdMatrix([&] {
                glDisable(GL_LIGHTING);
                glLineWidth(5.0);
                gl.draw<2, 3>(
                    GL_LINES,
                    {{{lightPos[0], lightPos[1], lightPos[2]}, {0, 0, 0}}},
                    {1, 1, 1, 1});
                // gl.drawAxes();
                glEnable(GL_LIGHTING);
            });
            c.draw(&gl);
        });
        gl.swapBuffers();
    }
}
