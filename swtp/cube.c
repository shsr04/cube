#include <GL/glut.h>
#include <math.h>
#include <stdio.h>

#define lighting_version 2
#define collision_version 3
#define drag_version 3
#define debugFrustum_off
#define toedIn_off
#define debugEdges_off

#if lighting_version > 2
#error "lighting v2 is the latest version!"
#endif
#if collision_version > 3
#error "collision v3 is the latest version!"
#endif
#if drag_version > 3
#error "drag v3 is the latest version!"
#endif

#define PI 3.14159265
#define Width 600
#define Height 600

#define NumBalls 5
#define CUBE 0
#define BALLS 1
#define CURVE 2
#define HYPER 3
#define QUAD 4
#define NUM_MODELS 4

void display(void);
void keyboard(unsigned char, int, int);
void reshape(int, int);
double absd(double);
int near(double, double, double);
void timer(int);
void menu(int);
void setupModels(void);

/*
        GL constants: (GL_...)
        TRUE	1
        FALSE	0
*/
/*
        GL functions: (gl...)
        Translate(x,y,z)		Matrix um (x,y,z) verschieben
        Rotate(w,x,y,z)			Matrix um w° im Verhältnis (x,y,z)
   rotieren Scale(x,y,z)			Matrix um den Faktor (x,y,z)
   skalieren
*/
#define X 0
#define Y 1
#define Z 2
#define W 3
struct cube {
    float angleX, angleY, angleZ;
    float x, y, z;
};
/* velocity "epsilon" (for smooth movement) */
#define VE ((1 << 7) * 1.5)
struct ball {
    float pos[3];
    double v[3];
};
struct curve {
    float c[3][3][3]; /* 3x3 control points, 3 dimensions each */
    float angle[3];
    int showC;
};
#define XY 0
#define YZ 1
#define XZ 2
#define XW 3
#define YW 4
#define ZW 5
struct hmodel {
    int m, proj;
    float cubeV[16][4];   /* hypercube: 16 vertices */
    int cubeE[32][2];     /* hypercube: 32 edges */
    int cubeF[24][4];     /* hypercube: 24 faces, 4 vertices/face */
    float simplexV[5][4]; /* simplex: 5 vertices */
    int simplexE[10][2];  /* simplex: 10 edges */
    int simplexF[10][3];  /* simplex: 10 faces, 3 vertices/face */
    int numV, numE, numF;
    float v[32][4];       /* current vertices */
    double angle[6];      /* angle in radians (xy,yz,xz,wx,wy,wz plane)*/
    float rotation[4][4]; /* (unused) */
};
struct quad {
    float angle[3];
};

struct env {
    int lighting, depth_test, cull_face, line;
    int anim;
};
static int mainWindow;
static float width, height;
static int stereo = 1, axes = 0;
static GLfloat lightAmbientIntensity[] = {0.8f, 0.3f, 0.3f, 1.0f},
               lightDiffuseIntensity[] = {0.3f, 0.3f, 0.3f, 1.0f},
               lightSpecularIntensity[] = {1.0f, 1.0f, 1.0f, 1.0f},
/*ambient: Umgebung, diffuse: gerichtet, specular: spiegelnd*/
#if lighting_version == 1
               lightRed[] = {1.0, 0.0, 0.0, 1.0},
               lightBlue[] = {0.0, 0.0, 1.0, 1.0},
#endif
               lightPosition[] = {0, 0, 0, 1}, /*mehr licht!*/
    materialAmbientIntensity[] = {0.9f, 0.8f, 0.8f,
                                  1.0f}, /*reflection of R/G/B parts of light*/
    materialShininess[] = {50.0f};
static struct cube c = {0, 0, 0, 0, 0, 0};
static struct ball b[NumBalls];
static struct curve cv = {{{{-4., -5., -10.}, {-17., 5., -5.}, {8., 3., -12.}},
                           {{-7., 6., 3.}, {2., -4., 7.}, {0., -2., 5.}},
                           {{12., 3., 6.}, {1., -3., 9.}, {2., 0., 4.}}},
                          {0, 0, 0},
                          0};
static struct hmodel h = {
    0,
    0,
    {/* hypercube vertices */
     {-1, -1, 1, -1},
     {1, -1, 1, -1},
     {-1, 1, 1, -1},
     {1, 1, 1, -1},
     {-1, -1, -1, -1},
     {1, -1, -1, -1},
     {-1, 1, -1, -1},
     {1, 1, -1, -1},
     {-1, -1, 1, 1},
     {1, -1, 1, 1},
     {-1, 1, 1, 1},
     {1, 1, 1, 1},
     {-1, -1, -1, 1},
     {1, -1, -1, 1},
     {-1, 1, -1, 1},
     {1, 1, -1, 1}},
    {/* hypercube edges */
     {0, 1},   {0, 2},   {0, 4},   {0, 8},  {1, 3},   {1, 5},   {1, 9},
     {2, 3},   {2, 6},   {2, 10},  {3, 7},  {3, 11},  {4, 5},   {4, 6},
     {4, 12},  {5, 7},   {5, 13},  {6, 7},  {6, 14},  {7, 15},  {8, 9},
     {8, 10},  {8, 12},  {9, 11},  {9, 13}, {10, 11}, {10, 14}, {11, 15},
     {12, 13}, {12, 14}, {13, 15}, {14, 15}},
    {/* hypercube faces (Drehsinn beachten wegen GL_QUADS!) */
     {0, 1, 3, 2},    {0, 1, 5, 4},    {0, 2, 6, 4},     {0, 1, 9, 8},
     {0, 2, 10, 8},   {0, 4, 12, 8},   {1, 3, 7, 5},     {1, 3, 11, 9},
     {1, 5, 13, 9},   {2, 3, 7, 6},    {2, 3, 11, 10},   {2, 6, 14, 10},
     {3, 7, 15, 11},  {4, 5, 7, 6},    {4, 5, 13, 12},   {4, 6, 14, 12},
     {5, 7, 15, 13},  {6, 7, 15, 14},  {8, 9, 11, 10},   {8, 9, 13, 12},
     {8, 10, 14, 12}, {9, 11, 15, 13}, {10, 11, 15, 14}, {12, 13, 15, 14}},
    {/* simplex vertices */
     {-1, -1, -1, -1},
     {1, -1, -1, -1},
     {0, 0.732, -1, -1},
     {0, -0.423, 0.732, -1},
     {0, -0.423, -0.567, 0.732}},
    {/* simplex edges */
     {0, 1},
     {0, 2},
     {0, 3},
     {0, 4},
     {1, 2},
     {1, 3},
     {1, 4},
     {2, 3},
     {2, 4},
     {3, 4}},
    {/* simplex faces */
     {0, 1, 2},
     {0, 1, 3},
     {0, 1, 4},
     {0, 2, 3},
     {0, 2, 4},
     {0, 3, 4},
     {1, 2, 3},
     {1, 2, 4},
     {1, 3, 4},
     {2, 3, 4}},
    16,
    32,
    24,
    {{0}},
    {0, 0, 0, 0, 0, 0},
    {{1, 0, 0, 0},
     {0, 1, 0, 0},
     {0, 0, 1, 0},
     {0, 0, 0, 1}} /* identity matrix */
};
static struct quad q = {{0}};
static int firstAnim = 1;
static int selector = CUBE, i, j;
static struct env e = {1, 1, 1, 0, 0};

