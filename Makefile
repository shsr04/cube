Sources := *.cpp
Deps := glad/libglad.a
Flags := -std=c++17 -I. -Lglad -Lsoil -Wall -Wextra -Wconversion
OptFlags := -Ofast -fno-exceptions -ffast-math
DebugFlags := -O0 -g
Libs := -lSDL2 -lSDL2_image -lglad -lGL -ldl
	#	install: mesa-common-dev libsdl2-dev libsdl2-image-dev
	#	glad etc. are compiled statically like:
	#		cd glad/
	#		clang -c *.c
	#		ar -rcs libglad.a *.o

default: $(Deps) cube 

.PHONY: format

%: %.cpp format
	clang++ $(Flags) $(OptFlags) -o $@ $< $(Libs)

format:
	clang-format -i -style=file *.cpp *.hpp *.glsl
