#include "engine.h"
#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <complex>
#include <iomanip>
#include <chrono>
#include <deque>
#include <cstring>
#include <omp.h>

extern "C" void run_cuda_nbody(void* particles, int N, float dt, float G, float softening);

namespace QuantumEngine {

const char* typeNames[] = {
    "Electron", "Proton", "Neutron", "Photon", "Muon",
    "Pion", "Kaon", "Up Quark", "Down Quark", "Gluon",
    "Neutrino", "Custom"
};

const float typeMasses[] = {
    9.11e-31f, 1.67e-27f, 1.67e-27f, 0.0f, 1.88e-28f,
    2.49e-28f, 4.98e-28f, 2.2e-30f, 4.7e-30f, 0.0f,
    0.0f, 1.0f
};

const float typeCharges[] = {
    -1.6e-19f, 1.6e-19f, 0.0f, 0.0f, -1.6e-19f,
    1.6e-19f, 1.6e-19f, 2.0f/3.0f * 1.6e-19f, -1.0f/3.0f * 1.6e-19f, 0.0f,
    0.0f, 0.0f
};

const float typeSpins[] = {
    0.5f, 0.5f, 0.5f, 1.0f, 0.5f,
    0.0f, 0.0f, 0.5f, 0.5f, 1.0f,
    0.5f, 0.5f
};

const float typeColors[][3] = {
    {0.2f, 0.8f, 1.0f},
    {1.0f, 0.2f, 0.2f},
    {0.8f, 0.8f, 0.8f},
    {1.0f, 1.0f, 0.0f},
    {0.5f, 0.5f, 1.0f},
    {1.0f, 0.5f, 0.0f},
    {0.0f, 1.0f, 0.5f},
    {1.0f, 0.0f, 1.0f},
    {0.0f, 1.0f, 1.0f},
    {0.5f, 1.0f, 0.5f},
    {0.5f, 0.5f, 0.5f},
    {0.5f, 0.5f, 0.5f}
};

struct QuantumState {
    std::vector<std::complex<float>> psi;
    std::vector<float> V;
    float x_min, x_max;
    int N;
    float dx;
    float time;
    
    QuantumState(int points, float xmin, float xmax) 
        : N(points), x_min(xmin), x_max(xmax), time(0) {
        dx = (x_max - x_min) / N;
        psi.resize(N);
        V.resize(N, 0.0f);
        
        for (int i = 0; i < N; i++) {
            float x = x_min + i * dx;
            float sigma = 0.5f;
            float k0 = 2.0f;
            psi[i] = std::exp(-0.5f * (x * x) / (sigma * sigma)) * 
                     std::exp(std::complex<float>(0, k0 * x));
        }
        normalize();
    }
    
    void normalize() {
        float norm = 0.0f;
        for (int i = 0; i < N; i++) {
            norm += std::norm(psi[i]);
        }
        norm = sqrt(norm * dx);
        if (norm > 0) {
            for (int i = 0; i < N; i++) {
                psi[i] /= norm;
            }
        }
    }
    
    void step(float dt, float hbar = 1.0f, float mass = 1.0f) {
        for (int i = 1; i < N-1; i++) {
            float laplacian_real = (std::real(psi[i+1]) - 2*std::real(psi[i]) + std::real(psi[i-1])) / (dx*dx);
            float laplacian_imag = (std::imag(psi[i+1]) - 2*std::imag(psi[i]) + std::imag(psi[i-1])) / (dx*dx);
            
            float kinetic_real = -hbar*hbar/(2*mass) * laplacian_real;
            float kinetic_imag = -hbar*hbar/(2*mass) * laplacian_imag;
            
            float pot_real = V[i] * std::real(psi[i]);
            float pot_imag = V[i] * std::imag(psi[i]);
            
            psi[i] += std::complex<float>(
                (kinetic_imag - pot_imag) / hbar * dt,
                -(kinetic_real - pot_real) / hbar * dt
            );
        }
        
        for (int i = 0; i < 10; i++) {
            psi[i] *= 0.99f;
            psi[N-1-i] *= 0.99f;
        }
        
        normalize();
        time += dt;
    }
    
    float get_probability(int i) const {
        return std::norm(psi[i]);
    }
};

struct FullState {
    float time;
    std::vector<Particle> particles;
};

class Engine::Impl {
public:
    std::vector<Particle> particles;
    QuantumState quantum;
    std::mt19937 rng;
    std::uniform_real_distribution<float> dist;
    
