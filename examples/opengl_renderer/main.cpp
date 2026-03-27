#include <glad/glad.h>  // GLAD must be included before GLFW
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

#include <WaveLogic.h>

#include "Shader.hpp"

std::string VERSION = "1.0.0";

// Window dimensions
int currentWidth = 800;
int currentHeight = 600;

// Callback for resizing the window
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    if (height == 0) height = 1;
    glViewport(0, 0, width, height);
    currentWidth = width;
    currentHeight = height;
}

// Generate plane vertices
void generatePlane(float size, int resolution, std::vector<float>& vertices, std::vector<unsigned int>& indices) {
    float step = size / resolution;
    
    // Generate Vertices
    for (int i = 0; i <= resolution; ++i) {
        for (int j = 0; j <= resolution; ++j) {
            float x = -size / 2.0f + j * step;
            float z = -size / 2.0f + i * step;
            float y = 0.0f; // Flat plane for now

            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
        }
    }

    // Generate Indices (Two triangles per grid square)
    for (int i = 0; i < resolution; ++i) {
        for (int j = 0; j < resolution; ++j) {
            int start = i * (resolution + 1) + j;
            // Triangle 1
            indices.push_back(start);
            indices.push_back(start + resolution + 1);
            indices.push_back(start + 1);
            // Triangle 2
            indices.push_back(start + 1);
            indices.push_back(start + resolution + 1);
            indices.push_back(start + resolution + 2);
        }
    }
}




int main() {
    // Initialize GLFW
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create Window
    GLFWwindow* window = glfwCreateWindow(currentWidth, currentHeight, ("WaveLogic OpenGL Renderer v" + VERSION).c_str(), NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Configure Global OpenGL State
    glEnable(GL_DEPTH_TEST); 


    // define vertices, shaders
    std::vector<float> planeVertices;
    std::vector<unsigned int> planeIndices;
    generatePlane(20.0f, 200, planeVertices, planeIndices); // 20x20 size, 200x200 resolution

    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // Upload Vertices
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, planeVertices.size() * sizeof(float), &planeVertices[0], GL_STATIC_DRAW);

    // Upload Indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, planeIndices.size() * sizeof(unsigned int), &planeIndices[0], GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);




    Shader mainShader("shader.vs", "shader.fs");


    // --- Initialization (Outside the loop) ---
    std::vector<WaveLogic::WaveParameters> activeWaves = WaveLogic::Simulator::GenerateSeaState(5.0f, 45.0f, 5.0f, 16, 123);

    float lastFrame = 0.0f;


    // --- Render Loop ---
    while (!glfwWindowShouldClose(window)) {
        
        // INPUTS

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);


        // DELTA TIME CALCULATION

        float currentFrame = glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;


        // RENDER

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        // SHADER

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Mesh wireframe mode for debugging
        //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Default fill mode
        mainShader.use();


        // LIGHTING

        mainShader.setVec3("lightPos", glm::vec3(0.0f, 10.0f, 0.0f)); // Light coming from above


        // CAMERA

        // Look from far away and high up to see the "Pattern"
        glm::vec3 cameraPos = glm::vec3(0.0f, 5.0f, 10.0f); 
        glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        mainShader.setVec3("viewPos", cameraPos);
        mainShader.setMat4("view", view);


        // WAVES

        glm::vec3 waterColor = glm::vec3(0.1, 0.4, 0.6);
        mainShader.setVec3("waterColor", waterColor);

        mainShader.setInt("numWaves", (int)activeWaves.size());

        if (activeWaves.size() > 16) {
            std::cout << "WARNING: Too many waves for shader capacity! Maximum is 16." << std::endl;
        }

        WaveLogic::Simulator::UpdateWaves(activeWaves, deltaTime);

        for (int i = 0; i < activeWaves.size(); ++i) {

            std::string wavePrefix = "waves[" + std::to_string(i) + "]";
            mainShader.setFloat(wavePrefix + ".amplitude", activeWaves[i].amplitude);
            mainShader.setFloat(wavePrefix + ".frequency", activeWaves[i].frequency);
            mainShader.setFloat(wavePrefix + ".speed", activeWaves[i].speed);
            mainShader.setFloat(wavePrefix + ".steepness", activeWaves[i].steepness);
            mainShader.setFloat(wavePrefix + ".phase", activeWaves[i].phase);
            mainShader.setVec2(wavePrefix + ".direction", glm::vec2(activeWaves[i].directionX, activeWaves[i].directionZ));
        }


        // MODEL & PROJECTION MATRICES

        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)currentWidth / (float)currentHeight, 0.1f, 100.0f);

        mainShader.setMat4("model", model);        
        mainShader.setMat4("projection", projection);


        // DRAW
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, planeIndices.size(), GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}