#pragma once
#include "sdl_wrap_header.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h> // using OpenGL 1.1 (Compatibility profile)
#include <SDL2/SDL_video.h>
#include <array>

template <size_t N, size_t D = 3>
using Vectors = std::array<std::array<GLdouble, D>, N>;

class GlContext {
    SDL_Window *window_;
    SDL_GLContext context_;

  public:
    GlContext(SDL_Window *window, int width, int height, int vsync = 1)
        : window_(window), context_(SDL_GL_CreateContext(window)) {
        assert(context_, "failed to create GL context");
        SDL_GL_SetSwapInterval(vsync);
        glEnable(GL_CULL_FACE | GL_DEPTH_TEST);
        glViewport(0, 0, width, height);
    }

    void matrix(GLenum mode) {
        glMatrixMode(mode);
        glLoadIdentity();
    }

    void drawAxes() {
        draw<2>(GL_LINES, {{{0, 0, 0}, {1, 0, 0}}}, {1, 0, 0, 1});
        draw<2>(GL_LINES, {{{0, 0, 0}, {0, 1, 0}}}, {0, 1, 0, 1});
        draw<2>(GL_LINES, {{{0, 0, 0}, {0, 0, 1}}}, {0, 0, 1, 1});
    }

    void setClearColor(std::array<GLclampf, 4> rgba) {
        glClearColor(rgba[0], rgba[1], rgba[2], rgba[3]);
    }

    void clear() { glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); }

    /**
     * Draw vertices given in a 2D array.
     * @param mode one of: GL_POINTS, GL_LINES, GL_LINE_STRIP, GL_TRIANGLES,
     * GL_TRIANGLE_STRIP, GL_QUADS
     * @param vertices an array containing `N` total values, divided into
     * sub-arrays of dimension `D`.
     *
     * E.g. to draw four 3D vertices in POINTS mode:
     *  draw<4,3>(GL_POINTS, {{{x1,y1,z1},{x2,y2,z2},...}});
     */
    template <size_t N, size_t D = 3>
    void draw(GLenum mode, Vectors<N, D> vertices,
              std::array<GLclampd, 4> color = {1, 1, 1, 1}) {
        assert(D == 2 || D == 3 || D == 4,
               "can only draw 2D, 3D or 4D vertices");
        std::array<GLclampd, 4> oldColor;
        glGetDoublev(GL_CURRENT_COLOR, oldColor.data());
        glColor4dv(color.data());
        glBegin(mode);
        for (auto &_v : vertices) {
            auto v = _v.data();
            if (D == 2)
                glVertex2dv(v);
            else if (D == 3)
                glVertex3dv(v);
            else if (D == 4)
                glVertex4dv(v);
        }
        glEnd();
        glColor4dv(oldColor.data());
    }

    void swapBuffers() { SDL_GL_SwapWindow(window_); }
};

class Cube {
  public:
    static constexpr Vectors<24> v_ = {{
        // front face
        {0, 0, 0},
        {1, 0, 0},
        {1, 1, 0},
        {0, 1, 0},
        // back face
        {0, 0, 1},
        {1, 0, 1},
        {1, 1, 1},
        {0, 1, 1},
        // left face
        {0, 0, 1},
        {0, 0, 0},
        {0, 1, 0},
        {0, 1, 1},
        // right face
        {1, 0, 0},
        {1, 1, 0},
        {1, 1, 1},
        {1, 0, 1},
        // top face
        {0, 1, 0},
        {0, 1, 1},
        {1, 1, 1},
        {1, 1, 0},
        // bottom face
        {0, 0, 0},
        {0, 0, 1},
        {1, 0, 1},
        {1, 0, 0},
    }};
};
