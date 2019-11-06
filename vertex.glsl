#version 450 core
in vec3 i_vertex_coord;

void main() {
    gl_Position = vec4(i_vertex_coord, 1);
}