void display(void) {
    int a = 0;
    float x1, y1, z1, w1, x2, y2, z2, w2, x3, y3, z3, w3, x4, y4, z4, w4, x5,
        y5, z5, w5, mu;
    float fov = 60.0 * PI / 180.0; /* vertical(!) field of view in radians */
    /*double conv=width/(2*tan(fov*(atan(1.0)/45.0)/2.0));*/ /*(OLD)
                                                                convergence*/
    float znear = -0.1, zscreen = -10.0, zfar = -100.0,
          eye = 0.0; /*zscreen: convergence*/
    float dnear = -znear, dscreen = -zscreen, dfar = -zfar;
    float w = 2 * dscreen * tan(fov / 2) *
              (width / height); /* virtual screen width */
    float sep = 1.4 / w;        /* eye separation - tune to adjust parallax */
    GLUquadricObj *quad;
    glClear(GL_COLOR_BUFFER_BIT);
    if (!stereo)
        glClear(GL_DEPTH_BUFFER_BIT);
    if (e.lighting)
        glEnable(GL_LIGHTING);
    else
        glDisable(GL_LIGHTING);
    if (e.depth_test)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);
    if (e.cull_face)
        glEnable(GL_CULL_FACE);
    else
        glDisable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK, e.line ? GL_LINE : GL_FILL);

    if (axes) {
        glDisable(GL_LIGHTING);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glTranslatef(0, 0, zscreen);
        glBegin(GL_LINES);
        glColor4f(1., 0., 0., 1.);
        glVertex3f(-10.0, 0.0, 0.0);
        glVertex3f(10.0, 0.0, 0.0);

        glColor4f(0., 1., 0., 1.);
        glVertex3f(0.0, -10.0, 0.0);
        glVertex3f(0.0, 10.0, 0.0);

        glColor4f(0., 0., 1., 1.);
        glVertex3f(0.0, 0.0, znear);
        glVertex3f(0.0, 0.0, zfar);
        glEnd();
        /*glColor4fv(materialAmbientIntensity);*/
        if (e.lighting)
            glEnable(GL_LIGHTING);
    }

    if (stereo) {
        goto red;
    red:
#if lighting_version == 1
        glEnable(GL_LIGHT0);
#endif
        glClear(GL_DEPTH_BUFFER_BIT);
        glColorMask(1, 0, 0, 1);
        a = 0;
        eye = sep / 2;
        goto render;
    blue:
#if lighting_version == 1
        glEnable(GL_LIGHT1);
#endif
        glClear(GL_DEPTH_BUFFER_BIT);
        glColorMask(0, 1, 1, 1);
        a = 1;
        eye = -sep / 2;
        goto render;
    }
render:
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
/* - Perspektive */
/*glFrustum(-znear*(width-eye)/(2*conv),znear*(width+eye)/(2*conv),
        -znear*height/(2*conv),znear*height/(2*conv),
        znear,zfar);*/
/* l1/(w/2-sep/2) = l2/(w/2+sep/2) = dnear/dscreen (für r1/2 analog)*/
#ifdef toedIn
    glFrustum(-(w / 2) * dnear / dscreen, (w / 2) * dnear / dscreen,
              -dnear * tan(fov / 2), dnear * tan(fov / 2), dnear, dfar); /*?*/
#else
    glFrustum(-(w / 2 - eye) * dnear / dscreen, (w / 2 + eye) * dnear / dscreen,
              -dnear * tan(fov / 2), dnear * tan(fov / 2), dnear, dfar);
#endif
#ifdef debugFrustum
    printf("tan(fov/2)=%f, w=%f, eye=%f, dnear=%f, dscreen=%f -> %lf %lf %lf "
           "%lf %lf %lf\n",
           tan(fov / 2), w, eye, dnear, dscreen,
           -(w / 2 - eye) * dnear / dscreen, (w / 2 + eye) * dnear / dscreen,
           -dnear * tan(fov / 2), dnear * tan(fov / 2), dnear, dfar);
