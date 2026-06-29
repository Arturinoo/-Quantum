#pragma once
#include <vector>

namespace QuantumEngine {

enum class ParticleType {
    ELECTRON = 0,
    PROTON = 1,
    NEUTRON = 2,
    PHOTON = 3,
    MUON = 4,
    PION = 5,
    KAON = 6,
    QUARK_UP = 7,
    QUARK_DOWN = 8,
    GLUON = 9,
    NEUTRINO = 10,
    CUSTOM = 11
};

struct Particle {
    float x, y, z;
    float vx, vy, vz;
    float mass;
    float charge;
    float spin;
    float energy;
    float temperature;
    float lifetime;
    float decayTime;
    ParticleType type;
    float r, g, b;
    float size;
    bool active;
    bool isAntiparticle;
    bool isGodParticle;  // <-- NOVÉ: God Particle flag
};

class Engine {
private:
    class Impl;
    Impl* impl;
    
public:
    Engine();
    ~Engine();
    
    bool initialize();
    void shutdown();
    
    void setTime(float time);
    float getTime() const;
    void step(float dt);
    void setSpeed(float speed);
    float getSpeed() const;
    void togglePause();
    bool isPaused() const;
    
    float* get_positions();
    int get_particle_count() const;
    void add_particle(float x, float y, float z, ParticleType type = ParticleType::ELECTRON);
    void applyForce(float x, float y, float z, float strength, bool attract);
    
    void saveState();
    void restoreState(float time);
    int getStateCount() const;
    float getStateTime(int index) const;
    
    float getTotalEnergy() const;
    float getTotalCharge() const;
    
    // Gettery pre inšpektor
    float getParticleX(int index) const;
    float getParticleY(int index) const;
    float getParticleZ(int index) const;
    float getParticleVx(int index) const;
    float getParticleVy(int index) const;
    float getParticleVz(int index) const;
    float getParticleMass(int index) const;
    float getParticleCharge(int index) const;
    float getParticleSpin(int index) const;
    float getParticleEnergy(int index) const;
    float getParticleTemperature(int index) const;
    bool getParticleGod(int index) const;
    
    void setParticleMass(int index, float mass);
    void setParticleCharge(int index, float charge);
    void setParticleSpin(int index, float spin);
    void setParticleVelocity(int index, float vx, float vy, float vz);
    void setParticleGod(int index, bool god);
    
    // God Particle – pohyb
    void moveGodParticle(float dx, float dy, float dz);
    void setGodParticle(int index);
    int getGodParticleIndex() const;
    void releaseGodParticle();
};

} // namespace QuantumEngine
