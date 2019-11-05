#pragma once
#include "sdl_wrap_header.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h> // using OpenGL 1.1 (Compatibility profile)
#include <SDL2/SDL_video.h>
#include <array>
#include <utility>

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

    void lighting(std::array<GLfloat, 4> lightPosition = {0, 0, 0, 1},
                  std::array<GLfloat, 4> materialReflection = {0, 0, 0, 0},
                  GLfloat shininess = 0) {
        glLightfv(GL_LIGHT0, GL_POSITION, lightPosition.data());
        const std::array<GLfloat, 4> lightAmbient = {.5, .5, .5, 1};
        glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient.data());
        const std::array<GLfloat, 4> lightDiffuse = {.7, .7, .5, 1};
        glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse.data());
        const std::array<GLfloat, 4> lightSpecular = {.3, .3, .3, 1};
        glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular.data());
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE,
                     materialReflection.data());
        std::array<GLfloat, 4> materialSpecular = {.5, .5, .1, 1};
        glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular.data());
        glMaterialf(GL_FRONT, GL_SHININESS, shininess);
        glShadeModel(GL_SMOOTH);
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
    }
};

template <class F> void newMatrix(F &&f) {
    glPushMatrix();
    std::forward<F>(f)();
    glPopMatrix();
}
template <class F> void newIdMatrix(F &&f) {
    newMatrix([&f] {
        glLoadIdentity();
        std::forward<F>(f)();
    });
}

class GlObject {
  public:
    std::array<double, 3> trans_;
    std::array<double, 3> rot_;
    std::array<double, 3> scale_;

    GlObject(std::array<double, 3> translation = {0},
             std::array<double, 3> rotation = {0},
             std::array<double, 3> scale = {1, 1, 1})
        : trans_(translation), rot_(rotation), scale_(scale) {}

    virtual void draw(GlContext *) = 0;

    void rotate() {
        glRotated(rot_[0], 1, 0, 0);
        glRotated(rot_[1], 0, 1, 0);
        glRotated(rot_[2], 0, 0, 1);
    }
    void translate() { glTranslated(trans_[0], trans_[1], trans_[2]); }
    void scale() { glScaled(scale_[0], scale_[1], scale_[2]); }
};

class Cube : public GlObject {
    const Vectors<24> v_ = {{
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

  public:
    Cube(std::array<double, 3> translation = {0},
         std::array<double, 3> rotation = {0},
         std::array<double, 3> scale = {1, 1, 1})
        : GlObject(translation, rotation, scale) {}
    void draw(GlContext *context) override { context->draw<24>(GL_QUADS, v_); }
    void drawWire(GlContext *context) { context->draw<24>(GL_LINE_LOOP, v_); }
};
