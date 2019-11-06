#include "range.hpp"
#include "sdl_wrap.hpp"
#include "sdl_wrap_gl45.hpp"
#include "soil/src/SOIL.h"
#include <SDL2/SDL_video.h>
#include <algorithm>
#include <optional>
#include <string>

// http://www.opengl-tutorial.org/beginners-tutorials/tutorial-2-the-first-triangle/
// http://open.gl

int main() {
    Init _init;
    setRequiredGlVersion();
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    Window window(800, 400, "Testing", SDL_WINDOW_OPENGL);
    GlContext context(window);

    std::array<GLfloat, 24> cube = {
        /*FDL*/ 0, 0, 0, /*FDR*/ 1, 0, 0,
        /*FUR*/ 1, 1, 0, /*FUL*/ 0, 1, 0,
        /*BDL*/ 0, 0, 1, /*BDR*/ 1, 0, 1,
        /*BUR*/ 1, 1, 1, /*BUL*/ 0, 1, 1,
    };
    std::transform(cube.begin(), cube.end(), cube.begin(),
                   [](auto x) { return x / 2; });
    const auto cubeElems = 3;
    std::array<GLint, cubeElems> elems = {0, 1, 2};

    VertexArray va;
    Buffer vb(GL_ARRAY_BUFFER, cube);
    Buffer eb(GL_ELEMENT_ARRAY_BUFFER, elems);

    Texture t;
    int width, height;
    auto image =
        SOIL_load_image("teapot.png", &width, &height, nullptr, SOIL_LOAD_RGB);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, image);
    SOIL_free_image_data(image);

    Shader vsh(GL_VERTEX_SHADER, "vertex.glsl");
    Shader fsh(GL_FRAGMENT_SHADER, "fragment.glsl");
    Program prog;
    prog.attachShader(vsh);
    prog.attachShader(fsh);
    glBindFragDataLocation(prog, 0, "color");
    prog.link();

    assert((err = glGetError()) == GL_NO_ERROR, errorString(), __LINE__);
    glUseProgram(prog);

    auto mainColor = glGetUniformLocation(prog, "u_main_color");
    glUniform3f(mainColor, 1, 0.5, 0);
    auto vertexCoord = prog.setAttribPointer("i_vertex_coord", 3, GL_FLOAT);

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
        glClearColor(0.6f, 0.6f, 0.6f, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        prog.enableAttrib({vertexCoord});
        // glDrawArrays(GL_TRIANGLE_STRIP, 0, 8);
        glDrawElements(GL_TRIANGLES, cubeElems, GL_UNSIGNED_INT, 0);
        prog.disableAttrib({vertexCoord});

        SDL_GL_SwapWindow(window);

        assert((err = glGetError()) == GL_NO_ERROR, errorString(), __LINE__);
    }
}
