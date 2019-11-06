#version 450 core
uniform vec3 mainColor;
out vec4 color;

void main() {
    color = vec4(mainColor, 1.0);
}
