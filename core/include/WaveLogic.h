#ifndef WAVELOGIC_H
#define WAVELOGIC_H

#pragma once
#include <vector>

namespace WaveLogic {

    struct WaveParameters {
        float amplitude;
        float frequency;
        float speed;
        float steepness; // For Gerstner "peaks"
        float directionX, directionZ;
        float phase; // Updated over time
    };

    // Simple POD (Plain Old Data) struct for 3D coordinates
    struct WaveVector3 {
        float x, y, z;
    };

    class Simulator {
    public:
        // Updates the phase of all waves based on deltaTime
        static void UpdateWaves(std::vector<WaveParameters>& waves, float deltaTime);

        // The "Single Point Query" for C++ (Buoyancy/Physics)
        static float GetHeightAt(float x, float z, const std::vector<WaveParameters>& waves);
        
        // Returns the offset vector (for Gerstner horizontal displacement)
        static void GetDisplacement(float x, float z, const std::vector<WaveParameters>& waves, float& outX, float& outY, float& outZ);

        static WaveVector3 GetNormalAt(float x, float z, const std::vector<WaveParameters>& waves);

        static std::vector<WaveParameters> GenerateSeaState(float windSpeed, float mainDirDegrees, float scale, int count, int seed);
    };

};
#endif // WAVELOGIC_H