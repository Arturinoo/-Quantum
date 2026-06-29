#include "classical_engine.h"
#include <iostream>

namespace QuantumEngine {

ClassicalEngine::ClassicalEngine(int count, float G, float softening) {}
ClassicalEngine::~ClassicalEngine() {}
bool ClassicalEngine::initialize() { return true; }
void ClassicalEngine::step(float dt) { /* N-body simulation */ }

} // namespace QuantumEngine
