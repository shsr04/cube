default: cube | glad/libglad.a soil/libSOIL.a

Sources := *.cpp
Flags := -std=c++17 -I. -Lglad -Lsoil -Wall -Wextra -Wconversion
OptFlags := -Ofast -fno-exceptions -ffast-math
DebugFlags := -O0 -g
Libs := -lSDL2 -lglad -lGL -ldl -lSOIL
	#	install: mesa-common-dev libsdl2
	#	glad etc. are compiled statically like:
	#		cd glad/
	#		clang -c *.c
	#		ar -rcs libglad.a *.o

%: %.cpp
	clang++ $(Flags) $(OptFlags) -o $@ $< $(Libs)
