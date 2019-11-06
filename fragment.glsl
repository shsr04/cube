#version 450 core
uniform vec3 u_main_color;
out vec4 o_color;

void main() {
    o_color = vec4(u_main_color, 1.0);
}