    float currentTime = 0.0f;
    float speed = 1.0f;
    bool paused = false;
    int step_count = 0;
    bool use_cuda = true;
    int godParticleIndex = -1;
    
    std::deque<FullState> history;
    int maxHistorySize = 10000;
    int historyIndex = -1;
    
    std::vector<float> positionBuffer;
    std::vector<float> wavefunctionBuffer;
    std::vector<float> potentialBuffer;
    
    Impl() : rng(std::random_device{}()), dist(-1.0f, 1.0f), quantum(512, -5.0f, 5.0f) {}
    
    void init_particles(int count) {
        particles.resize(count);
        positionBuffer.resize(count * 3);
        wavefunctionBuffer.resize(512);
        potentialBuffer.resize(512);
        
        std::uniform_real_distribution<float> radius_dist(0.5f, 6.0f);
        std::uniform_real_distribution<float> angle_dist(0, 2 * 3.14159f);
        std::uniform_real_distribution<float> height_dist(-0.8f, 0.8f);
        std::uniform_real_distribution<float> speed_dist(-0.03f, 0.03f);
        
        #pragma omp parallel for
        for (int i = 0; i < count; i++) {
            float r = radius_dist(rng);
            float theta = angle_dist(rng);
            float z = height_dist(rng);
            
            int typeIdx = i % 5;
            
            particles[i].x = r * cos(theta);
            particles[i].y = r * sin(theta);
            particles[i].z = z;
            
            float v = 0.7f / sqrt(r + 0.1f);
            particles[i].vx = -v * sin(theta) + speed_dist(rng) * 0.5f;
            particles[i].vy = v * cos(theta) + speed_dist(rng) * 0.5f;
            particles[i].vz = speed_dist(rng) * 0.5f;
            
            particles[i].type = (ParticleType)typeIdx;
            particles[i].mass = typeMasses[typeIdx] * 1e27f;
            particles[i].charge = typeCharges[typeIdx];
            particles[i].spin = typeSpins[typeIdx];
            particles[i].energy = 0.0f;
            particles[i].temperature = 300.0f + dist(rng) * 200.0f;
            particles[i].lifetime = 1e10f;
            particles[i].decayTime = 0.0f;
            particles[i].active = true;
            particles[i].isAntiparticle = false;
            particles[i].isGodParticle = false;
            particles[i].r = typeColors[typeIdx][0];
            particles[i].g = typeColors[typeIdx][1];
            particles[i].b = typeColors[typeIdx][2];
            particles[i].size = 2.0f + (particles[i].mass / 1e-27f) * 0.5f;
        }
        
        std::cout << "   ✅ " << count << " particles created" << std::endl;
        std::cout << "   🚀 CUDA: " << (use_cuda ? "ENABLED" : "DISABLED") << std::endl;
        std::cout << "   💻 CPU cores: " << omp_get_max_threads() << std::endl;
    }
    
    void saveState() {
        FullState state;
        state.time = currentTime;
        state.particles = particles;
        
        if (historyIndex >= 0 && historyIndex < (int)history.size() - 1) {
            history.resize(historyIndex + 1);
        }
        
        history.push_back(state);
        if ((int)history.size() > maxHistorySize) {
            history.pop_front();
        }
        historyIndex = (int)history.size() - 1;
    }
    
    bool restoreState(float time) {
        if (history.empty()) return false;
        
        int bestIdx = 0;
        float bestDiff = std::abs(history[0].time - time);
        for (int i = 1; i < (int)history.size(); i++) {
            float diff = std::abs(history[i].time - time);
            if (diff < bestDiff) {
                bestDiff = diff;
                bestIdx = i;
            }
        }
        
        const FullState& state = history[bestIdx];
        historyIndex = bestIdx;
        currentTime = state.time;
        particles = state.particles;
        positionBuffer.resize(particles.size() * 3);
        
        return true;
    }
    
