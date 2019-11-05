default: cube

Sources := *.cpp
Flags := -std=c++17 -Wall -Wextra -Wconversion
OptFlags := -Ofast -fno-exceptions -ffast-math
DebugFlags := -O0 -g
Libs := -lSDL2 -lGL

cube: $(Sources)
	clang++ $(Flags) $(OptFlags) -o cube $(Sources) $(Libs)

run: cube
	./cube
