#include "opengl_renderer.h"
#include <iostream>
#include <cmath>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Shadery
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 color;

uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * vec4(aPos, 1.0);
    color = aColor;
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
in vec3 color;
out vec4 FragColor;

void main() {
    FragColor = vec4(color, 1.0);
}
)";

namespace QuantumEngine {

OpenGLRenderer::OpenGLRenderer() {}

OpenGLRenderer::~OpenGLRenderer() {
    shutdown();
}

void OpenGLRenderer::setupCallbacks() {
    glfwSetWindowUserPointer(window, this);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetKeyCallback(window, keyCallback);
}

void OpenGLRenderer::mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    auto renderer = static_cast<OpenGLRenderer*>(glfwGetWindowUserPointer(window));
    if (!renderer) return;
    
    renderer->interaction.mouseX = xpos;
    renderer->interaction.mouseY = ypos;
    renderer->updateWorldCoordinates(xpos, ypos);
    
    if (renderer->firstMouse) {
        renderer->lastMouseX = xpos;
        renderer->lastMouseY = ypos;
        renderer->firstMouse = false;
        return;
    }
    
    double dx = xpos - renderer->lastMouseX;
    double dy = ypos - renderer->lastMouseY;
    renderer->lastMouseX = xpos;
    renderer->lastMouseY = ypos;
    
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        if (!renderer->freeCam) {
            renderer->rotateCamera(dx * 0.005f, dy * 0.005f);
        }
    } else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && 
               glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS) {
        renderer->panCamera(-dx * 0.02f, dy * 0.02f);
    }
}

void OpenGLRenderer::mouseButtonCallback(GLFWwindow* window, int button, int action, int /*mods*/) {
    auto renderer = static_cast<OpenGLRenderer*>(glfwGetWindowUserPointer(window));
    if (!renderer) return;
    
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        renderer->interaction.leftMouseDown = (action == GLFW_PRESS);
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        renderer->interaction.rightMouseDown = (action == GLFW_PRESS);
    }
}

void OpenGLRenderer::scrollCallback(GLFWwindow* window, double /*xoffset*/, double yoffset) {
    auto renderer = static_cast<OpenGLRenderer*>(glfwGetWindowUserPointer(window));
    if (!renderer) return;
    
    float zoomFactor = 1.0f - yoffset * 0.1f;
    if (zoomFactor > 0.0f) {
        renderer->cameraDistance *= zoomFactor;
        renderer->cameraDistance = std::max(0.001f, std::min(10000.0f, renderer->cameraDistance));
    }
}

void OpenGLRenderer::keyCallback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/) {
    auto renderer = static_cast<OpenGLRenderer*>(glfwGetWindowUserPointer(window));
    if (!renderer) return;
    
    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_R:
                renderer->resetCamera();
                std::cout << "🔄 Kamera resetovaná" << std::endl;
                break;
            case GLFW_KEY_C:
                renderer->interaction.currentTool = InteractionState::CREATE;
                std::cout << "🟢 Nástroj: PRIDAŤ ČASTICU (klikni do okna)" << std::endl;
                break;
            case GLFW_KEY_E:
                renderer->interaction.currentTool = InteractionState::EXPLODE;
                std::cout << "💥 Nástroj: ODPUDZOVANIE" << std::endl;
                break;
            case GLFW_KEY_G:
                renderer->interaction.currentTool = InteractionState::GRAVITY;
                std::cout << "🌀 Nástroj: PRIŤAHOVANIE" << std::endl;
                break;
            case GLFW_KEY_F:
                renderer->interaction.currentTool = InteractionState::FIELD;
                std::cout << "🧲 Nástroj: GRAVITAČNÉ POLE" << std::endl;
                break;
            case GLFW_KEY_B:
                renderer->interaction.currentTool = InteractionState::BARRIER;
                std::cout << "🔮 Nástroj: KVANTOVÁ BARIÉRA" << std::endl;
                break;
            case GLFW_KEY_F11:
                renderer->interaction.currentTool = InteractionState::FOLLOW;
                std::cout << "🎯 Nástroj: FOLLOW (klikni na časticu) - F11" << std::endl;
                break;
            case GLFW_KEY_X:
                renderer->interaction.currentTool = InteractionState::NONE;
                renderer->interaction.followMode = false;
                std::cout << "❌ Nástroj: VYPNUTÝ" << std::endl;
                break;
            case GLFW_KEY_LEFT_SHIFT:
            case GLFW_KEY_RIGHT_SHIFT:
                renderer->shiftPressed = true;
                break;
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(window, GLFW_TRUE);
                break;
        }
    }
    
    if (action == GLFW_RELEASE) {
        switch (key) {
            case GLFW_KEY_LEFT_SHIFT:
            case GLFW_KEY_RIGHT_SHIFT:
                renderer->shiftPressed = false;
                break;
        }
    }
}