    void step(float dt) {
        if (paused) return;
        
        float effectiveDt = dt * speed;
        step_count++;
        const float G = 0.05f;
        const float softening = 0.02f;
        const float coupling = 0.3f;
        int N = particles.size();
        
        #pragma omp parallel for
        for (int i = 0; i < quantum.N; i++) {
            float x = quantum.x_min + i * quantum.dx;
            float V_sum = 0.0f;
            for (const auto& p : particles) {
                float dx = x - p.x;
                float dy = -p.y;
                float dz = -p.z;
                float dist = sqrt(dx*dx + dy*dy + dz*dz + 0.01f);
                V_sum += -coupling / dist;
            }
            quantum.V[i] = V_sum * 0.01f;
        }
        
        #pragma omp parallel for
        for (int i = 0; i < quantum.N; i++) {
            float x = quantum.x_min + i * quantum.dx;
            if (std::abs(x) < 0.5f && std::abs(x) > 0.2f) {
                quantum.V[i] += 3.0f;
            }
        }
        
        quantum.step(effectiveDt * 0.3f);
        
        std::vector<float> ax(N), ay(N), az(N);
        
        #pragma omp parallel for
        for (int i = 0; i < N; i++) {
            if (particles[i].isGodParticle) {
                ax[i] = 0.0f;
                ay[i] = 0.0f;
                az[i] = 0.0f;
                continue;
            }
            
            float accelX = 0, accelY = 0, accelZ = 0;
            for (int j = 0; j < N; j++) {
                if (i == j) continue;
                float dx = particles[j].x - particles[i].x;
                float dy = particles[j].y - particles[i].y;
                float dz = particles[j].z - particles[i].z;
                float dist2 = dx*dx + dy*dy + dz*dz + softening*softening;
                float dist = sqrt(dist2);
                float force = G * particles[j].mass / (dist2);
                accelX += force * dx / dist;
                accelY += force * dy / dist;
                accelZ += force * dz / dist;
            }
            ax[i] = accelX;
            ay[i] = accelY;
            az[i] = accelZ;
        }
        
        #pragma omp parallel for
        for (int i = 0; i < N; i++) {
            if (particles[i].isGodParticle) continue;
            
            particles[i].vx += ax[i] * effectiveDt;
            particles[i].vy += ay[i] * effectiveDt;
            particles[i].vz += az[i] * effectiveDt;
            particles[i].x += particles[i].vx * effectiveDt;
            particles[i].y += particles[i].vy * effectiveDt;
            particles[i].z += particles[i].vz * effectiveDt;
            
            particles[i].energy = 0.5f * particles[i].mass * 
                (particles[i].vx*particles[i].vx + 
                 particles[i].vy*particles[i].vy + 
                 particles[i].vz*particles[i].vz);
        }
        
        #pragma omp parallel for
        for (int i = 0; i < N; i++) {
            if (particles[i].isGodParticle) continue;
            
            float dist = sqrt(particles[i].x*particles[i].x + 
                            particles[i].y*particles[i].y + 
                            particles[i].z*particles[i].z);
            if (dist > 8.0f) {
                particles[i].x *= 0.995f;
                particles[i].y *= 0.995f;
                particles[i].z *= 0.995f;
            }
        }
        
        currentTime += effectiveDt;
        
        if (step_count % 50 == 0) {
            std::cout << "🔹 STEP " << step_count << " | Time: " << currentTime << "s" << std::endl;
        }
    }
    
    float* get_positions() {
        for (int i = 0; i < particles.size(); i++) {
            positionBuffer[i*3] = particles[i].x;
            positionBuffer[i*3+1] = particles[i].y;
            positionBuffer[i*3+2] = particles[i].z;
        }
        return positionBuffer.data();
    }
    
    int get_count() const { return particles.size(); }
    float get_time() const { return currentTime; }
    
    void add_particle(float x, float y, float z, ParticleType type) {
        Particle p;
        p.x = x;
        p.y = y;
        p.z = z;
        p.vx = (dist(rng) - 0.5f) * 0.1f;
        p.vy = (dist(rng) - 0.5f) * 0.1f;
        p.vz = (dist(rng) - 0.5f) * 0.1f;
        
        int idx = (int)type;
        p.type = type;
        p.mass = typeMasses[idx] * 1e27f;
        p.charge = typeCharges[idx];
        p.spin = typeSpins[idx];
        p.energy = 0.0f;
        p.temperature = 300.0f;
        p.lifetime = 1e10f;
        p.decayTime = 0.0f;
        p.active = true;
        p.isAntiparticle = false;
        p.isGodParticle = false;
        p.r = typeColors[idx][0];
        p.g = typeColors[idx][1];
        p.b = typeColors[idx][2];
        p.size = 2.0f + (p.mass / 1e-27f) * 0.5f;
        
        particles.push_back(p);
        positionBuffer.resize(particles.size() * 3);
        std::cout << "   ✅ Added " << typeNames[idx] << " at (" << x << ", " << y << ", " << z << ")" << std::endl;
    }
    
