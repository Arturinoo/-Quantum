#pragma once

namespace QuantumEngine {
class HybridEngine {
public:
    HybridEngine(void* classical, void* quantum, float coupling);
    ~HybridEngine();
    bool initialize();
    void couple(float dt);
};
} // namespace QuantumEngine
