#include "range.hpp"
#include "sdl_wrap.hpp"
#include "sdl_wrap_gl45.hpp"
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>
#include <algorithm>
#include <optional>
#include <string>

int main() {
    Init _init;
    setInitialGlAttributes();
    Window window(800, 400, "Testing", SDL_WINDOW_OPENGL);
    GlContext context(window);

    std::array<GLfloat, 24> cube = {
        /*v*/ 0, 0, 0, /*v*/ 1, 0, 0,
        /*v*/ 1, 1, 0, /*v*/ 0, 1, 0,
        /*v*/ 0, 0, 1, /*v*/ 1, 0, 1,
        /*v*/ 1, 1, 1, /*v*/ 0, 1, 1,
    };
    std::transform(cube.begin(), cube.end(), cube.begin(),
                   [](auto x) { return x / 2; });

    VertexArray va;
    va.bind();
    Buffer vb;
    vb.bind();
    glBufferData(GL_ARRAY_BUFFER, cube.size() * sizeof(GLfloat), cube.data(),
                 GL_STATIC_DRAW);

    Shader vsh(GL_VERTEX_SHADER, "vertex.glsl");
    Shader fsh(GL_FRAGMENT_SHADER, "fragment.glsl");
    Program prog;
    prog.attachShader(vsh);
    prog.attachShader(fsh);
    glBindFragDataLocation(prog, 0, "color");
    prog.link();

    assert((err = glGetError()) == GL_NO_ERROR, errorString(), __LINE__);
    // not working?
    // http://www.opengl-tutorial.org/beginners-tutorials/tutorial-2-the-first-triangle/

    glUseProgram(prog);

    auto mainColor = glGetUniformLocation(prog, "mainColor");
    glUniform3f(mainColor, 1, 0.5, 0);
    auto vertexCoord = GLuint(glGetAttribLocation(prog, "vertexCoord"));
    glVertexAttribPointer(vertexCoord, 3, GL_FLOAT, GL_FALSE, 0, 0);

    while (true) {
        SDL_Event e;
        if (SDL_PollEvent(&e)) {
            if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                case SDLK_ESCAPE:
                    e.type = SDL_QUIT;
                    break;
                }
            }
            if (e.type == SDL_QUIT)
                break;
        }

        glClearColor(0.6, 0.6, 0.6, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnableVertexAttribArray(vertexCoord);
        glLineWidth(10.0);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 8);
        glDisableVertexAttribArray(vertexCoord);

        SDL_GL_SwapWindow(window);

        assert((err = glGetError()) == GL_NO_ERROR, errorString(), __LINE__);
    }
}
