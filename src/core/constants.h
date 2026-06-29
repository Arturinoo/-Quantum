#pragma once

namespace QuantumEngine {
namespace Constants {

// Fyzikálne konštanty (v simulovaných jednotkách)
constexpr float PI = 3.14159265359f;
constexpr float TWO_PI = 2.0f * PI;
constexpr float SPEED_OF_LIGHT = 299792458.0f;  // m/s (pre referenciu)
constexpr float GRAVITATIONAL_CONSTANT = 6.67430e-11f;

// Simulované konštanty
constexpr float DEFAULT_DT = 0.001f;
constexpr float DEFAULT_GRID_SIZE = 10.0f;
constexpr int DEFAULT_GRID_POINTS = 1024;

// GPU
constexpr int CUDA_THREADS_PER_BLOCK = 256;

} // namespace Constants
} // namespace QuantumEngine