#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include <math.h>
#include <iostream>

// Štruktúra častice pre GPU (musí byť identická s CPU verziou!)
struct GPUParticle {
    float x, y, z;
    float vx, vy, vz;
    float mass;
};

// CUDA kernel pre výpočet zrýchlení
__global__ void compute_acceleration_kernel(
    const GPUParticle* particles,
    float* ax, float* ay, float* az,
    int N,
    float G,
    float softening
) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= N) return;
    
    float accelX = 0.0f;
    float accelY = 0.0f;
    float accelZ = 0.0f;
    
    float px = particles[i].x;
    float py = particles[i].y;
    float pz = particles[i].z;
    
    for (int j = 0; j < N; j++) {
        if (i == j) continue;
        
        float dx = particles[j].x - px;
        float dy = particles[j].y - py;
        float dz = particles[j].z - pz;
        
        float dist2 = dx*dx + dy*dy + dz*dz + softening*softening;
        float dist = sqrtf(dist2);
        float force = G * particles[j].mass / dist2;
        
        accelX += force * dx / dist;
        accelY += force * dy / dist;
        accelZ += force * dz / dist;
    }
    
    ax[i] = accelX;
    ay[i] = accelY;
    az[i] = accelZ;
}

// CUDA kernel pre update častíc
__global__ void update_particles_kernel(
    GPUParticle* particles,
    const float* ax,
    const float* ay,
    const float* az,
    int N,
    float dt
) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= N) return;
    
    particles[i].vx += ax[i] * dt;
    particles[i].vy += ay[i] * dt;
    particles[i].vz += az[i] * dt;
    particles[i].x += particles[i].vx * dt;
    particles[i].y += particles[i].vy * dt;
    particles[i].z += particles[i].vz * dt;
}

// Wrapper funkcia pre volanie z C++
extern "C" void run_cuda_nbody(
    void* particles_ptr,
    int N,
    float dt,
    float G,
    float softening
) {
    // Skontrolujeme, či máme častice
    if (N == 0 || particles_ptr == nullptr) {
        std::cerr << "❌ CUDA: Chyba - žiadne častice!" << std::endl;
        return;
    }
    
    GPUParticle* particles = static_cast<GPUParticle*>(particles_ptr);
    
    // Alokácia pamäte na GPU
    GPUParticle* d_particles;
    float* d_ax;
    float* d_ay;
    float* d_az;
    
    size_t particle_size = N * sizeof(GPUParticle);
    size_t accel_size = N * sizeof(float);
    
    cudaError_t err;
    
    err = cudaMalloc(&d_particles, particle_size);
    if (err != cudaSuccess) {
        std::cerr << "❌ CUDA malloc particles: " << cudaGetErrorString(err) << std::endl;
        return;
    }
    
    err = cudaMalloc(&d_ax, accel_size);
    if (err != cudaSuccess) {
        std::cerr << "❌ CUDA malloc ax: " << cudaGetErrorString(err) << std::endl;
        cudaFree(d_particles);
        return;
    }
    
    err = cudaMalloc(&d_ay, accel_size);
    if (err != cudaSuccess) {
        std::cerr << "❌ CUDA malloc ay: " << cudaGetErrorString(err) << std::endl;
        cudaFree(d_particles);
        cudaFree(d_ax);
        return;
    }
    
    err = cudaMalloc(&d_az, accel_size);
    if (err != cudaSuccess) {
        std::cerr << "❌ CUDA malloc az: " << cudaGetErrorString(err) << std::endl;
        cudaFree(d_particles);
        cudaFree(d_ax);
        cudaFree(d_ay);
        return;
    }
    
    // Kopírovanie dát na GPU
    err = cudaMemcpy(d_particles, particles, particle_size, cudaMemcpyHostToDevice);
    if (err != cudaSuccess) {
        std::cerr << "❌ CUDA memcpy to device: " << cudaGetErrorString(err) << std::endl;
        cudaFree(d_particles);
        cudaFree(d_ax);
        cudaFree(d_ay);
        cudaFree(d_az);
        return;
    }
    
    // Konfigurácia kernelu
    int threads_per_block = 256;
    int blocks = (N + threads_per_block - 1) / threads_per_block;
    
    // Výpočet zrýchlení
    compute_acceleration_kernel<<<blocks, threads_per_block>>>(
        d_particles, d_ax, d_ay, d_az, N, G, softening
    );
    
    // Čakanie na dokončenie
    cudaDeviceSynchronize();
    
    // Update častíc
    update_particles_kernel<<<blocks, threads_per_block>>>(
        d_particles, d_ax, d_ay, d_az, N, dt
    );
    
    cudaDeviceSynchronize();
    
    // Kopírovanie výsledkov späť
    err = cudaMemcpy(particles, d_particles, particle_size, cudaMemcpyDeviceToHost);
    if (err != cudaSuccess) {
        std::cerr << "❌ CUDA memcpy from device: " << cudaGetErrorString(err) << std::endl;
    }
    
    // Uvoľnenie pamäte
    cudaFree(d_particles);
    cudaFree(d_ax);
    cudaFree(d_ay);
    cudaFree(d_az);
}
