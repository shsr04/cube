default: cube

Sources := *.cpp
Flags := -std=c++17 -I. -Wall -Wextra -Wconversion
OptFlags := -Ofast -fno-exceptions -ffast-math
DebugFlags := -O0 -g
Libs := glad/glad.o -lSDL2 -lGL -ldl

%: %.cpp
	clang++ $(Flags) $(OptFlags) -o $@ $< $(Libs)