    void applyForce(float x, float y, float z, float strength, bool attract) {
        float sign = attract ? 1.0f : -1.0f;
        float radius = 5.0f;
        
        #pragma omp parallel for
        for (int i = 0; i < particles.size(); i++) {
            auto& p = particles[i];
            if (p.isGodParticle) continue;
            float dx = p.x - x;
            float dy = p.y - y;
            float dz = p.z - z;
            float dist = sqrt(dx*dx + dy*dy + dz*dz);
            if (dist < 0.01f) continue;
            if (dist > radius) continue;
            
            float force = strength * sign / (dist + 0.1f);
            p.vx += force * dx / dist * 0.01f;
            p.vy += force * dy / dist * 0.01f;
            p.vz += force * dz / dist * 0.01f;
        }
    }
    
    float getTotalEnergy() const {
        float total = 0.0f;
        #pragma omp parallel for reduction(+:total)
        for (int i = 0; i < particles.size(); i++) {
            if (particles[i].isGodParticle) continue;
            total += particles[i].energy;
        }
        return total;
    }
    
    float getTotalCharge() const {
        float total = 0.0f;
        #pragma omp parallel for reduction(+:total)
        for (int i = 0; i < particles.size(); i++) {
            total += particles[i].charge;
        }
        return total;
    }
    
    void setGodParticle(int index) {
        if (index < 0 || index >= (int)particles.size()) return;
        
        if (godParticleIndex >= 0 && godParticleIndex < (int)particles.size()) {
            particles[godParticleIndex].isGodParticle = false;
        }
        
        godParticleIndex = index;
        particles[index].isGodParticle = true;
        particles[index].vx = 0.0f;
        particles[index].vy = 0.0f;
        particles[index].vz = 0.0f;
        particles[index].r = 1.0f;
        particles[index].g = 1.0f;
        particles[index].b = 0.0f;
        particles[index].size = 5.0f;
        
        std::cout << "🌟 GOD PARTICLE: " << index << std::endl;
    }
    
    int getGodParticleIndex() const {
        return godParticleIndex;
    }
    
    void releaseGodParticle() {
        if (godParticleIndex >= 0 && godParticleIndex < (int)particles.size()) {
            particles[godParticleIndex].isGodParticle = false;
            particles[godParticleIndex].r = typeColors[(int)particles[godParticleIndex].type][0];
            particles[godParticleIndex].g = typeColors[(int)particles[godParticleIndex].type][1];
            particles[godParticleIndex].b = typeColors[(int)particles[godParticleIndex].type][2];
            particles[godParticleIndex].size = 2.0f + (particles[godParticleIndex].mass / 1e-27f) * 0.5f;
        }
        godParticleIndex = -1;
        std::cout << "🌟 GOD PARTICLE RELEASED" << std::endl;
    }
    
