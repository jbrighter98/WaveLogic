
#include "WaveLogic.h"
#include <vector>
#include <cmath>
#include <random>

#include <iostream>

#define _USE_MATH_DEFINES
#include <math.h>

namespace WaveLogic {

    float Simulator::GetHeightAt(float x, float z, const std::vector<WaveParameters>& waves) {
        float height = 0.0f;
        for (const auto& w : waves) {
            float dotDir = (w.directionX * x + w.directionZ * z);
            float angle = w.frequency * dotDir + w.phase;
            height += w.amplitude * sin(angle);
        }
        return height;
    }


    void Simulator::UpdateWaves(std::vector<WaveParameters>& waves, float deltaTime) {
        
        for (int i = 0; i < waves.size(); ++i) {

            waves[i].phase += waves[i].speed * deltaTime;

        }
    }

    WaveVector3 Simulator::GetNormalAt(float x, float z, const std::vector<WaveParameters>& waves) {
        float nx = 0.0f;
        float nz = 0.0f;
        float ny = 1.0f; // Start with the "Up" component

        for (const auto& w : waves) {
            float dotDir = (w.directionX * x + w.directionZ * z);
            float angle = w.frequency * dotDir + w.phase;

            float wa = w.frequency * w.amplitude;
            float cosA = std::cos(angle);
            float sinA = std::sin(angle);

            nx -= w.directionX * wa * cosA;
            nz -= w.directionZ * wa * cosA;
            ny -= w.steepness * wa * sinA;
        }

        // Manual Normalization: length = sqrt(x^2 + y^2 + z^2)
        float length = std::sqrt(nx * nx + ny * ny + nz * nz);
        
        // Prevent division by zero if waves are somehow invalid
        if (length > 0.0001f) {
            return { nx / length, ny / length, nz / length };
        }

        return { 0.0f, 1.0f, 0.0f };
    }


    std::vector<WaveParameters> Simulator::GenerateSeaState(float windSpeed, float mainDirDegrees, float scale,int count, int seed) {
        std::vector<WaveParameters> waves;
        std::mt19937 rng(seed);
        
        // Distributions for variety
        std::uniform_real_distribution<float> dist(0.8f, 1.2f); // Slight randomness factor
        std::uniform_real_distribution<float> dirSpread(-0.5f, 0.5f); // Directional scattering
        
        float mainRad = mainDirDegrees * (M_PI / 180.0f);
        float gravity = 9.81f;

        float totalWa = 0.0f;

        for (int i = 0; i < count; ++i) {
            WaveParameters w;
            
            // Frequency (w): We spread them from large swells to small ripples
            // We use i to ensure a good spread across the spectrum
            float frequencyBase = 0.2f + (i * 0.15f); 
            w.frequency = frequencyBase * dist(rng) * scale; // Scale up for more visible waves

            // Amplitude (A): Inversely proportional to frequency
            // Higher wind speed scales the whole ocean up
            w.amplitude = (windSpeed / (w.frequency * 10.0f)) * dist(rng);

            totalWa += w.amplitude * w.frequency; // For normalization later

            // Speed (v): Driven by the Dispersion Relation
            // v = sqrt(g / k) where k is frequency
            float speedScale = 0.5f;
            w.speed = std::sqrt(gravity / w.frequency) * speedScale;

            // Steepness (Q): Needs to be lower as we add more waves
            w.steepness = 0.5f / count; 

            // Direction: Main wind direction + some "spreading"
            float angle = mainRad + dirSpread(rng);
            w.directionX = std::cos(angle);
            w.directionZ = std::sin(angle);

            w.phase = 0.0f; // Start at zero
            
            waves.push_back(w);
        }

        // Normalize amplitudes to ensure the total height is within a reasonable range
        float maxPower = 0.9f; // The ocean will never be taller than 2 units
        if (totalWa > maxPower) {
            float scale = maxPower / totalWa;
            for (auto& w : waves) {
                w.amplitude *= scale;
            }
        }

        return waves;
    }

};