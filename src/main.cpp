#include <iostream>
#include <chrono>
#include <thread>
#include <iomanip>
#include <vector>
#include <cmath>
#include "core/engine.h"
#include "visualization/opengl_renderer.h"
#include "visualization/imgui_renderer.h"

using namespace QuantumEngine;

int main() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "     QUANTUM ENGINE v1.5.0           " << std::endl;
    std::cout << "        (God Flight - Camera)        " << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    Engine engine;
    if (!engine.initialize()) {
        std::cerr << "ERROR: Init failed!" << std::endl;
        return 1;
    }
    
    OpenGLRenderer renderer;
    if (!renderer.initialize(1280, 720, "Quantum Engine - God Flight")) {
        std::cerr << "ERROR: Renderer init failed!" << std::endl;
        return 1;
    }
    
    ImGuiRenderer gui;
    if (!gui.initialize(renderer.getWindow())) {
        std::cerr << "ERROR: GUI init failed!" << std::endl;
        return 1;
    }
    
    renderer.setCameraDistance(5.0f);
    renderer.resetCamera();
    
    std::cout << "UNIVERSE SIMULATION RUNNING..." << std::endl;
    std::cout << "   Press 'G' to create God Particle" << std::endl;
    std::cout << "   SPACE = fly forward (camera direction)" << std::endl;
    std::cout << "   SHIFT = fly backward" << std::endl;
    std::cout << "   Q / E = up / down" << std::endl;
    std::cout << "   Right mouse = look around\n" << std::endl;
    
    int frame = 0;
    auto start = std::chrono::high_resolution_clock::now();
    int inspectorUpdateCounter = 0;
    float flySpeed = 0.2f;
    
    const int targetFPS = 60;
    const std::chrono::milliseconds frameDuration(1000 / targetFPS);
    
    while (!renderer.shouldClose()) {
        auto frameStart = std::chrono::high_resolution_clock::now();
        renderer.pollEvents();
        
        if (renderer.shouldClose()) break;
        
        GLFWwindow* win = renderer.getWindow();
        int godIdx = engine.getGodParticleIndex();
        
        // GOD PARTICLE – VYTVORENIE
        if (glfwGetKey(win, GLFW_KEY_G) == GLFW_PRESS) {
            if (gui.selectedParticleIndex >= 0 && gui.selectedParticleIndex < engine.get_particle_count()) {
                int idx = gui.selectedParticleIndex;
                if (godIdx >= 0) engine.releaseGodParticle();
                engine.setGodParticle(idx);
                gui.followParticle = true;
                std::cout << "🌟 GOD PARTICLE ACTIVATED on particle " << idx << std::endl;
            }
        }
        
        if (glfwGetKey(win, GLFW_KEY_R) == GLFW_PRESS) {
            if (godIdx >= 0) {
                engine.releaseGodParticle();
                std::cout << "🌟 GOD PARTICLE RELEASED" << std::endl;
            }
        }
        
        // LIETANIE GOD PARTICLE
        if (godIdx >= 0) {
            float speed = flySpeed;
            float dx = 0, dy = 0, dz = 0;
            bool moved = false;
            
            float forwardX = 0, forwardY = 0, forwardZ = -1.0f;
            
            bool spacePressed = glfwGetKey(win, GLFW_KEY_SPACE) == GLFW_PRESS;
            bool shiftPressed = glfwGetKey(win, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || 
                               glfwGetKey(win, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
            
            if (spacePressed) {
                dx += forwardX * speed;
                dy += forwardY * speed;
                dz += forwardZ * speed;
                moved = true;
            }
            if (shiftPressed) {
                dx -= forwardX * speed;
                dy -= forwardY * speed;
                dz -= forwardZ * speed;
                moved = true;
            }
            
            if (glfwGetKey(win, GLFW_KEY_Q) == GLFW_PRESS) {
                dy += speed * 0.5f;
                moved = true;
            }
            if (glfwGetKey(win, GLFW_KEY_E) == GLFW_PRESS) {
                dy -= speed * 0.5f;
                moved = true;
            }
            
            if (moved) {
                float len = sqrt(dx*dx + dy*dy + dz*dz);
                if (len > 0) {
                    dx /= len;
                    dy /= len;
                    dz /= len;
                    dx *= speed;
                    dy *= speed;
                    dz *= speed;
                }
                
                engine.moveGodParticle(dx, dy, dz);
                
                gui.selectedParticleIndex = godIdx;
                gui.inspectorPosX = engine.getParticleX(godIdx);
                gui.inspectorPosY = engine.getParticleY(godIdx);
                gui.inspectorPosZ = engine.getParticleZ(godIdx);
            }
            
            if (glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
                engine.releaseGodParticle();
                std::cout << "🌟 GOD PARTICLE RELEASED" << std::endl;
            }
        }
        
        // FOLLOW – kamera sleduje God Particle
        if (godIdx >= 0 && gui.followParticle) {
            float gx = engine.getParticleX(godIdx);
            float gy = engine.getParticleY(godIdx);
            float gz = engine.getParticleZ(godIdx);
            renderer.setCameraTarget(gx, gy, gz);
        }
        
        // GUI INTERAKCIA
        bool guiPlaying = gui.isPlaying;
        bool guiStop = gui.stopPressed;
        bool guiStepFwd = gui.stepForward;
        bool guiStepBwd = gui.stepBackward;
        float guiTimeline = gui.timelinePosition;
        
        if (guiPlaying != !engine.isPaused()) {
            engine.togglePause();
            std::cout << (engine.isPaused() ? "[PAUSE]" : "[PLAY]") << std::endl;
        }
        
        if (guiStop) {
            std::cout << "[STOP]" << std::endl;
            gui.stopPressed = false;
            engine.setTime(0.0f);
            gui.timelinePosition = 0.0f;
            if (!engine.isPaused()) {
                engine.togglePause();
            }
            if (godIdx >= 0) {
                engine.releaseGodParticle();
                std::cout << "🌟 GOD PARTICLE RELEASED" << std::endl;
            }
        }
        
        if (guiStepFwd) {
            std::cout << "[STEP FORWARD] dt=" << gui.timeStep << "s" << std::endl;
            gui.stepForward = false;
            engine.saveState();
            engine.step(gui.timeStep);
            gui.timelinePosition = engine.getTime();
        }
        
        if (guiStepBwd) {
            std::cout << "[STEP BACKWARD]" << std::endl;
            gui.stepBackward = false;
            float t = gui.timelinePosition - gui.timeStep;
            if (t < 0) t = 0;
            engine.setTime(t);
            gui.timelinePosition = t;
        }
        
        if (guiTimeline != engine.getTime()) {
            engine.setTime(guiTimeline);
        }
        
        // SIMULÁCIA
        if (!engine.isPaused()) {
            int steps = gui.stepsPerFrame;
            float dt = gui.timeStep;
            for (int i = 0; i < steps; i++) {
                engine.step(dt);
            }
            gui.timelinePosition = engine.getTime();
            if (frame % 10 == 0) engine.saveState();
        }
        
        // AKTUALIZÁCIA INŠPEKTORA
        if (gui.selectedParticleIndex >= 0 && 
            gui.selectedParticleIndex < engine.get_particle_count() &&
            inspectorUpdateCounter % 5 == 0) {
            int idx = gui.selectedParticleIndex;
            gui.inspectorPosX = engine.getParticleX(idx);
            gui.inspectorPosY = engine.getParticleY(idx);
            gui.inspectorPosZ = engine.getParticleZ(idx);
            gui.inspectorVx = engine.getParticleVx(idx);
            gui.inspectorVy = engine.getParticleVy(idx);
            gui.inspectorVz = engine.getParticleVz(idx);
            gui.inspectorMass = engine.getParticleMass(idx);
            gui.inspectorCharge = engine.getParticleCharge(idx);
            gui.inspectorSpin = engine.getParticleSpin(idx);
            gui.inspectorEnergy = engine.getParticleEnergy(idx);
            gui.inspectorTemperature = engine.getParticleTemperature(idx);
            
            gui.manualMass = gui.inspectorMass;
            gui.manualCharge = gui.inspectorCharge;
            gui.manualSpin = gui.inspectorSpin;
            gui.manualVx = gui.inspectorVx;
            gui.manualVy = gui.inspectorVy;
            gui.manualVz = gui.inspectorVz;
        }
        inspectorUpdateCounter++;
        
        // MANUÁLNE ZMENY
        if (gui.applyChanges && 
            gui.selectedParticleIndex >= 0 && 
            gui.selectedParticleIndex < engine.get_particle_count()) {
            int idx = gui.selectedParticleIndex;
            engine.setParticleMass(idx, gui.manualMass);
            engine.setParticleCharge(idx, gui.manualCharge);
            engine.setParticleSpin(idx, gui.manualSpin);
            engine.setParticleVelocity(idx, gui.manualVx, gui.manualVy, gui.manualVz);
            std::cout << "Applied changes to particle " << idx << std::endl;
            gui.applyChanges = false;
        }
        
        // FOLLOW – VÝBER ČASTICE MYŠOU
        auto& interaction = renderer.getInteraction();
        
        if (interaction.leftMouseDown && gui.selectedTool == 5) {
            float* positions = engine.get_positions();
            int count = engine.get_particle_count();
            float minDist = 2.0f;
            int bestIdx = -1;
            
            for (int i = 0; i < count; i++) {
                float dx = positions[i*3] - interaction.worldX;
                float dy = positions[i*3+1] - interaction.worldY;
                float dz = positions[i*3+2] - interaction.worldZ;
                float dist = sqrt(dx*dx + dy*dy + dz*dz);
                if (dist < minDist) {
                    minDist = dist;
                    bestIdx = i;
                }
            }
            
            if (bestIdx >= 0) {
                gui.selectedParticleIndex = bestIdx;
                gui.followParticle = true;
                std::cout << "🔍 Selected particle: " << bestIdx << std::endl;
            }
            interaction.leftMouseDown = false;
        }
        
        if (interaction.leftMouseDown && gui.selectedTool == 0) {
            engine.add_particle(interaction.worldX, interaction.worldY, interaction.worldZ, ParticleType::ELECTRON);
            interaction.leftMouseDown = false;
        }
        
        if (interaction.rightMouseDown) {
            if (gui.selectedTool == 1) {
                engine.applyForce(interaction.worldX, interaction.worldY, interaction.worldZ, 5.0f, false);
            } else if (gui.selectedTool == 2) {
                engine.applyForce(interaction.worldX, interaction.worldY, interaction.worldZ, 5.0f, true);
            }
        }
        
        // RENDER
        glClearColor(0.02f, 0.02f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        int width, height;
        glfwGetWindowSize(renderer.getWindow(), &width, &height);
        glViewport(0, 0, width, height);
        
        renderer.render(engine.get_positions(), engine.get_particle_count());
        
        // GUI
        SimulationStats stats;
        stats.particleCount = engine.get_particle_count();
        stats.simulationTime = engine.getTime();
        stats.isPaused = engine.isPaused();
        stats.simulationSpeed = engine.getSpeed();
        stats.frameCount = frame;
        
        gui.newFrame();
        gui.update(stats);
        gui.drawParticleInspector();  // <-- LEN RAZ!
        gui.render();
        
        frame++;
        if (frame % 100 == 0) {
            auto now = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
            float fps = 100.0f / (elapsed / 1000.0f);
            std::cout << "FPS: " << fps << " | Time: " << engine.getTime() << "s | Particles: " << engine.get_particle_count() 
                      << " | States: " << engine.getStateCount() << " | God: " << (godIdx >= 0 ? "YES" : "NO") << std::endl;
            start = now;
        }
        
        auto frameEnd = std::chrono::high_resolution_clock::now();
        auto frameDurationActual = std::chrono::duration_cast<std::chrono::milliseconds>(frameEnd - frameStart);
        if (frameDurationActual < frameDuration) {
            std::this_thread::sleep_for(frameDuration - frameDurationActual);
        }
    }
    
    gui.shutdown();
    renderer.shutdown();
    engine.shutdown();
    std::cout << "\nUNIVERSE SIMULATION FINISHED!" << std::endl;
    return 0;
}
