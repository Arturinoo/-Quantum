#include "imgui_renderer.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cmath>

namespace QuantumEngine {

ImGuiRenderer::ImGuiRenderer() {
    tools = {
        {"Add Particle", "C", GLFW_KEY_C, false, "Click in window to add particle"},
        {"Repulse", "E", GLFW_KEY_E, false, "Repulses particles from cursor"},
        {"Attract", "G", GLFW_KEY_G, false, "Attracts particles to cursor"},
        {"Gravity Field", "F", GLFW_KEY_F, false, "Creates gravity field"},
        {"Quantum Barrier", "B", GLFW_KEY_B, false, "Adds quantum barrier"},
        {"Follow", "F11", GLFW_KEY_F11, false, "Click particle to select"},
    };
}

ImGuiRenderer::~ImGuiRenderer() {
    shutdown();
}

bool ImGuiRenderer::initialize(GLFWwindow* win) {
    window = win;
    
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 4.0f;
    style.FrameRounding = 4.0f;
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.08f, 0.10f, 0.95f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.12f, 0.12f, 0.15f, 1.0f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.18f, 0.18f, 0.22f, 1.0f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.15f, 0.15f, 0.20f, 1.0f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.25f, 0.25f, 0.32f, 1.0f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.40f, 0.40f, 0.50f, 1.0f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.20f, 0.20f, 0.28f, 1.0f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.30f, 0.30f, 0.40f, 1.0f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.40f, 0.40f, 0.50f, 1.0f);
    
    if (!ImGui_ImplGlfw_InitForOpenGL(window, true)) {
        std::cerr << "ERROR: ImGui_ImplGlfw_InitForOpenGL failed!" << std::endl;
        return false;
    }
    if (!ImGui_ImplOpenGL3_Init("#version 330")) {
        std::cerr << "ERROR: ImGui_ImplOpenGL3_Init failed!" << std::endl;
        return false;
    }
    
    initialized = true;
    std::cout << "ImGui renderer initialized" << std::endl;
    return true;
}

void ImGuiRenderer::shutdown() {
    if (initialized) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        initialized = false;
        std::cout << "ImGui renderer shutdown" << std::endl;
    }
}

