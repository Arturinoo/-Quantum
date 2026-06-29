#pragma once

namespace QuantumEngine {

class Renderer {
public:
    Renderer();
    ~Renderer();
    
    bool initialize();
    void shutdown();
    void render(const float* positions, int count, const void* wavefunction, int grid_points);
};

} // namespace QuantumEngine
