#version 450 core
in vec3 i_vertex_coord;
in vec2 i_texture_coord;
out vec2 i2_texture_coord;

void main() {
    gl_Position = vec4(i_vertex_coord, 1);
    i2_texture_coord = i_texture_coord;
}