void ImGuiRenderer::newFrame() {
    if (!initialized) return;
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiRenderer::update(const SimulationStats& stats) {
    currentTime = stats.simulationTime;
    particleCount = stats.particleCount;
    isPaused = stats.isPaused;
    simulationSpeed = stats.simulationSpeed;
}

void ImGuiRenderer::render() {
    if (!initialized) return;
    
    static int renderCallCount = 0;
    renderCallCount++;
    
    drawMainMenu();
    drawToolPanel();
    drawInfoPanel();
    drawCameraPanel();
    drawTimeControlsPanel();
    drawTimelinePanel();
    drawStatsPanel();
    
    // drawParticleInspector(); // ZAKOMENTOVANE - vola sa z main.cpp
    
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    
    glfwSwapBuffers(window);
}

float ImGuiRenderer::logSlider(float value, float minVal, float maxVal) {
    float logMin = log10(minVal);
    float logMax = log10(maxVal);
    float logVal = logMin + value * (logMax - logMin);
    return pow(10000.0f, logVal);
}

float ImGuiRenderer::expSlider(float value, float minVal, float maxVal) {
    float logMin = log10(minVal);
    float logMax = log10(maxVal);
    return (log10(value) - logMin) / (logMax - logMin);
}

void ImGuiRenderer::drawTimeControlsPanel() {
    if (!showTimeControls) return;
    
    ImGui::SetNextWindowSize(ImVec2(350, 180), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(10, 440), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Time Controls", &showTimeControls)) {
        ImGui::Text("⏱️ Precision Time Control");
        ImGui::Separator();
        
        ImGui::Text("Time Step: %.9f s", timeStep);
        float logValue = expSlider(timeStep, 0.000000001f, 0.1f);
        if (ImGui::SliderFloat("##TimeStep", &logValue, 0.0f, 1.0f, "%.9f")) {
            timeStep = logSlider(logValue, 0.000000001f, 0.1f);
        }
        
        ImGui::Text("Steps per Frame: %d", stepsPerFrame);
        ImGui::SliderInt("##StepsPerFrame", &stepsPerFrame, 1, 100);
        
        ImGui::Separator();
        
        if (ImGui::Button("1 ns")) { timeStep = 0.000000001f; }
        ImGui::SameLine();
        if (ImGui::Button("1 µs")) { timeStep = 0.000001f; }
        ImGui::SameLine();
        if (ImGui::Button("1 ms")) { timeStep = 0.001f; }
        ImGui::SameLine();
        if (ImGui::Button("10 ms")) { timeStep = 0.01f; }
        ImGui::SameLine();
        if (ImGui::Button("100 ms")) { timeStep = 0.1f; }
        
        ImGui::Separator();
        ImGui::Text("Current Time: %.9f s", currentTime);
        ImGui::Text("Precision: %.0f ps", timeStep * 1e12f);
    }
    ImGui::End();
}

void ImGuiRenderer::drawParticleInspector() {
    static int inspectorCallCount = 0;
    inspectorCallCount++;
    
    ImGui::SetNextWindowSize(ImVec2(350, 450), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(370, 620), ImGuiCond_FirstUseEver);
    
    if (!ImGui::Begin("Particle Inspector", nullptr)) {
        ImGui::End();
        return;
    }
    
    if (selectedParticleIndex < 0) {
        ImGui::Text("🔍 No particle selected");
        ImGui::Text("Press 'F11' and click a particle");
        
        if (ImGui::Button("Select Random Particle")) {
            selectedParticleIndex = rand() % 10000;
            std::cout << "Selected particle: " << selectedParticleIndex << std::endl;
        }
    } else {
        ImGui::Text("🔬 Particle #%d", selectedParticleIndex);
        ImGui::Separator();
        
        ImGui::Text("📍 Position");
        ImGui::Text("  X: %.3f", inspectorPosX);
        ImGui::Text("  Y: %.3f", inspectorPosY);
        ImGui::Text("  Z: %.3f", inspectorPosZ);
        
        ImGui::Separator();
        ImGui::Text("🚀 Velocity");
        ImGui::Text("  Vx: %.3f", inspectorVx);
        ImGui::Text("  Vy: %.3f", inspectorVy);
        ImGui::Text("  Vz: %.3f", inspectorVz);
        
        ImGui::Separator();
        ImGui::Text("⚛️ Properties");
        ImGui::Text("  Mass: %.3e kg", inspectorMass);
        ImGui::Text("  Charge: %.3e C", inspectorCharge);
        ImGui::Text("  Spin: %.2f", inspectorSpin);
        ImGui::Text("  Energy: %.3f J", inspectorEnergy);
        ImGui::Text("  Temperature: %.1f K", inspectorTemperature);
        
        ImGui::Separator();
        ImGui::Text("🎯 Controls");
        
        if (ImGui::Button(followParticle ? "🔴 Stop Follow" : "🟢 Follow")) {
            followParticle = !followParticle;
            std::cout << (followParticle ? "Following particle " : "Stopped following") << selectedParticleIndex << std::endl;
        }
        ImGui::SameLine();
        if (ImGui::Button(showGhostArrow ? "👻 Hide Arrow" : "👻 Show Arrow")) {
            showGhostArrow = !showGhostArrow;
        }
        ImGui::SameLine();
        if (ImGui::Button(showSpinRotation ? "🔄 Hide Spin" : "🔄 Show Spin")) {
            showSpinRotation = !showSpinRotation;
        }
        
        ImGui::Separator();
        ImGui::Text("✏️ Manual Override");
        
        ImGui::SliderFloat("Mass", &manualMass, 0.1f, 10000.0f, "%.2f");
        ImGui::SliderFloat("Charge", &manualCharge, -5.0f, 5.0f, "%.2f");
        ImGui::SliderFloat("Spin", &manualSpin, 0.0f, 2.0f, "%.2f");
        ImGui::SliderFloat("Vx", &manualVx, -5.0f, 5.0f, "%.2f");
        ImGui::SliderFloat("Vy", &manualVy, -5.0f, 5.0f, "%.2f");
        ImGui::SliderFloat("Vz", &manualVz, -5.0f, 5.0f, "%.2f");
        
        if (ImGui::Button("Apply Changes")) {
            applyChanges = true;
            std::cout << "Applied changes to particle " << selectedParticleIndex << std::endl;
        }
        ImGui::SameLine();
        if (ImGui::Button("Deselect")) {
            selectedParticleIndex = -1;
            followParticle = false;
        }
    }
    
    ImGui::End();
}

void ImGuiRenderer::drawMainMenu() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Save Simulation", "Ctrl+S")) {
                std::cout << "Saving simulation..." << std::endl;
            }
            if (ImGui::MenuItem("Load Simulation", "Ctrl+O")) {
                std::cout << "Loading simulation..." << std::endl;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Reset", "Ctrl+R")) {
                std::cout << "Reset simulation" << std::endl;
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Tools", nullptr, &showTools);
            ImGui::MenuItem("Info", nullptr, &showInfo);
            ImGui::MenuItem("Camera", nullptr, &showCamera);
            ImGui::MenuItem("Time Controls", nullptr, &showTimeControls);
            ImGui::MenuItem("Timeline", nullptr, &showTimeline);
            ImGui::MenuItem("Stats", nullptr, &showStats);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("Controls", "F1")) {
                std::cout << "Showing controls..." << std::endl;
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void ImGuiRenderer::drawToolPanel() {
    if (!showTools) return;
    
    ImGui::SetNextWindowSize(ImVec2(200, 400), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(10, 30), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Tools", &showTools)) {
        ImGui::Text("Tools");
        ImGui::Separator();
        
        for (size_t i = 0; i < tools.size(); i++) {
            auto& tool = tools[i];
            bool isActive = (selectedTool == (int)i);
            
            ImGui::PushID(i);
            if (ImGui::Selectable(tool.name.c_str(), isActive)) {
                selectedTool = i;
                std::cout << "Selected tool: " << tool.name << std::endl;
            }
            
            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                ImGui::Text("Shortcut: %s", tool.shortcut.c_str());
                ImGui::Text("%s", tool.tooltip.c_str());
                ImGui::EndTooltip();
            }
            ImGui::PopID();
        }
        
        ImGui::Separator();
        ImGui::Text("Controls");
        ImGui::Text("  Right click = rotate");
        ImGui::Text("  Alt+Left = pan");
        ImGui::Text("  Scroll = zoom");
        ImGui::Text("  R = reset camera");
    }
    ImGui::End();
}