void OpenGLRenderer::setCameraDistance(float dist) {
    cameraDistance = std::max(0.001f, std::min(10000.0f, dist));
}

void OpenGLRenderer::rotateCamera(float dx, float dy) {
    if (!freeCam) {
        cameraAngleX += dx;
        cameraAngleY += dy;
        cameraAngleY = std::max(-1.57f, std::min(1.57f, cameraAngleY));
    }
}

void OpenGLRenderer::panCamera(float dx, float dy) {
    cameraTarget.x += dx * 0.5f;
    cameraTarget.y += dy * 0.5f;
}

void OpenGLRenderer::resetCamera() {
    cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    cameraAngleX = 0.0f;
    cameraAngleY = 0.0f;
    cameraDistance = 10.0f;
    freeCam = false;
    interaction.followMode = false;
    interaction.followParticleIndex = -1;
}

void OpenGLRenderer::setFollowParticle(int index) {
    interaction.followParticleIndex = index;
    interaction.followMode = (index >= 0);
    if (interaction.followMode) {
        std::cout << "🎯 Sledujem časticu " << index << std::endl;
    }
}

void OpenGLRenderer::updateCamera(const float* positions, int count) {
    if (interaction.followMode && positions && interaction.followParticleIndex < count) {
        int idx = interaction.followParticleIndex;
        float x = positions[idx*3];
        float y = positions[idx*3+1];
        float z = positions[idx*3+2];
        cameraTarget = glm::vec3(x, y, z);
    }
}

void OpenGLRenderer::updateWorldCoordinates(double xpos, double ypos) {
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    
    float ndcX = 2.0f * (xpos / width) - 1.0f;
    float ndcY = 1.0f - 2.0f * (ypos / height);
    
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / height, 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, cameraUp);
    glm::mat4 invVP = glm::inverse(projection * view);
    
    glm::vec4 clipPos = glm::vec4(ndcX, ndcY, 0.5f, 1.0f);
    glm::vec4 worldPos = invVP * clipPos;
    worldPos /= worldPos.w;
    
    interaction.worldX = worldPos.x;
    interaction.worldY = worldPos.y;
    interaction.worldZ = worldPos.z;
}

void OpenGLRenderer::clearInteractionPositions() {
    interactionPositions.clear();
}

void OpenGLRenderer::addInteractionPosition(float x, float y, float z) {
    interactionPositions.push_back(x);
    interactionPositions.push_back(y);
    interactionPositions.push_back(z);
}

void OpenGLRenderer::setParticlePositions(const float* /*positions*/, int /*count*/) {
    // Placeholder
}

void OpenGLRenderer::handleFreeCam(float dt) {
    if (!freeCam) return;
    
    float speed = shiftPressed ? moveSpeed * 2.0f : moveSpeed;
    
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        cameraTarget += glm::vec3(0.0f, 0.0f, -speed * dt);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        cameraTarget += glm::vec3(0.0f, 0.0f, speed * dt);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        cameraTarget += glm::vec3(-speed * dt, 0.0f, 0.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        cameraTarget += glm::vec3(speed * dt, 0.0f, 0.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        cameraTarget += glm::vec3(0.0f, speed * dt, 0.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        cameraTarget += glm::vec3(0.0f, -speed * dt, 0.0f);
    }
}

int OpenGLRenderer::pickParticle(double xpos, double ypos, const float* positions, int count) {
    if (!positions || count == 0) return -1;
    
    updateWorldCoordinates(xpos, ypos);
    float minDist = 1.0f;
    int bestIdx = -1;
    
    for (int i = 0; i < std::min(count, 10000); i++) {
        float dx = positions[i*3] - interaction.worldX;
        float dy = positions[i*3+1] - interaction.worldY;
        float dz = positions[i*3+2] - interaction.worldZ;
        float dist = sqrt(dx*dx + dy*dy + dz*dz);
        if (dist < minDist) {
            minDist = dist;
            bestIdx = i;
        }
    }
    
    return bestIdx;
}

void OpenGLRenderer::updateCameraMatrices(int width, int height) {
    cameraPos.x = cameraTarget.x + cameraDistance * cos(cameraAngleX) * cos(cameraAngleY);
    cameraPos.y = cameraTarget.y + cameraDistance * sin(cameraAngleY);
    cameraPos.z = cameraTarget.z + cameraDistance * sin(cameraAngleX) * cos(cameraAngleY);
    
    glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, cameraUp);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / height, 0.1f, 100.0f);
    
    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
}

