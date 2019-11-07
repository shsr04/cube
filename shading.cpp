#include "range.hpp"
#include "sdl_wrap.hpp"
#include "sdl_wrap_gl45.hpp"
#include "soil/src/SOIL.h"
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_video.h>
#include <algorithm>
#include <glm/ext/matrix_transform.hpp>
#include <optional>
#include <string>

// http://www.opengl-tutorial.org/beginners-tutorials/tutorial-2-the-first-triangle/
// http://open.gl

int main() {
    Init _init;
    assert_true((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != 0,
                "PNG init failed");
    setRequiredGlVersion();
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    Window window(800, 400, "Testing", SDL_WINDOW_OPENGL);
    GlContext context(window);

    std::array<GLfloat, 8 * 5> cube = {
        //      xyz      uv
        /*FDL*/ 0, 0, 0, 0, 0, /*FDR*/ 1, 0, 0, 1, 0,
        /*FUR*/ 1, 1, 0, 1, 1, /*FUL*/ 0, 1, 0, 0, 1,
        /*BDL*/ 0, 0, 1, 0, 0, /*BDR*/ 1, 0, 1, 1, 0,
        /*BUR*/ 1, 1, 1, 1, 1, /*BUL*/ 0, 1, 1, 1, 0,
    };
    glm::mat4 cubeRotX =
        glm::rotate(glm::mat4(1.f), glm::radians(0.f), glm::vec3(1, 0, 0));
    std::transform(cube.begin(), cube.end(), cube.begin(),
                   [](auto x) { return x / 2; });
    const auto cubeElems = 6;
    std::array<GLuint, cubeElems> elems = {
        0, 1, 2, 2, 3, 0,
    };

    VertexArray va;
    Buffer vb(GL_ARRAY_BUFFER, cube);
    Buffer eb(GL_ELEMENT_ARRAY_BUFFER, elems);

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

    auto mainColor = glGetUniformLocation(prog, "u_main_color");
    glUniform3f(mainColor, 1, 0.5, 0);
    auto vertexCoord = prog.setAttribPointer("i_vertex_coord", 3, GL_FLOAT,
                                             GL_FALSE, 5 * sizeof(GLfloat), 0);
    // auto textureCoord =
    //    prog.setAttribPointer("i_texture_coord", 2, GL_FLOAT, GL_FALSE,
    //                          5 * sizeof(GLfloat), 3 * sizeof(GLfloat));
    std::vector<GLuint> cubeAttribs = {
        vertexCoord,
    };
    assert_true((err = glGetError()) == GL_NO_ERROR, errorString(), __LINE__);

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
        prog.enableAttrib(cubeAttribs);
        // glDrawArrays(GL_TRIANGLE_STRIP, 0, 8);
        glDrawElements(GL_TRIANGLES, cubeElems, GL_UNSIGNED_INT, 0);
        prog.disableAttrib(cubeAttribs);

        SDL_GL_SwapWindow(window);
        assert_true((err = glGetError()) == GL_NO_ERROR, errorString(),
                    __LINE__);
    }
}