void ImGuiRenderer::drawInfoPanel() {
    if (!showInfo) return;
    
    ImGui::SetNextWindowSize(ImVec2(250, 200), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(220, 30), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Info", &showInfo)) {
        ImGui::Text("Statistics");
        ImGui::Separator();
        ImGui::Text("Particles: %d", particleCount);
        ImGui::Text("FPS: %.1f", 45.0f);
        ImGui::Text("Time: %.9f s", currentTime);
        
        ImGui::Separator();
        ImGui::Text("CUDA: ENABLED");
        ImGui::Text("Memory: 256 MB");
    }
    ImGui::End();
}

void ImGuiRenderer::drawCameraPanel() {
    if (!showCamera) return;
    
    ImGui::SetNextWindowSize(ImVec2(250, 150), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(480, 30), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Camera", &showCamera)) {
        ImGui::Text("Camera Controls");
        ImGui::Separator();
        
        ImGui::SliderFloat("Speed", &simulationSpeed, 0.0f, 3.0f, "%.1fx");
        
        if (ImGui::Button(isPaused ? "Play" : "Pause")) {
            isPaused = !isPaused;
            std::cout << (isPaused ? "Simulation paused" : "Simulation resumed") << std::endl;
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset")) {
            std::cout << "Camera reset" << std::endl;
        }
    }
    ImGui::End();
}

void ImGuiRenderer::drawTimelinePanel() {
    if (!showTimeline) return;
    
    ImGui::SetNextWindowSize(ImVec2(700, 160), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(10, 630), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Timeline", &showTimeline)) {
        ImGui::Text("Time: %.9f s / 10.00 s", currentTime);
        ImGui::Separator();
        
        if (ImGui::Button(isPlaying ? "Pause" : "Play")) {
            isPlaying = !isPlaying;
            std::cout << (isPlaying ? "Play" : "Pause") << std::endl;
        }
        ImGui::SameLine();
        if (ImGui::Button("Stop")) {
            stopPressed = true;
            isPlaying = false;
            std::cout << "Stop" << std::endl;
        }
        ImGui::SameLine();
        if (ImGui::Button("Step <")) {
            stepBackward = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Step >")) {
            stepForward = true;
        }
        ImGui::SameLine();
        if (ImGui::Button(isLooping ? "Loop ON" : "Loop OFF")) {
            isLooping = !isLooping;
            std::cout << (isLooping ? "Loop ON" : "Loop OFF") << std::endl;
        }
        
        ImGui::Separator();
        ImGui::SetNextItemWidth(400);
        if (ImGui::SliderFloat("##TimeSlider", &timelinePosition, 0.0f, 10000.0f, "%.9f s")) {
        }
    }
    ImGui::End();
}

void ImGuiRenderer::drawStatsPanel() {
    if (!showStats) return;
    
    ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(740, 30), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Statistics", &showStats)) {
        ImGui::Text("Detailed Stats");
        ImGui::Separator();
        
        ImGui::Text("Particles: %d", particleCount);
        ImGui::Text("Simulation Time: %.9f s", currentTime);
        ImGui::Text("FPS: %.1f", 45.0f);
        ImGui::Text("Speed: %.1fx", simulationSpeed);
        ImGui::Text("Status: %s", isPaused ? "Paused" : "Running");
        
        ImGui::Separator();
        ImGui::Text("Energy");
        ImGui::Text("  Total: %.3f", 12.45f);
        ImGui::Text("  Kinetic: %.3f", 8.12f);
        ImGui::Text("  Potential: %.3f", 4.33f);
    }
    ImGui::End();
}

} // namespace QuantumEngine
