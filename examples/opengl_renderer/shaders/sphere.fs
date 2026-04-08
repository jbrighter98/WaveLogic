#version 460 core
in vec3 Normal;
in vec3 FragPos;

uniform vec3 lightDir;   // normalized, world space
uniform vec3 lightColor;
uniform vec3 objectColor;

out vec4 FragColor;

void main() {
    float ambient  = 0.2;
    float diffuse  = max(dot(normalize(Normal), normalize(-lightDir)), 0.0);
    vec3  lighting = (ambient + diffuse) * lightColor;
    FragColor      = vec4(lighting * objectColor, 1.0);
}