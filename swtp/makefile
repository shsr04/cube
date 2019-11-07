LIBRARY=./libs
FLAGS=-Wall -std=c99 -pedantic -I$(LIBRARY) -L$(LIBRARY) -ffreestanding
LIB_FLAGS=-lGL -lGLU -lglut -lm
GLUT_DEPS=$(LIBRARY)/GL/GL.H $(LIBRARY)/GL/GLU.H $(LIBRARY)/GL/glut.h \
	 $(LIBRARY)/libGL.so $(LIBRARY)/libGLU.so $(LIBRARY)/libglut.so

cube: cube.c $(GLUT_DEPS)
	gcc $(FLAGS) -o cube cube.c $(LIB_FLAGS)
	./cube

tar:
	tar -cvf swtp.tar *
	gzip swtp.tar

report:
	make -C docs/report all

stereo-all: stereo stereoAC

stereo: samples/redblue_stereo.c $(GLUT_DEPS)
	gcc $(FLAGS) -o samples/redblue_stereo samples/redblue_stereo.c $(LIB_FLAGS)
	./samples/redblue_stereo
	
stereoAC: samples/redblue_stereoAC.c $(GLUT_DEPS)
	gcc $(FLAGS) -o samples/redblue_stereoAC samples/redblue_stereoAC.c $(LIB_FLAGS)
	./samples/redblue_stereoAC

bi: samples/bi_objGL.c samples/bi_objGL.h $(GLUT_DEPS)
	gcc $(FLAGS) -o samples/bi_objGL samples/bi_objGL.c $(LIB_FLAGS)
	./samples/bi_objGL

