  #pragma once
#include "../core/types.h"

namespace QuantumEngine {

struct Particle {
    float3 position;
    float3 velocity;
    float3 acceleration;
    float mass;
    float radius;
    bool active;
    
    Particle() : position(0,0,0), velocity(0,0,0), acceleration(0,0,0), 
                 mass(1.0f), radius(0.1f), active(true) {}
    
    Particle(float x, float y, float z, float mass_) 
        : position(x, y, z), velocity(0,0,0), acceleration(0,0,0),
          mass(mass_), radius(0.1f), active(true) {}
    
    float kinetic_energy() const {
        return 0.5f * mass * velocity.length_squared();
    }
};

// Barnes-Hut strom pre optimalizáciu N-body
struct OctreeNode {
    float3 center;
    float size;
    float total_mass;
    float3 center_of_mass;
    OctreeNode* children[8];
    Particle* particle;
    bool is_leaf;
    int particle_count;
    
    OctreeNode(const float3& center_, float size_) 
        : center(center_), size(size_), total_mass(0), center_of_mass(0,0,0),
          particle(nullptr), is_leaf(true), particle_count(0) {
        for (int i = 0; i < 8; i++) children[i] = nullptr;
    }
    
    ~OctreeNode() {
        for (int i = 0; i < 8; i++) {
            delete children[i];
        }
    }
};

} // namespace QuantumEngine