#endif
    /* 	Frustum: Pyramidenstumpf
            glFrustum(left,right,bottom,top,near,far);

                            ||		(znear)		_	_
<top
>>>		|	–	||				|
|
                            ||	  	left>	|			| <right
^Auge	^znear					|	_	_	|
                            ^zfar
<bottom
    */
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(eye, 0.0, 0.0); /*Auge verschieben*/
    glTranslatef(0, 0, zscreen);
    if (selector == CUBE) {
        glPushMatrix();
        glTranslatef(c.x, c.y, c.z);
        glRotatef(c.angleX, 1.0, 0.0, 0.0);
        glRotatef(c.angleY, 0.0, 1.0, 0.0);
        glRotatef(c.angleZ, 0.0, 0.0, 1.0);
        glutSolidCube(1.0);
        glPopMatrix();
    } else if (selector == BALLS) {
        for (i = 0; i < NumBalls; i++) {
            glPushMatrix();
            glTranslatef(b[i].pos[X], b[i].pos[Y], b[i].pos[Z]);
            glutSolidSphere(1.0, 50, 50);
            glPopMatrix();
        }
    } else if (selector == CURVE) {
        glPushMatrix();
        glRotatef(cv.angle[X], 1.0, 0.0, 0.0);
        glRotatef(cv.angle[Y], 0.0, 1.0, 0.0);
        glRotatef(cv.angle[Z], 0.0, 0.0, 1.0);
        /* curve: */
        glMap2f(GL_MAP2_VERTEX_3, /* coords u,v */
                0.0, 1.0, 3, 3,   /* u=[0;1], width=3, order=3*/
                0.0, 1.0, 9, 3,   /* v=[0;1], width=9, order=3*/
                (float *)cv.c);
        glEnable(GL_MAP2_VERTEX_3);
        glMapGrid2f(50, 0.0, 1.0, /*u: in 50 steps from 0 to 1*/
                    50, 0., 1.    /*v: in 50 steps from 0 to 1*/
        );
        glEvalMesh2(e.line ? GL_LINE : GL_FILL, 0,
                    50,   /*evaluate u from step 0 to step 50*/
                    0, 50 /*evaluate v from step 0 to step 50*/
        );
        if (cv.showC) {
            for (i = 0; i < 3; i++) {
                for (j = 0; j < 3; j++) {
                    glPushMatrix();
                    glTranslatef(cv.c[i][j][0], cv.c[i][j][1], cv.c[i][j][2]);
                    glutSolidSphere(0.1, 10, 10);
                    glPopMatrix();
                }
            }
        }
#ifdef curve_v1
        for (j = 0; j < 10; j++) {
            glBegin(e.line ? GL_LINE_STRIP : GL_POLYGON);
            for (i = 0; i < 50; i++)
                glEvalCoord2f(0.02 * i, 0.1 * j);
            glEnd();
            glBegin(e.line ? GL_LINE_STRIP : GL_POLYGON);
            for (i = 0; i < 50; i++)
                glEvalCoord2f(0.1 * j, 0.02 * i);
            glEnd();
        }
#endif
        glPopMatrix();
    } else if (selector == HYPER) {
        /*glRotatef(h.angle[XY],0,0,1.0);*/
        for (i = 0; i < h.numV; i++) {
            /* 6 Rotationen, jeweils alternierend x1/x2, y1/y2 usw. verwenden */
            if (h.m == 0) {
                x2 = h.cubeV[i][X];
                y2 = h.cubeV[i][Y];
                z2 = h.cubeV[i][Z];
                w2 = h.cubeV[i][W];
            } else if (h.m == 1) {
                x2 = h.simplexV[i][X];
                y2 = h.simplexV[i][Y];
                z2 = h.simplexV[i][Z];
                w2 = h.simplexV[i][W];
            }
            /*printf("[P%d,0]
             * x=%2.2f,y=%2.2f,z=%2.2f,w=%2.2f\n",i,x2,y2,y2,w2);*/
            /* rotation around xy plane: */
            x1 = (cos(h.angle[XY]) * x2 + sin(h.angle[XY]) * y2);
            y1 = (-sin(h.angle[XY]) * x2 + cos(h.angle[XY]) * y2);
            z1 = z2;
            w1 = w2;
            /* rotation around yz plane: */
            y2 = (cos(h.angle[YZ]) * y1 + sin(h.angle[YZ]) * z1);
            z2 = (-sin(h.angle[YZ] * y1) + cos(h.angle[YZ]) * z1);
            x2 = x1;
            w2 = w1;
            /* rotation around xz plane: */
            x1 = (cos(h.angle[XZ]) * x2 - sin(h.angle[XZ]) * z2);
            z1 = (sin(h.angle[XZ]) * x2 + cos(h.angle[XZ]) * z2);
            y1 = y2;
            w1 = w2;
            /*printf("[P%d,1]
             * x=%2.2f,y=%2.2f,z=%2.2f,w=%2.2f\n",i,x1,y1,y1,w1);*/

            /* rotation around xw plane: */
            x2 = (cos(h.angle[XW]) * x1 + sin(h.angle[XW]) * w1);
            w2 = (-sin(h.angle[XW]) * x1 + cos(h.angle[XW]) * w1);
            y2 = y1;
            z2 = z1;
            /* rotation around yw plane: */
            y1 = (cos(h.angle[YW]) * y2 - sin(h.angle[YW]) * w2);
            w1 = (sin(h.angle[YW]) * y2 + cos(h.angle[YW]) * w2);
            x1 = x2;
            z1 = z2;
            /* rotation around zw plane: */
            z2 = (cos(h.angle[ZW]) * z1 - sin(h.angle[ZW]) * w1);
            w2 = (sin(h.angle[ZW]) * z1 + cos(h.angle[ZW]) * w1);
            x2 = x1;
            y2 = y1;
#define T 1
            h.v[i][X] = x2 * T;
            h.v[i][Y] = y2 * T;
            h.v[i][Z] = z2 * T;
            h.v[i][W] = w2 * T;
            /*printf("[P%d] x=%2.2f,y=%2.2f,z=%2.2f,w=%2.2f\n",i,x2,y2,y2,w2);*/
        }

        glTranslatef(0, 0, 0);
        if (e.line) {
            glBegin(GL_LINES);
            for (i = 0; i < h.numE; i++) {
                /* draw edges */
                if (h.m == 0) {
                    x1 = h.v[h.cubeE[i][0]][X];
                    y1 = h.v[h.cubeE[i][0]][Y];
                    z1 = h.v[h.cubeE[i][0]][Z];
                    w1 = h.v[h.cubeE[i][0]][W];
                    x2 = h.v[h.cubeE[i][1]][X];
                    y2 = h.v[h.cubeE[i][1]][Y];
                    z2 = h.v[h.cubeE[i][1]][Z];
                    w2 = h.v[h.cubeE[i][1]][W];
                } else if (h.m == 1) {
                    x1 = h.v[h.simplexE[i][0]][X];
                    y1 = h.v[h.simplexE[i][0]][Y];
                    z1 = h.v[h.simplexE[i][0]][Z];
                    w1 = h.v[h.simplexE[i][0]][W];
                    x2 = h.v[h.simplexE[i][1]][X];
                    y2 = h.v[h.simplexE[i][1]][Y];
                    z2 = h.v[h.simplexE[i][1]][Z];
                    w2 = h.v[h.simplexE[i][1]][W];
                }
                if (h.proj == 0) {
                    glVertex3f(x1, y1, z1);
                    glVertex3f(x2, y2, z2);
                } else {
                    /* hollasch: clip edges with negative w signs */
                    if (w1 > 0.001 && w2 > 0.001) {
                        glVertex4f(x1, y1, z1, w1);
                        glVertex4f(x2, y2, z2, w2);
#ifdef debugEdges
                        printf("[E%d] (%.2f,%.2f,%.2f,%.2f) - "
                               "(%.2f,%.2f,%.2f,%.2f)\n",
                               i, x1, y1, z1, w1, x2, y2, z2, w2);
#endif
                    } else if (w1 > 0.001 || w2 > 0.001) {
                        /*project to w=0 hyperplane*/
                        mu = (0.001 - w1) / (w2 - w1);
                        x3 = (1 - mu) * x1 + mu * x2;
                        y3 = (1 - mu) * y1 + mu * y2;
                        z3 = (1 - mu) * z1 + mu * z2;
                        w3 = 0.001;
                        if (w1 > 0.001) {
                            glVertex4f(x1, y1, z1, w1);
                            glVertex4f(x3, y3, z3, w3);
                        } else {
                            glVertex4f(x3, y3, z3, w3);
                            glVertex4f(x2, y2, z2, w2);
                        }
                    }
                }
            }
            glEnd();
        } else {
            if (h.m == 0)
                glBegin(GL_QUADS);
            else if (h.m == 1)
                glBegin(GL_TRIANGLES);
            for (i = 0; i < h.numF; i++) {
                if (h.m == 0) {
                    x1 = h.v[h.cubeF[i][0]][X];
                    y1 = h.v[h.cubeF[i][0]][Y];
                    z1 = h.v[h.cubeF[i][0]][Z];
                    w1 = h.v[h.cubeF[i][0]][W];
                    x2 = h.v[h.cubeF[i][1]][X];
                    y2 = h.v[h.cubeF[i][1]][Y];
                    z2 = h.v[h.cubeF[i][1]][Z];
                    w2 = h.v[h.cubeF[i][1]][W];
                    x3 = h.v[h.cubeF[i][2]][X];
                    y3 = h.v[h.cubeF[i][2]][Y];
                    z3 = h.v[h.cubeF[i][2]][Z];
                    w3 = h.v[h.cubeF[i][2]][W];
                    x4 = h.v[h.cubeF[i][3]][X];
                    y4 = h.v[h.cubeF[i][3]][Y];
                    z4 = h.v[h.cubeF[i][3]][Z];
                    w4 = h.v[h.cubeF[i][3]][W];
                } else if (h.m == 1) {
                    w1 = h.v[h.simplexF[i][0]][W];
                    w2 = h.v[h.simplexF[i][1]][W];
                    w3 = h.v[h.simplexF[i][2]][W];
                }
                if (h.proj == 0) {
                    if (h.m == 0) {
                        glVertex3fv(h.v[h.cubeF[i][0]]);
                        glVertex3fv(h.v[h.cubeF[i][1]]);
                        glVertex3fv(h.v[h.cubeF[i][2]]);
                        glVertex3fv(h.v[h.cubeF[i][3]]);
                    } else if (h.m == 1) {
                        glVertex3fv(h.v[h.simplexF[i][0]]);
                        glVertex3fv(h.v[h.simplexF[i][1]]);
                        glVertex3fv(h.v[h.simplexF[i][2]]);
                    }
                } else {
                    /*TODO better clipping*/
                    if (h.m == 0) {
                        if (w1 > 0.001 && w2 > 0.001 && w3 > 0.001 &&
                            w4 > 0.001) {
                            glVertex4f(x1, y1, z1, w1);
                            glVertex4f(x2, y2, z2, w1);
                            glVertex4f(x3, y3, z3, w3);
                            glVertex4f(x4, y4, z4, w4);
                        }
#define quadClipping_v2
#ifdef quadClipping_v2
                        if (w1 > 0.001) {
                            glVertex4f(x1, y1, z1, w1);
                        } else {
                            if (w2 > 0.001) {
                                mu = (0.001 - w2) / (w1 - w2);
                                x5 = (1 - mu) * x2 + mu * x1;
                                y5 = (1 - mu) * y2 + mu * y1;
                                z5 = (1 - mu) * z2 + mu * z1;
                                w5 = 0.001;
                                glVertex4f(x5, y5, z5, w5);
                            } else if (w4 > 0.001) {
                                mu = (0.001 - w4) / (w1 - w4);
                                x5 = (1 - mu) * x4 + mu * x1;
                                y5 = (1 - mu) * y4 + mu * y1;
                                z5 = (1 - mu) * z4 + mu * z1;
                                w5 = 0.001;
                                glVertex4f(x5, y5, z5, w5);
                            }
                        }
                        if (w2 > 0.001) {
                            glVertex4f(x2, y2, z2, w2);
                        } else {
                            if (w3 > 0.001) {
                                mu = (0.001 - w3) / (w2 - w3);
                                x5 = (1 - mu) * x3 + mu * x2;
                                y5 = (1 - mu) * y3 + mu * y2;
                                z5 = (1 - mu) * z3 + mu * z2;
                                w5 = 0.001;
                                glVertex4f(x5, y5, z5, w5);
                            } else if (w1 > 0.001) {
                                mu = (0.001 - w1) / (w2 - w1);
                                x5 = (1 - mu) * x1 + mu * x2;
                                y5 = (1 - mu) * y1 + mu * y2;
                                z5 = (1 - mu) * z1 + mu * z2;
                                w5 = 0.001;
                                glVertex4f(x5, y5, z5, w5);
                            }
                        }
                        if (w3 > 0.001) {
                            glVertex4f(x3, y3, z3, w3);
                        } else {
                            if (w2 > 0.001) {
                                mu = (0.001 - w2) / (w3 - w2);
                                x5 = (1 - mu) * x2 + mu * x3;
                                y5 = (1 - mu) * y2 + mu * y3;
                                z5 = (1 - mu) * z2 + mu * z3;
                                w5 = 0.001;
                                glVertex4f(x5, y5, z5, w5);
                            } else if (w4 > 0.001) {
                                mu = (0.001 - w4) / (w3 - w4);
                                x5 = (1 - mu) * x4 + mu * x3;
                                y5 = (1 - mu) * y4 + mu * y3;
                                z5 = (1 - mu) * z4 + mu * z3;
                                w5 = 0.001;
                                glVertex4f(x5, y5, z5, w5);
                            }
                        }
                        if (w4 > 0.001) {
                            glVertex4f(x4, y4, z4, w4);
                        } else {
                            if (w3 > 0.001) {
                                mu = (0.001 - w3) / (w4 - w3);
                                x5 = (1 - mu) * x3 + mu * x4;
                                y5 = (1 - mu) * y3 + mu * y4;
                                z5 = (1 - mu) * z3 + mu * z4;
                                w5 = 0.001;
                                glVertex4f(x5, y5, z5, w5);
                            } else if (w1 > 0.001) {
                                mu = (0.001 - w1) / (w4 - w1);
                                x5 = (1 - mu) * x1 + mu * x4;
                                y5 = (1 - mu) * y1 + mu * y4;
                                z5 = (1 - mu) * z1 + mu * z4;
                                w5 = 0.001;
                                glVertex4f(x5, y5, z5, w5);
                            }
                        }
#endif
                    } else if (h.m == 1) {
                        glVertex4fv(h.v[h.simplexF[i][0]]);
                        glVertex4fv(h.v[h.simplexF[i][1]]);
                        glVertex4fv(h.v[h.simplexF[i][2]]);
                    }
                }
            }
            glEnd();
        }
    } else if (selector == QUAD) {
        glPushMatrix();
        glRotatef(q.angle[X], 1.0, 0.0, 0.0);
        glRotatef(q.angle[Y], 0.0, 1.0, 0.0);
        glRotatef(q.angle[Z], 0.0, 0.0, 1.0);
        quad = gluNewQuadric();
        gluQuadricDrawStyle(quad, GLU_FILL);
        gluQuadricNormals(quad, GLU_SMOOTH);
        gluCylinder(quad, 2.0, 2.0, 4.0, 20, 20);
        glPopMatrix();
    }