void OpenGLRenderer::drawInteractionTools() {
    // Placeholder
}

// ============================================================
// setCameraTarget – SPRÁVNE UMESTNENÁ V NAMESPACE
// ============================================================
void OpenGLRenderer::setCameraTarget(float x, float y, float z) {
    cameraTarget = glm::vec3(x, y, z);
}

bool OpenGLRenderer::initialize(int width, int height, const char* title) {
    if (!glfwInit()) {
        std::cerr << "❌ GLFW inicializácia zlyhala!" << std::endl;
        return false;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    
    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window) {
        std::cerr << "❌ Okno sa nepodarilo vytvoriť!" << std::endl;
        glfwTerminate();
        return false;
    }
    
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "❌ GLEW inicializácia zlyhala!" << std::endl;
        return false;
    }
    
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);
    
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);
    
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &colorVBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    glViewport(0, 0, width, height);
    glClearColor(0.02f, 0.02f, 0.05f, 1.0f);
    
    setupCallbacks();
    
    initialized = true;
    std::cout << "✅ OpenGL renderer inicializovaný!" << std::endl;
    std::cout << "🎮 Ovládanie:" << std::endl;
    std::cout << "   C = Pridať časticu (klikni do okna)" << std::endl;
    std::cout << "   E = Odpudzovanie (pravé tlačidlo)" << std::endl;
    std::cout << "   G = Priťahovanie (pravé tlačidlo)" << std::endl;
    std::cout << "   F = Gravit. pole" << std::endl;
    std::cout << "   B = Kvantová bariéra" << std::endl;
    std::cout << "   F11 = FOLLOW (klikni na časticu)" << std::endl;
    std::cout << "   X = Vypnúť nástroj" << std::endl;
    std::cout << "   Pravé tlačidlo = rotácia" << std::endl;
    std::cout << "   Alt+Ľavé tlačidlo = posun" << std::endl;
    std::cout << "   Scroll = Nekonečný zoom" << std::endl;
    std::cout << "   R = reset" << std::endl;
    return true;
}

void OpenGLRenderer::shutdown() {
    if (initialized) {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &colorVBO);
        glDeleteProgram(shaderProgram);
        glfwDestroyWindow(window);
        glfwTerminate();
        initialized = false;
        std::cout << "🛑 OpenGL renderer vypnutý" << std::endl;
    }
}

void OpenGLRenderer::render(const float* positions, int count) {
    if (!initialized || !positions || count == 0) return;
    
    particleCount = count;
    std::vector<float> posData(count * 3);
    std::vector<float> colorData(count * 3);
    
    for (int i = 0; i < count; i++) {
        posData[i*3] = positions[i*3];
        posData[i*3+1] = positions[i*3+1];
        posData[i*3+2] = positions[i*3+2];
        
        float dist = sqrt(positions[i*3]*positions[i*3] + 
                         positions[i*3+1]*positions[i*3+1] + 
                         positions[i*3+2]*positions[i*3+2]);
        float t = std::min(dist / 5.0f, 1.0f);
        colorData[i*3] = 0.2f + 0.8f * (1.0f - t);
        colorData[i*3+1] = 0.2f + 0.8f * t;
        colorData[i*3+2] = 0.6f + 0.4f * (1.0f - t);
        
        if (interaction.followMode && i == interaction.followParticleIndex) {
            colorData[i*3] = 1.0f;
            colorData[i*3+1] = 0.0f;
            colorData[i*3+2] = 0.0f;
        }
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, posData.size() * sizeof(float), posData.data(), GL_DYNAMIC_DRAW);
    
    glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
    glBufferData(GL_ARRAY_BUFFER, colorData.size() * sizeof(float), colorData.data(), GL_DYNAMIC_DRAW);
    
    glEnable(GL_DEPTH_TEST);
    
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    glViewport(0, 0, width, height);
    updateCameraMatrices(width, height);
    
    glBindVertexArray(VAO);
    glPointSize(2.0f);
    glDrawArrays(GL_POINTS, 0, count);
    glBindVertexArray(0);
}

bool OpenGLRenderer::shouldClose() {
    return glfwWindowShouldClose(window);
}

void OpenGLRenderer::pollEvents() {
    glfwPollEvents();
}

} // namespace QuantumEngine
