#version 460 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 waterColor;


void main() {

    // Ambient
    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * vec3(1.0, 1.0, 1.0);

    // Diffuse
    vec3 norm = normalize(Normal);
    //vec3 lightDir = normalize(lightPos - FragPos);
    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.2)); // Light coming straight down
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * vec3(1.0, 1.0, 1.0);

    // Specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = spec * specularStrength * vec3(1.0, 1.0, 1.0);

    vec3 result = (ambient + diffuse + specular) * waterColor;

    FragColor = vec4(result, 1.0);
}