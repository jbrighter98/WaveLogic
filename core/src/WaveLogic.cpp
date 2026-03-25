#include <vector>
#include <WaveLogic.h>



float WaveLogic::GetHeightAt(float x, float z, const std::vector<WaveParameters>& waves) {
    float height = 0.0f;
    for (const auto& w : waves) {
        float dotDir = (w.directionX * x + w.directionZ * z);
        float angle = w.frequency * dotDir + w.phase;
        height += w.amplitude * sin(angle);
    }
    return height;
}