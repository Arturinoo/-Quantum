#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <string>

namespace QuantumEngine {

struct ToolButton {
    std::string name;
    std::string shortcut;
    int keyCode;
    bool active;
    std::string tooltip;
};

struct SimulationStats {
    int particleCount = 0;
    float simulationTime = 0.0f;
    float fps = 0.0f;
    float totalEnergy = 0.0f;
    float kineticEnergy = 0.0f;
    float potentialEnergy = 0.0f;
    bool isPaused = false;
    float simulationSpeed = 1.0f;
    int frameCount = 0;
};

class ImGuiRenderer {
public:
    ImGuiRenderer();
    ~ImGuiRenderer();
    
    bool initialize(GLFWwindow* window);
    void shutdown();
    void newFrame();
    void render();
    void update(const SimulationStats& stats);
    
    void drawMainMenu();
    void drawToolPanel();
    void drawInfoPanel();
    void drawCameraPanel();
    void drawTimelinePanel();
    void drawStatsPanel();
    void drawTimeControlsPanel();
    void drawParticleInspector();
    
    // Stav
    bool showTools = true;
    bool showInfo = true;
    bool showCamera = true;
    bool showTimeline = true;
    bool showStats = true;
    bool showTimeControls = true;
    int selectedTool = 0;
    float simulationSpeed = 1.0f;
    bool isPaused = false;
    float currentTime = 0.0f;
    int particleCount = 0;
    
    float timelinePosition = 0.0f;
    bool isPlaying = false;
    bool isLooping = false;
    bool stopPressed = false;
    bool stepForward = false;
    bool stepBackward = false;
    
    float timeStep = 0.001f;
    int stepsPerFrame = 1;
    bool usePreciseTime = false;
    
    // PARTICLE INSPECTOR – reálne dáta
    int selectedParticleIndex = -1;
    float manualMass = 1.0f;
    float manualCharge = 0.0f;
    float manualSpin = 0.5f;
    float manualVx = 0.0f;
    float manualVy = 0.0f;
    float manualVz = 0.0f;
    bool followParticle = false;
    bool showGhostArrow = true;
    bool showSpinRotation = true;
    bool applyChanges = false;
    
    // Reálne dáta z enginu (aktualizované každý frame)
    float inspectorPosX = 0.0f;
    float inspectorPosY = 0.0f;
    float inspectorPosZ = 0.0f;
    float inspectorVx = 0.0f;
    float inspectorVy = 0.0f;
    float inspectorVz = 0.0f;
    float inspectorMass = 1.0f;
    float inspectorCharge = 0.0f;
    float inspectorSpin = 0.5f;
    float inspectorEnergy = 0.0f;
    float inspectorTemperature = 300.0f;
    int inspectorType = 0;
    
private:
    GLFWwindow* window;
    bool initialized = false;
    std::vector<ToolButton> tools;
    
    float logSlider(float value, float minVal, float maxVal);
    float expSlider(float value, float minVal, float maxVal);
};

} // namespace QuantumEngine
