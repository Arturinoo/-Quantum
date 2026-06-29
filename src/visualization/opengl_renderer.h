#pragma once
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace QuantumEngine {

struct InteractionState {
    bool leftMouseDown = false;
    bool rightMouseDown = false;
    double mouseX = 0.0;
    double mouseY = 0.0;
    double worldX = 0.0;
    double worldY = 0.0;
    double worldZ = 0.0;
    
    enum Tool { NONE, CREATE, EXPLODE, GRAVITY, FIELD, BARRIER, FOLLOW };
    Tool currentTool = NONE;
    
    int followParticleIndex = -1;
    bool followMode = false;
};

class OpenGLRenderer {
private:
    GLFWwindow* window;
    GLuint VAO, VBO, colorVBO;
    int particleCount = 0;
    bool initialized = false;
    GLuint shaderProgram;
    
    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 10.0f);
    glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    float cameraDistance = 10.0f;
    float cameraAngleX = 0.0f;
    float cameraAngleY = 0.0f;
    double lastMouseX, lastMouseY;
    bool firstMouse = true;
    
    float moveSpeed = 5.0f;
    bool freeCam = false;
    bool shiftPressed = false;
    
    InteractionState interaction;
    std::vector<float> interactionPositions;
    
public:
    OpenGLRenderer();
    ~OpenGLRenderer();
    
    bool initialize(int width, int height, const char* title);
    void shutdown();
    void render(const float* positions, int count);
    bool shouldClose();
    void pollEvents();
    
    void setCameraDistance(float dist);
    void rotateCamera(float dx, float dy);
    void panCamera(float dx, float dy);
    void resetCamera();
    void setFreeCam(bool enabled) { freeCam = enabled; }
    void setFollowParticle(int index);
    void updateCamera(const float* positions, int count);
    
    // ============================================================
    // NOVÁ FUNKCIA – pre follow
    // ============================================================
    void setCameraTarget(float x, float y, float z);
    
    InteractionState& getInteraction() { return interaction; }
    void clearInteractionPositions();
    void addInteractionPosition(float x, float y, float z);
    const std::vector<float>& getInteractionPositions() const { return interactionPositions; }
    void setParticlePositions(const float* positions, int count);
    
    GLFWwindow* getWindow() { return window; }
    
private:
    void setupCallbacks();
    void updateCameraMatrices(int width, int height);
    void updateWorldCoordinates(double xpos, double ypos);
    void handleFreeCam(float dt);
    void drawInteractionTools();
    int pickParticle(double xpos, double ypos, const float* positions, int count);
    
    static void mouseCallback(GLFWwindow* window, double xpos, double ypos);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
};

} // namespace QuantumEngine
