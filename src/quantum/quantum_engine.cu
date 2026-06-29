#include "quantum_engine.h"
#include <iostream>

namespace QuantumEngine {

QuantumEngine::QuantumEngine(int points, float size, float mass, float hbar) {}
QuantumEngine::~QuantumEngine() {}
bool QuantumEngine::initialize() { return true; }
void QuantumEngine::step(float dt) { /* CUDA kernel */ }

} // namespace QuantumEngine