#if lighting_version == 2
    if (stereo) {
        if (a == 0)
            goto blue;
        glColorMask(1, 1, 1, 1);
    }
#endif
#if lighting_version == 1
    if (stereo) {
        if (a == 0) {
            glDisable(GL_LIGHT0);
            goto blue;
        } else if (a == 1) {
            glDisable(GL_LIGHT1);
        }
        glColorMask(1, 1, 1, 1);
    }
#endif
    /*after render:*/
    glutSwapBuffers(); /*benötigt wegen double buffering*/
}
void keyboard(unsigned char p1, int p2, int p3) {
    switch (p1) {
    case '1':
        selector = CUBE;
        e.line = 0;
        break;
    case '2':
        selector = BALLS;
        e.line = 0;
        break;
    case '3':
        selector = CURVE;
        e.line = 1; /* looks better */
        break;
    case '4':
        selector = HYPER;
        e.line = 1;
        break;
    case '5':
        selector = QUAD;
        e.line = 0;
        break;
    case 'M':
        stereo = !stereo;
        break;
    case 'L':
        e.lighting = !e.lighting;
        break;
    case 'D':
        e.depth_test = !e.depth_test;
        break;
    case 'C':
        e.cull_face = !e.cull_face;
        break;
    case 'W':
        e.line = !e.line;
        break;
    }
    if (selector == CUBE) {
        switch (p1) {
        case 'w':
            c.angleY = fmod(c.angleY - 5.0, -360.0);
            break;
        case 's':
            c.angleY = fmod(c.angleY + 5.0, 360.0);
            break;
        case 'a':
            c.angleX = fmod(c.angleX - 5.0, -360.0);
            break;
        case 'd':
            c.angleX = fmod(c.angleX + 5.0, 360.0);
            break;
        case 'q':
            c.angleZ = fmod(c.angleZ - 5.0, -360.0);
            break;
        case 'e':
            c.angleZ = fmod(c.angleZ + 5.0, 360.0);
            break;
        case 'r':
            c.z += 0.1;
            break;
        case 'f':
            c.z -= 0.1;
            break;
        case 'G':
            e.anim = !e.anim;
            glutTimerFunc(5, timer, 0);
            break;
        }
    } else if (selector == BALLS) {
        switch (p1) {
        case 'G':
            if (e.anim == 0) {
                if (firstAnim) {
                    /*b[0].v[X]=(b[1].pos[X]-b[0].pos[X])/VE;
                    b[0].v[Y]=(b[1].pos[Y]-b[0].pos[Y])/VE;
                    b[0].v[Z]=(b[1].pos[Z]-b[0].pos[Z])/VE;*/
                    b[0].v[X] = -8.0 / VE;
                    b[0].v[Y] = -6.0 / VE;
                    b[0].v[Z] = 8.0 / VE;
                    b[1].v[X] = 1.0 / VE;
                    b[1].v[Y] = 3.0 / VE;
                    b[1].v[Z] = -6.0 / VE;
                    firstAnim = 0;
                }
                e.anim = 1;
                glutTimerFunc(5, timer, 0);
            } else {
                /*b[0].v[X]=0;
                b[0].v[Y]=0;
                b[0].v[Z]=0;*/
                e.anim = 0;
            }
            break;
        }
    } else if (selector == CURVE) {
        switch (p1) {
        case 'w':
            cv.angle[Y] = fmod(cv.angle[Y] - 5.0, -360.0);
            break;
        case 's':
            cv.angle[Y] = fmod(cv.angle[Y] + 5.0, 360.0);
            break;
        case 'a':
            cv.angle[X] = fmod(cv.angle[X] - 5.0, -360.0);
            break;
        case 'd':
            cv.angle[X] = fmod(cv.angle[X] + 5.0, 360.0);
            break;
        case 'q':
            cv.angle[Z] = fmod(cv.angle[Z] - 5.0, -360.0);
            break;
        case 'e':
            cv.angle[Z] = fmod(cv.angle[Z] + 5.0, 360.0);
            break;
        case 'P':
            cv.showC = !cv.showC;
            break;
        }
    } else if (selector == HYPER) {
        switch (p1) {
#define dAngle 3.0
        case 'q':
            h.angle[XY] = fmod(h.angle[XY] + dAngle * PI / 180.0, 2 * PI);
            break;
        case 'w': /* TODO fix: yz plane lag?? */
            h.angle[YZ] = fmod(h.angle[YZ] + dAngle * PI / 180.0, 2 * PI);
            break;
        case 'e':
            h.angle[XZ] = fmod(h.angle[XZ] + dAngle * PI / 180.0, 2 * PI);
            break;
        case 'a':
            h.angle[XW] = fmod(h.angle[XW] + dAngle * PI / 180.0, 2 * PI);
            break;
        case 's':
            h.angle[YW] = fmod(h.angle[YW] + dAngle * PI / 180.0, 2 * PI);
            break;
        case 'd':
            h.angle[ZW] = fmod(h.angle[ZW] + dAngle * PI / 180.0, 2 * PI);
            break;
        case 'P':
            h.proj = !h.proj;
            break;
        case 'H':
            h.m = !h.m;
            if (h.m == 0) {
                h.numV = 16;
                h.numE = 32;
                h.numF = 24;
            } else if (h.m == 1) {
                h.numV = 5;
                h.numE = 10;
                h.numF = 10;
            }
            break;
        }
    } else if (selector == QUAD) {
        switch (p1) {
        case 'w':
            q.angle[Y] -= 15.0;
            while (q.angle[Y] < 0)
                q.angle[Y] += 360.0;
            break;
        case 's':
            q.angle[Y] += 15.0;
            while (q.angle[Y] > 360)
                q.angle[Y] -= 360.0;
            break;
        case 'a':
            q.angle[X] -= 15.0;
            while (q.angle[X] < 0)
                q.angle[X] += 360.0;
            break;
        case 'd':
            q.angle[X] += 15.0;
            while (q.angle[X] > 360)
                q.angle[X] -= 360.0;
            break;
        case 'q':
            q.angle[Z] -= 15.0;
            while (q.angle[Z] < 0)
                q.angle[Z] += 360.0;
            break;
        case 'e':
            q.angle[Z] += 15.0;
            while (q.angle[Z] > 360)
                q.angle[Z] -= 360.0;
            break;
        }
    }
    glutPostRedisplay();

#if defined(__unix) || defined(__unix__)
    printf("\e[1;1H\e[2J"); /*clear screen*/
#elif defined(_WIN32)
/* just imagine the screen clearing */
#endif
    if (selector == CUBE) {
        printf("\n\n------\n Cube \n------\n");
        /*printf("\n Current data:\n");
        printf("\tX/Y/Z
        angle:\t\t%3.2f\t%3.2f\t%3.2f\n",c.angleX,c.angleY,c.angleZ);
        printf("\tZ pos:\t\t\t%3.2f\n",c.z);*/
    } else if (selector == BALLS) {
        printf("\n\n-------\n Balls \n-------\n");
        printf("\n Current data:\n");
        for (i = 0; i < NumBalls; i++) {
            printf("  Ball object #%d\n", i);
            printf("\tX/Y/Z pos:\t\t%3.2f\t%3.2f\t%3.2f\n", b[i].pos[X],
                   b[i].pos[Y], b[i].pos[Z]);
            printf("\tX/Y/Z velocity:\t\t%3.2f\t%3.2f\t%3.2f\n", b[i].v[X] * VE,
                   b[i].v[Y] * VE, b[i].v[Z] * VE);
        }
    } else if (selector == CURVE) {
        printf("\n\n-------\n Curve \n-------\n");
        /*printf("\n Current data:\n");
        printf("\tX/Y/Z
        angle:\t\t%3.2f\t%3.2f\t%3.2f\n",cv.angle[X],cv.angle[Y],cv.angle[Z]);*/
        /*printf("\tMAP1_VERTEX_3: %d \n",glIsEnabled(GL_MAP1_VERTEX_3));*/

    } else if (selector == HYPER) {
        printf("\n\n------------\n Hyperspace \n------------\n");
        printf("\n Current data:\n");
        printf("\tXY/YZ/XZ angle:\t\t%3.2f\t%3.2f\t%3.2f\n", h.angle[XY],
               h.angle[YZ], h.angle[XZ]);
        printf("\tXW/YW/ZW angle:\t\t%3.2f\t%3.2f\t%3.2f\n", h.angle[XW],
               h.angle[YW], h.angle[ZW]);
    }
    printf("\n User Input:\n");
    printf("\tMode (M):\t\t%s\n", stereo ? "STEREO" : "MONO");
    printf("\tLighting (L):\t\t%i\n\tDepth test (D):\t\t%i\n\tCull face "
           "(C):\t\t%i\n\tWireframe (W):\t\t%i\n",
           e.lighting, e.depth_test, e.cull_face, e.line);
    if (selector == CUBE) {
        printf("\tRotate X/Y/Z axis:\ta/d\tw/s\tq/e\n");
        printf("\t  (X/Y/Z angle:)\t%3.2f\t%3.2f\t%3.2f\n", c.angleX, c.angleY,
               c.angleZ);
        printf("\tTranslate Z:\t\tr/f\n");
        printf("\t  (Z pos:)\t\t%3.2f\n", c.z);
        printf("\tAuto-rotate (G):\t%i\n", e.anim);
    } else if (selector == BALLS) {
        printf("\tAnimation (G):\t\t%i\n", e.anim);
    } else if (selector == CURVE) {
        printf("\tRotate X/Y/Z axis:\ta/d\tw/s\tq/e\n");
        printf("\t  (X/Y/Z angle:)\t%3.2f\t%3.2f\t%3.2f\n", cv.angle[X],
               cv.angle[Y], cv.angle[Z]);
        printf("\tControl points (P):\t%i\n", cv.showC);
    } else if (selector == HYPER) {
        printf("\tRotate XY/YZ/XZ plane:\tq\tw\te\n");
        printf("\tRotate XW/YW/ZW plane:\ta\ts\td\n");
        printf("\tProjection (P):\t\t%s\n",
               h.proj == 0 ? "parallel" : "perspective");
        printf("\t4D Model (H):\t\t%s\n",
               h.m == 0 ? "tesseract" : h.m == 1 ? "pentachoron" : "unknown");
    }
    printf("\n\t--- switch models with 1/2/3/4 (cube/balls/curve/hyperspace) "
           "---\n");
    printf("\n");
}
void reshape(int w, int h) {
    width = w;
    height = h;
    glViewport(0, 0, (GLsizei)width, (GLsizei)height);
}
double absd(double p) { return p < 0 ? -p : p; }
int near(double p1, double p2, double delta) { return absd(p1 - p2) < delta; }
void timer(int p) {
    int a1, a2;
    double b1x, b1y, b1z, b2x, b2y, b2z;
    double cr = 0.95;
    double cd = 0.2;
    if (selector == CUBE && e.anim) {
        c.angleY += 1.0;
        while (c.angleY > 360)
            c.angleY -= 360.0;
    } else if (selector == BALLS && e.anim) {
        for (a1 = 1; a1 < NumBalls; a1++) {
            for (a2 = 0; a2 < a1; a2++) {
                /*if(a1>=a2) continue;*/
                if (near(b[a1].pos[X], b[a2].pos[X], 1.0) &&
                    near(b[a1].pos[Y], b[a2].pos[Y], 1.0) &&
                    near(b[a1].pos[Z], b[a2].pos[Z], 1.0)) {
/* collision a2->a1 */
#if collision_version == 3
                    /* m1=0.5, m2=0.5, CR=0.9 */
                    b1x = b[a1].v[X];
                    b1y = b[a1].v[Y];
                    b1z = b[a1].v[Z];
                    b2x = b[a2].v[X];
                    b2y = b[a2].v[Y];
                    b2z = b[a2].v[Z];
                    b[a1].v[X] = b1x * 0.5 + b2x * 0.5 + 0.5 * cr * (b2x - b1x);
                    b[a1].v[Y] = b1y * 0.5 + b2y * 0.5 + 0.5 * cr * (b2y - b1y);
                    b[a1].v[Z] = b1z * 0.5 + b2z * 0.5 + 0.5 * cr * (b2z - b1z);
                    b[a2].v[X] = b1x * 0.5 + b2x * 0.5 + 0.5 * cr * (b1x - b2x);
                    b[a2].v[Y] = b1y * 0.5 + b2y * 0.5 + 0.5 * cr * (b1y - b2y);
                    b[a2].v[Z] = b1z * 0.5 + b2z * 0.5 + 0.5 * cr * (b1z - b2z);
#endif
#if collision_version == 2
                    b[a2].v[X] += b[a1].v[X] * 0.7;
                    b[a2].v[Y] += b[a1].v[Y] * 0.7;
                    b[a2].v[Z] += b[a1].v[Z] * 0.7;
                    b[a1].v[X] *= -0.3;
                    b[a1].v[Y] *= -0.3;
                    b[a1].v[Z] *= -0.3;
#endif
#if collision_version == 1
                    b[a2].pos[X] += 0.01;
#endif
                }
            }
        }
        for (i = 0; i < NumBalls; i++) {

            b1x = b[i].v[X];
            b1y = b[i].v[Y];
            b1z = b[i].v[Z];
#if drag_version == 3
            if (!near(b[i].v[X], 0.0, 0.01))
                b[i].v[X] = b1x > 0 ? b1x - cd * (b1x * b1x) / 2
                                    : b1x + cd * (b1x * b1x) / 2;
            if (!near(b[i].v[Y], 0.0, 0.01))
                b[i].v[Y] = b1y > 0 ? b1y - cd * (b1y * b1y) / 2
                                    : b1y + cd * (b1y * b1y) / 2;
            if (!near(b[i].v[Z], 0.0, 0.01))
                b[i].v[Z] = b1z > 0 ? b1z - cd * (b1z * b1z) / 2
                                    : b1z + cd * (b1z * b1z) / 2;
#endif
#if drag_version == 2
            if (!near(b[i].v[X], 0.0, drag))
                b[i].v[X] -= b[i].v[X] > 0 ? drag : -drag;
            else
                b[i].v[X] = 0.0;
            if (!near(b[i].v[Y], 0.0, drag))
                b[i].v[Y] -= b[i].v[Y] > 0 ? drag : -drag;
            else
                b[i].v[Y] = 0.0;
            if (!near(b[i].v[Z], 0.0, drag))
                b[i].v[Z] -= b[i].v[Z] > 0 ? drag : -drag;
            else
                b[i].v[Z] = 0.0;
#endif
#if drag_version == 1
            if (!near(b[i].v[X], 0.0, drag))
                b[i].v[X] = b[i].v[X] > 0 ? b[i].v[X] - drag : b[i].v[X] + drag;
            if (!near(b[i].v[Y], 0.0, drag))
                b[i].v[Y] = b[i].v[Y] > 0 ? b[i].v[Y] - drag : b[i].v[Y] + drag;
            if (!near(b[i].v[Z], 0.0, drag))
                b[i].v[Z] = b[i].v[Z] > 0 ? b[i].v[Z] - drag : b[i].v[Z] + drag;
#endif
            b[i].pos[X] += b[i].v[X];
            b[i].pos[Y] += b[i].v[Y];
            b[i].pos[Z] += b[i].v[Z];
        }
    }
    glutPostRedisplay();
    if (e.anim)
        glutTimerFunc(5, timer, 0);
}
void menu(int p) {
    switch (p) {
    case 0: /*lighting*/
        e.lighting = !e.lighting;
        break;
    case 1: /*depth test*/
        e.depth_test = !e.depth_test;
        break;
    case 2: /*cull face*/
        e.cull_face = !e.cull_face;
        break;
    case 3: /*wireframe*/
        e.line = !e.line;
        break;
    case 4: /*auto-rotate*/
        e.anim = !e.anim;
        break;
    case 5: /*stereo/mono*/
        stereo = !stereo;
        break;
    case 6: /*Achsen*/
        axes = !axes;
        break;
    case 7:
        /*b[0].v[Z]=b[0].v[Z]==0.0?0.01:0.0;*/
        if (e.anim == 0) {
            if (firstAnim) {
                b[0].v[X] = (b[1].pos[X] - b[0].pos[X]) / VE;
                b[0].v[Y] = (b[1].pos[Y] - b[0].pos[Y]) / VE;
                b[0].v[Z] = (b[1].pos[Z] - b[0].pos[Z]) / VE;
                firstAnim = 0;
            }
            e.anim = 1;
        } else
            e.anim = 0;
        break;
    case 8:
        selector = (selector + 1) % NUM_MODELS;
        break;
    case 9:
        cv.showC = !cv.showC;
        break;
    case 10:
        h.proj = !h.proj;
        break;
    case 11:
        h.m = !h.m;
        if (h.m == 0) {
            h.numV = 16;
            h.numE = 32;
            h.numF = 24;
        } else if (h.m == 1) {
            h.numV = 5;
            h.numE = 10;
            h.numF = 10;
        }
        break;
    }
    keyboard('#', -1, -1);
    glutPostRedisplay();
}
void setupModels(void) {
    /* balls: */
    b[0].pos[X] = 3.5;
    b[0].pos[Y] = 4.0;
    b[0].pos[Z] = -3.0;
    b[1].pos[X] = -1.0;
    b[1].pos[Y] = 0.0;
    b[1].pos[Z] = 3.0;
    b[2].pos[X] = 1.0;
    b[2].pos[Y] = 2.0;
    b[2].pos[Z] = 0.0;
    b[3].pos[X] = 0.0;
    b[3].pos[Y] = 2.0;
    b[3].pos[Z] = -1.0;
    b[4].pos[X] = 5.0;
    b[4].pos[Y] = -3.0;
    b[4].pos[Z] = -5.0;
}
int main(int argc, char **argv) {
    int cubeMenu, ballMenu, curveMenu, hyperMenu, envMenu;
    /*window init*/
    glutInit(&argc, argv);
    setupModels();
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    /*RGBA: Modus, DOUBLE: double buffering, DEPTH: Verdeckung*/
    glutInitWindowPosition(400, 100);
    mainWindow = glutCreateWindow("Cube");
    glutReshapeWindow(Width, Height);
    glutSetWindow(mainWindow);

    /*cube init*/
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glDrawBuffer(GL_BACK);
    glEnable(GL_LIGHTING);
#if lighting_version == 2
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbientIntensity);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuseIntensity);
    glLightfv(GL_LIGHT0, GL_SPECULAR,
              lightSpecularIntensity); /*specular reflection*/
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, materialAmbientIntensity);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, materialShininess);
    glEnable(GL_LIGHT0);
