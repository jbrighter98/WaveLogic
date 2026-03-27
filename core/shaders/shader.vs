#version 460 core
layout (location = 0) in vec3 aPos;

struct Wave {
    float amplitude;
    float frequency;
    float speed;
    float steepness; // For Gerstner "peaks"
    float phase; // Updated over time
    vec2 direction;
};

#define MAX_WAVES 16
uniform Wave waves[MAX_WAVES];
uniform int numWaves;

uniform mat4 model, view, projection;

// Data to pass to the fragment shader
out vec3 Normal;
out vec3 FragPos;

void main() {
    vec3 gridPoint = aPos;
    vec3 displacedPos = gridPoint;

    // accumulate normal components
    float tx = 0.0;
    float tz = 0.0;
    float ty = 1.0;

    for(int i = 0; i < numWaves; i++) {
        float dotDir = dot(waves[i].direction, gridPoint.xz);
        float angle = waves[i].frequency * dotDir + waves[i].phase;

        // only do sin/cos once since it is expensive
        float cosA = cos(angle);
        float sinA = sin(angle);
        float wa = waves[i].frequency * waves[i].amplitude;
        
        // Horizontal displacement (creates the "choppy" peaks)
        displacedPos.x += waves[i].steepness * waves[i].amplitude * waves[i].direction.x * cosA;
        displacedPos.z += waves[i].steepness * waves[i].amplitude * waves[i].direction.y * sinA;
        
        // Vertical displacement
        displacedPos.y += waves[i].amplitude * sinA;

        // Normal Derivatives (Match core WaveLogic)
        tx -= waves[i].direction.x * wa * cosA;
        tz -= waves[i].direction.y * wa * cosA;
        ty -= waves[i].steepness * wa * sinA;

    }

    // normalize and transform to world space
    ty = max(ty, 0.1);
    Normal = normalize(mat3(transpose(inverse(model))) * vec3(tx, ty, tz));

    // calculate world position for lighting calculations in the fs
    FragPos = vec3(model * vec4(displacedPos, 1.0));

    gl_Position = projection * view * model * vec4(displacedPos, 1.0);
}