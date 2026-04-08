
#include "WaveLogic.h"
#include <vector>
#include <cmath>
#include <random>

#include <iostream>

#define _USE_MATH_DEFINES
#include <math.h>

namespace WaveLogic {


    inline float dot2D(float ax, float ay, float bx, float by) {
        return ax * bx + ay * by;
    }

    inline float invSqrt(float x) {
        return 1.0f / std::sqrt(x);
    }


    void Simulator::UpdateWaves(std::vector<WaveParameters>& waves, float deltaTime) {
        
        for (int i = 0; i < waves.size(); ++i) {

            waves[i].phase += waves[i].speed * deltaTime;

        }
    }

    GerstnerResult Simulator::EvaluateWaveAt(float x, float z, const std::vector<WaveParameters>& waves) {
        float px = x;
        float py = 0.0f;
        float pz = z;

        float tx  = 0.0f;
        float tz  = 0.0f;
        float txy = 0.0f;
        float ty  = 1.0f;

        for (const auto& wave : waves)
        {
            float dotDir = dot2D(wave.directionX, wave.directionZ, x, z);
            float angle  = wave.frequency * dotDir + wave.phase;

            float cosA = std::cos(angle);
            float sinA = std::sin(angle);
            float wa   = wave.frequency * wave.amplitude;
            float Qwa  = wave.steepness * wa;

            // Displacement
            px += wave.steepness * wave.amplitude * wave.directionX * cosA;
            pz += wave.steepness * wave.amplitude * wave.directionZ * cosA;
            py += wave.amplitude * sinA;

            // Normal accumulation
            tx  += wave.directionX * wave.directionX * Qwa * sinA;
            tz  += wave.directionZ * wave.directionZ * Qwa * sinA;
            txy += wave.directionX * wave.directionZ * Qwa * sinA;
            ty  -= Qwa * sinA;
        }

        // Normalize
        float nx = -(tx + txy);
        float ny = ty;
        float nz = -(tz + txy);
        float len = std::sqrt(nx * nx + ny * ny + nz * nz);
        float invLen = (len > 0.0f) ? 1.0f / len : 1.0f;

        GerstnerResult result;
        result.position = { px, py, pz };
        result.normal   = { nx * invLen, ny * invLen, nz * invLen };
        return result;
    }


    std::vector<WaveParameters> Simulator::GenerateSea(const SeaParameters& seaParams) {
        std::mt19937 rng(seaParams.seed);
        auto randRange = [&](float lo, float hi) {
            return std::uniform_real_distribution<float>(lo, hi)(rng);
        };
        auto randNormal = [&](float mean, float stddev) {
            return std::normal_distribution<float>(mean, stddev)(rng);
        };

        constexpr float g = 9.81f;

        // Convert fractions to actual plane-space wavelengths
        float lambdaMin = seaParams.shortestWaveFrac * seaParams.planeSize;  // e.g. 1.0
        float lambdaMax = seaParams.longestWaveFrac  * seaParams.planeSize;  // e.g. 12.0

        // Band structure — same as before, but now expressed as
        // fractions of the [lambdaMin, lambdaMax] range
        struct Band {
            float countFraction;
            float lambdaFracLo;   // 0.0 = lambdaMin, 1.0 = lambdaMax
            float lambdaFracHi;
            float amplitudeScale; // relative to sea.amplitudeScale
            float spreadMult;
        };

        const Band bands[] = {
            { 0.15f, 0.7f, 1.0f, 1.00f, 0.3f },  // primary swell:   long, straight
            { 0.35f, 0.3f, 0.7f, 0.45f, 0.7f },  // secondary swell: mid, some spread
            { 0.50f, 0.0f, 0.3f, 0.15f, 1.4f },  // wind chop:       short, chaotic
        };

        float budgetUsed = 0.0f;
        const float budgetMax = 0.85f;

        std::vector<WaveParameters> waves;
        waves.reserve(seaParams.waveCount);

        for (const Band& band : bands)
        {
            int bandCount = std::max(1, static_cast<int>(seaParams.waveCount * band.countFraction));

            for (int i = 0; i < bandCount; ++i)
            {
                WaveParameters wp;

                // 1. Sample wavelength directly in plane-space
                float t = randRange(band.lambdaFracLo, band.lambdaFracHi);
                float lambda = lambdaMin + t * (lambdaMax - lambdaMin);

                // 2. Wavenumber from wavelength (plane-space, no dispersion scaling)
                float k = (2.0f * static_cast<float>(M_PI)) / lambda;
                wp.frequency = k;

                // 3. Phase speed — use dispersion but relative to plane-space k.
                //    sqrt(g/k) gives a plausible speed without needing real-world units.
                //    Scale it down so waves don't race across a 20-unit plane instantly.
                float speed = std::sqrt(g / k);
                wp.speed = speed * 0.3f;  // tweak this one constant to control animation pace

                // 4. Amplitude — proportional to wavelength (longer waves are taller),
                //    scaled by the band's relative amplitude and the master amplitudeScale.
                //    The (lambda / lambdaMax) term gives the right energy falloff shape.
                float ampBase = seaParams.amplitudeScale * band.amplitudeScale;
                wp.amplitude = ampBase * (lambda / lambdaMax) * randRange(0.75f, 1.25f);

                // 5. Steepness — budget-aware, same as before
                float steepnessDesired = seaParams.steepness * randRange(0.5f, 1.0f);
                float cost = steepnessDesired * wp.amplitude * k;
                float remaining = budgetMax - budgetUsed;
                if (cost > remaining)
                    steepnessDesired = (remaining > 0.0f) ? remaining / (wp.amplitude * k) : 0.0f;
                wp.steepness = std::max(0.0f, steepnessDesired);
                budgetUsed += wp.steepness * wp.amplitude * k;

                // 6. Direction
                float spread = seaParams.directionalSpread * band.spreadMult;
                float angle  = randNormal(seaParams.dominantDirection, spread);
                wp.directionX = std::sin(angle);
                wp.directionZ = std::cos(angle);

                // 7. Random phase
                wp.phase = randRange(0.0f, 2.0f * static_cast<float>(M_PI));

                waves.push_back(wp);
            }
        }

        return waves;

    }

};