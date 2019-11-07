#version 450 core
uniform mat4 u_model_transf;
uniform mat4 u_view_transf;
uniform mat4 u_proj_transf;

in vec3 i_vertex_coord;

void main() {
    gl_Position = u_proj_transf * u_view_transf * u_model_transf *
                  vec4(i_vertex_coord, 1);
}
