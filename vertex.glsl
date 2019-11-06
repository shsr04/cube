#version 450 core
in vec3 vertexCoord;

void main() {
    gl_Position = vec4(vertexCoord, 1);
}
