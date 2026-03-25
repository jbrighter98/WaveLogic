#version 330 core
layout (location = 0) in vec3 aPos;

struct Wave {
    float amplitude;
    float frequency;
    float speed;
    float steepness; // For Gerstner "peaks"
    float phase; // Updated over time
    vec2 direction;
};

#define MAX_WAVES 8
uniform Wave waves[MAX_WAVES];
uniform int numWaves;

uniform mat4 model, view, projection;

void main() {
    vec3 gridPoint = aPos;
    vec3 displacedPos = gridPoint;

    for(int i = 0; i < numWaves; i++) {
        float dotDir = dot(waves[i].direction, gridPoint.xz);
        float angle = waves[i].frequency * dotDir + waves[i].phase;
        
        // Horizontal displacement (creates the "choppy" peaks)
        displacedPos.x += waves[i].steepness * waves[i].amplitude * waves[i].direction.x * cos(angle);
        displacedPos.z += waves[i].steepness * waves[i].amplitude * waves[i].direction.y * cos(angle);
        
        // Vertical displacement
        displacedPos.y += waves[i].amplitude * sin(angle);
    }

    gl_Position = projection * view * model * vec4(displacedPos, 1.0);
}