#endif
#if lighting_version == 1
#error "lighting v1 is obsolete"
#endif

    /*glut setup*/
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutReshapeFunc(reshape);
    glutTimerFunc(5, timer, 0);
    cubeMenu = glutCreateMenu(menu);
    glutAddMenuEntry("Rotate on/off", 4);
    ballMenu = glutCreateMenu(menu);
    glutAddMenuEntry("Animation on/off", 7);
    curveMenu = glutCreateMenu(menu);
    glutAddMenuEntry("Control points on/off", 9);
    hyperMenu = glutCreateMenu(menu);
    glutAddMenuEntry("Projection par./persp.", 10);
    glutAddMenuEntry("Change 4D model", 11);
    envMenu = glutCreateMenu(menu);
    glutAddMenuEntry("Lighting on/off", 0);
    glutAddMenuEntry("Depth test on/off", 1);
    glutAddMenuEntry("Cull face on/off", 2);
    glutAddMenuEntry("Wireframe on/off", 3);
    glutAddMenuEntry("Axes on/off", 6);
    glutCreateMenu(menu);
    glutAddSubMenu("Cube", cubeMenu);
    glutAddSubMenu("Ball", ballMenu);
    glutAddSubMenu("Curve", curveMenu);
    glutAddSubMenu("Hyperspace", hyperMenu);
    glutAddSubMenu("System", envMenu);
    glutAddMenuEntry("Stereo/Mono", 5);
    glutAddMenuEntry("Switch Model", 8);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
    /*glutCreateMenu(menu);
    glutAddMenuEntry("Lighting on/off",0);
    glutAddMenuEntry("Depth test on/off",1);
    glutAddMenuEntry("Cull face on/off",2);
    glutAddMenuEntry("Wireframe on/off",3);
    glutAddMenuEntry("Rotate on/off",4);
    glutAddMenuEntry("Stereo/Mono",5);
    glutAddMenuEntry("Axes on/off",6);
    glutAttachMenu(GLUT_RIGHT_BUTTON);*/
    keyboard('#', -1, -1);
    glutMainLoop();
    return 0;
}
