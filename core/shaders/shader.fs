#version 460 core
out vec4 FragColor;
uniform vec3 waterColor;

void main() {
    FragColor = vec4(waterColor, 1.0);
}