#include "range.hpp"
#include "sdl_wrap.hpp"
#include "sdl_wrap_gl45.hpp"
#include <algorithm>
#include <iostream>
#include <string>

// http://www.opengl-tutorial.org/beginners-tutorials/tutorial-2-the-first-triangle/
// http://open.gl

struct Config {
    const int windowW = 800, windowH = 400;
    GLfloat rotDelta = 10.f;
    glm::mat4 viewMatrix, projMatrix;
} config;

void handleKeys(SDL_Event &e, Program &prog, Cube &cube, Config &config) {
    switch (e.key.keysym.sym) {
    case SDLK_w:
        cube.rotate(config.rotDelta, 0, 0);
        prog.setUniformMatrix("u_model_transf", cube.model());
        break;
    case SDLK_s:
        cube.rotate(-config.rotDelta, 0, 0);
        prog.setUniformMatrix("u_model_transf", cube.model());
        break;
    case SDLK_a:
        cube.rotate(0, config.rotDelta, 0);
        prog.setUniformMatrix("u_model_transf", cube.model());
        break;
    case SDLK_d:
        cube.rotate(0, -config.rotDelta, 0);
        prog.setUniformMatrix("u_model_transf", cube.model());
        break;
    case SDLK_q:
        cube.rotate(0, 0, config.rotDelta);
        prog.setUniformMatrix("u_model_transf", cube.model());
        break;
    case SDLK_e:
        cube.rotate(0, 0, -config.rotDelta);
        prog.setUniformMatrix("u_model_transf", cube.model());
        break;
    case SDLK_r:
        cube.translate(0, 0, -0.1f);
        prog.setUniformMatrix("u_model_transf", cube.model());
        break;
    case SDLK_f:
        cube.translate(0, 0, 0.1f);
        prog.setUniformMatrix("u_model_transf", cube.model());
        break;
    case SDLK_ESCAPE:
        e.type = SDL_QUIT;
        break;
    }
}

int main() {
    Init _init;
    assert_true((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != 0,
                "PNG init failed");
    setRequiredGlVersion();
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    Window window(config.windowW, config.windowH, "Testing", SDL_WINDOW_OPENGL);
    GlContext context(window);

    Cube cube;

    VertexArray va;
    Buffer vb(GL_ARRAY_BUFFER, CUBE_VERTICES);
    Buffer eb(GL_ELEMENT_ARRAY_BUFFER, CUBE_WIRE_INDICES);

    Shader vsh(GL_VERTEX_SHADER, "vertex.glsl");
    Shader fsh(GL_FRAGMENT_SHADER, "fragment.glsl");
    Program prog;
    prog.attachShader(vsh);
    prog.attachShader(fsh);
    // specify the fragment color output
    glBindFragDataLocation(prog, 0, "o_color");
    prog.link();
    glUseProgram(prog);
    assert_true((err = glGetError()) == GL_NO_ERROR, errorString(), __LINE__);

    prog.setUniformVector("u_main_color", {1, 0.5, 0, 1});
    prog.setUniformMatrix("u_model_transf", cube.model());
    config.viewMatrix =
        glm::lookAt(glm::vec3{0, 0, 2}, glm::vec3{0, 0, 0}, glm::vec3{0, 1, 0});
    prog.setUniformMatrix("u_view_transf", glm::value_ptr(config.viewMatrix));
    config.projMatrix = glm::perspective(
        45.f, GLfloat(config.windowW) / GLfloat(config.windowH), 0.f, 10.f);
    auto projTransf = glGetUniformLocation(prog, "u_proj_transf");
    glUniformMatrix4fv(projTransf, 1, GL_FALSE,
                       glm::value_ptr(config.projMatrix));

    auto vertexCoord = prog.setAttribPointer("i_vertex_coord", 3, GL_FLOAT,
                                             GL_FALSE, 3 * sizeof(GLfloat), 0);
    cube.attr_ = {vertexCoord};
    assert_true((err = glGetError()) == GL_NO_ERROR, errorString(), __LINE__);

    int mode = 1;
    while (true) {
        SDL_Event e;
        if (SDL_PollEvent(&e)) {
            if (e.type == SDL_KEYDOWN) {
                handleKeys(e, prog, cube, config);
            }
            if (e.type == SDL_QUIT)
                break;
        }
        glClearColor(0.6f, 0.6f, 0.6f, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        if (mode == 1) {
            prog.enableAttrib(cube.attr_);
            cube.drawElems(GL_LINES, CUBE_WIRE_INDICES.size());
            prog.disableAttrib(cube.attr_);
        } else if (mode == 2) {
        }

        SDL_GL_SwapWindow(window);
        assert_true((err = glGetError()) == GL_NO_ERROR, errorString(),
                    __LINE__);
    }
}