    void moveGodParticle(float dx, float dy, float dz) {
        if (godParticleIndex < 0 || godParticleIndex >= (int)particles.size()) return;
        
        particles[godParticleIndex].x += dx;
        particles[godParticleIndex].y += dy;
        particles[godParticleIndex].z += dz;
        particles[godParticleIndex].vx = dx / 0.01f;
        particles[godParticleIndex].vy = dy / 0.01f;
        particles[godParticleIndex].vz = dz / 0.01f;
    }
};

// ============================================================
// ENGINE WRAPPER
// ============================================================
Engine::Engine() : impl(new Impl()) {}
Engine::~Engine() { delete impl; }

bool Engine::initialize() {
    std::cout << "\n🔧 Initializing hybrid engine (CUDA)..." << std::endl;
    impl->init_particles(5000);
    std::cout << "✅ " << impl->get_count() << " particles created" << std::endl;
    std::cout << "✅ Quantum grid: " << 512 << " points" << std::endl;
    return true;
}

void Engine::shutdown() {
    std::cout << "🛑 Engine shutdown" << std::endl;
}

void Engine::setTime(float time) {
    impl->restoreState(time);
}

float Engine::getTime() const {
    return impl->get_time();
}

void Engine::step(float dt) {
    impl->step(dt);
}

void Engine::setSpeed(float speed) {
    impl->speed = speed;
}

float Engine::getSpeed() const {
    return impl->speed;
}

void Engine::togglePause() {
    impl->paused = !impl->paused;
}

bool Engine::isPaused() const {
    return impl->paused;
}

void Engine::saveState() {
    impl->saveState();
}

void Engine::restoreState(float time) {
    impl->restoreState(time);
}

int Engine::getStateCount() const {
    return impl->history.size();
}

float Engine::getStateTime(int index) const {
    if (index < 0 || index >= (int)impl->history.size()) return -1.0f;
    return impl->history[index].time;
}

float* Engine::get_positions() {
    return impl->get_positions();
}

int Engine::get_particle_count() const {
    return impl->get_count();
}

void Engine::add_particle(float x, float y, float z, ParticleType type) {
    impl->add_particle(x, y, z, type);
}

void Engine::applyForce(float x, float y, float z, float strength, bool attract) {
    impl->applyForce(x, y, z, strength, attract);
}

float Engine::getTotalEnergy() const {
    return impl->getTotalEnergy();
}

float Engine::getTotalCharge() const {
    return impl->getTotalCharge();
}

float Engine::getParticleX(int index) const {
    if (index < 0 || index >= (int)impl->particles.size()) return 0.0f;
    return impl->particles[index].x;
}

float Engine::getParticleY(int index) const {
    if (index < 0 || index >= (int)impl->particles.size()) return 0.0f;
    return impl->particles[index].y;
}

float Engine::getParticleZ(int index) const {
    if (index < 0 || index >= (int)impl->particles.size()) return 0.0f;
    return impl->particles[index].z;
}

float Engine::getParticleVx(int index) const {
    if (index < 0 || index >= (int)impl->particles.size()) return 0.0f;
    return impl->particles[index].vx;
}

float Engine::getParticleVy(int index) const {
    if (index < 0 || index >= (int)impl->particles.size()) return 0.0f;
    return impl->particles[index].vy;
}

float Engine::getParticleVz(int index) const {
    if (index < 0 || index >= (int)impl->particles.size()) return 0.0f;
    return impl->particles[index].vz;
}

float Engine::getParticleMass(int index) const {
    if (index < 0 || index >= (int)impl->particles.size()) return 0.0f;
    return impl->particles[index].mass;
}

float Engine::getParticleCharge(int index) const {
    if (index < 0 || index >= (int)impl->particles.size()) return 0.0f;
    return impl->particles[index].charge;
}

float Engine::getParticleSpin(int index) const {
    if (index < 0 || index >= (int)impl->particles.size()) return 0.0f;
    return impl->particles[index].spin;
}

float Engine::getParticleEnergy(int index) const {
    if (index < 0 || index >= (int)impl->particles.size()) return 0.0f;
    return impl->particles[index].energy;
}

float Engine::getParticleTemperature(int index) const {
    if (index < 0 || index >= (int)impl->particles.size()) return 0.0f;
    return impl->particles[index].temperature;
}

bool Engine::getParticleGod(int index) const {
    if (index < 0 || index >= (int)impl->particles.size()) return false;
    return impl->particles[index].isGodParticle;
}

// ============================================================
// SETTERY PRE MANUAL OVERRIDE (SPRÁVNE V NAMESPACE)
// ============================================================
void Engine::setParticleMass(int index, float mass) {
    if (index < 0 || index >= (int)impl->particles.size()) return;
    impl->particles[index].mass = mass;
}

void Engine::setParticleCharge(int index, float charge) {
    if (index < 0 || index >= (int)impl->particles.size()) return;
    impl->particles[index].charge = charge;
}

void Engine::setParticleSpin(int index, float spin) {
    if (index < 0 || index >= (int)impl->particles.size()) return;
    impl->particles[index].spin = spin;
}

void Engine::setParticleVelocity(int index, float vx, float vy, float vz) {
    if (index < 0 || index >= (int)impl->particles.size()) return;
    impl->particles[index].vx = vx;
    impl->particles[index].vy = vy;
    impl->particles[index].vz = vz;
}

void Engine::setParticleGod(int index, bool god) {
    if (index < 0 || index >= (int)impl->particles.size()) return;
    impl->particles[index].isGodParticle = god;
    if (god) {
        impl->godParticleIndex = index;
        impl->particles[index].r = 1.0f;
        impl->particles[index].g = 1.0f;
        impl->particles[index].b = 0.0f;
        impl->particles[index].size = 5.0f;
    } else {
        impl->particles[index].r = typeColors[(int)impl->particles[index].type][0];
        impl->particles[index].g = typeColors[(int)impl->particles[index].type][1];
        impl->particles[index].b = typeColors[(int)impl->particles[index].type][2];
        impl->particles[index].size = 2.0f + (impl->particles[index].mass / 1e-27f) * 0.5f;
    }
}

void Engine::moveGodParticle(float dx, float dy, float dz) {
    impl->moveGodParticle(dx, dy, dz);
}

void Engine::setGodParticle(int index) {
    impl->setGodParticle(index);
}

int Engine::getGodParticleIndex() const {
    return impl->getGodParticleIndex();
}

void Engine::releaseGodParticle() {
    impl->releaseGodParticle();
}

} // namespace QuantumEngine
