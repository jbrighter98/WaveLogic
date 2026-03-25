#pragma once
#include <vector>

struct WaveParameters {
    float amplitude;
    float frequency;
    float speed;
    float steepness; // For Gerstner "peaks"
    float directionX, directionZ;
    float phase; // Updated over time
};

class WaveLogic {
public:
    // Updates the phase of all waves based on deltaTime
    static void UpdateWaves(std::vector<WaveParameters>& waves, float deltaTime);

    // The "Single Point Query" for C++ (Buoyancy/Physics)
    static float GetHeightAt(float x, float z, const std::vector<WaveParameters>& waves);
    
    // Returns the offset vector (for Gerstner horizontal displacement)
    static void GetDisplacement(float x, float z, const std::vector<WaveParameters>& waves, float& outX, float& outY, float& outZ);
};