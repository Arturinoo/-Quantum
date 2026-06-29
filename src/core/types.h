  #pragma once
#include <cmath>

namespace QuantumEngine {

// 3D vektor pre klasickú fyziku
struct float3 {
    float x, y, z;
    
    float3() : x(0), y(0), z(0) {}
    float3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
    
    float3 operator+(const float3& other) const {
        return float3(x + other.x, y + other.y, z + other.z);
    }
    
    float3 operator-(const float3& other) const {
        return float3(x - other.x, y - other.y, z - other.z);
    }
    
    float3 operator*(float scalar) const {
        return float3(x * scalar, y * scalar, z * scalar);
    }
    
    float3& operator+=(const float3& other) {
        x += other.x; y += other.y; z += other.z;
        return *this;
    }
    
    float length() const {
        return sqrt(x*x + y*y + z*z);
    }
    
    float length_squared() const {
        return x*x + y*y + z*z;
    }
};

// Komplexné číslo pre kvantovú mechaniku
struct complex {
    float real, imag;
    
    complex() : real(0), imag(0) {}
    complex(float r, float i) : real(r), imag(i) {}
    
    complex operator+(const complex& other) const {
        return complex(real + other.real, imag + other.imag);
    }
    
    complex operator-(const complex& other) const {
        return complex(real - other.real, imag - other.imag);
    }
    
    complex operator*(float scalar) const {
        return complex(real * scalar, imag * scalar);
    }
    
    complex operator*(const complex& other) const {
        return complex(
            real * other.real - imag * other.imag,
            real * other.imag + imag * other.real
        );
    }
    
    float magnitude_squared() const {
        return real*real + imag*imag;
    }
    
    float magnitude() const {
        return sqrt(magnitude_squared());
    }
};

// Konfigurácia simulácie
struct SimulationConfig {
    // Čas
    float dt = 0.001f;
    float total_time = 10.0f;
    int steps_per_frame = 10;
    
    // Klasické
    int particle_count = 10000;
    float gravitational_constant = 1.0f;  // Škálované pre simuláciu
    float softening_factor = 0.01f;
    
    // Kvantové
    int grid_points = 1024;
    float grid_size = 10.0f;
    float mass = 1.0f;
    float hbar = 1.0f;
    float potential_barrier_height = 5.0f;
    float potential_barrier_width = 1.0f;
    float potential_barrier_position = 0.0f;
    
    // Hybridné
    float coupling_strength = 0.01f;
    bool enable_quantum_noise = true;
    
    // Vizualizácia
    bool enable_visualization = true;
    int window_width = 1280;
    int window_height = 720;
};

} // namespace QuantumEngine
