#pragma once

namespace QuantumEngine {
class ClassicalEngine {
public:
    ClassicalEngine(int count, float G, float softening);
    ~ClassicalEngine();
    bool initialize();
    void step(float dt);
};
} // namespace QuantumEngine
