#ifndef WAVELOGIC_H
#define WAVELOGIC_H

#pragma once
#include <vector>
#include <cmath>

namespace WaveLogic {

    struct WaveParameters {
        float amplitude;
        float frequency;
        float speed;
        float steepness;
        float directionX, directionZ;
        float phase; // Updated over time
    };

    struct SeaParameters {
        float dominantDirection;
        float directionalSpread;
        float planeSize;          
        float amplitudeScale;     // master amplitude, in plane units. Start with 0.05f (= 5% of plane)
        float steepness;          // global steepness target, 0.0–1.0
        float shortestWaveFrac;   // shortest wave as fraction of planeSize. e.g. 0.05 = 1 unit
        float longestWaveFrac;    // longest wave as fraction of planeSize.  e.g. 0.6  = 12 units
        int   waveCount;
        unsigned int seed;           
    };

    // Simple struct for 3D coordinates
    struct WaveVector3 {
        float x, y, z;
    };

    struct GerstnerResult {
        WaveVector3 position;
        WaveVector3 normal;
    };

    class Simulator {
    public:
        // Updates the phase of all waves based on deltaTime
        static void UpdateWaves(std::vector<WaveParameters>& waves, float deltaTime);

        static GerstnerResult EvaluateWaveAt(float x, float z, const std::vector<WaveParameters>& waves);

        static std::vector<WaveParameters> GenerateSea(const SeaParameters& seaParams);
    };

};
#endif // WAVELOGIC_H