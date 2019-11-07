#version 450 core
uniform vec3 u_main_color;
uniform sampler2D u_sampler;

in vec2 i2_texture_coord;
out vec4 o_color;

void main() {
    o_color = texture(u_sampler, i2_texture_coord) * vec4(u_main_color, 1.);
}
