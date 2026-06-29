#pragma once

namespace QuantumEngine {
class QuantumEngine {
public:
    QuantumEngine(int points, float size, float mass, float hbar);
    ~QuantumEngine();
    bool initialize();
    void step(float dt);
};
} // namespace QuantumEngine
