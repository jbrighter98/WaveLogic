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
    generatePlane(100.0f, 1000, planeVertices, planeIndices); // 100x100 size, 1000x1000 resolution

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
    std::vector<WaveParameters> activeWaves;
    activeWaves.push_back({0.5f, 0.8f, 1.5f, 0.4f, 1.0f, 0.2f, 0.0f}); // Main swell
    activeWaves.push_back({0.2f, 1.5f, 2.5f, 0.2f, 0.3f, 0.8f, 0.0f}); // Smaller, faster chop
    activeWaves.push_back({0.1f, 3.0f, 4.0f, 0.1f, -0.5f, 0.5f, 0.0f}); // High-frequency ripples

    float lastFrame = 0.0f;


    // --- Render Loop ---
    while (!glfwWindowShouldClose(window)) {
        // Input
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);


        // Calculate DeltaTime
        float currentFrame = glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Render
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Start using shader
        // *** Polygon mode to see the wireframe of the plane ***
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // See the "mesh" structure
        mainShader.use();

        mainShader.setInt("numWaves", (int)activeWaves.size());

        for (int i = 0; i < activeWaves.size(); ++i) {

            activeWaves[i].phase += activeWaves[i].speed * deltaTime;

            std::string wavePrefix = "waves[" + std::to_string(i) + "]";
            mainShader.setFloat(wavePrefix + ".amplitude", activeWaves[i].amplitude);
            mainShader.setFloat(wavePrefix + ".frequency", activeWaves[i].frequency);
            mainShader.setFloat(wavePrefix + ".speed", activeWaves[i].speed);
            mainShader.setFloat(wavePrefix + ".steepness", activeWaves[i].steepness);
            mainShader.setFloat(wavePrefix + ".phase", activeWaves[i].phase);
            mainShader.setVec2(wavePrefix + ".direction", glm::vec2(activeWaves[i].directionX, activeWaves[i].directionZ));
        }


        glm::vec3 currentBlue = glm::vec3(0.0f, 0.5f, 1.0f);
        // Send it to the "waterColor" uniform in the shader
        unsigned int colorLoc = glGetUniformLocation(mainShader.ID, "waterColor");
        glUniform3fv(colorLoc, 1, glm::value_ptr(currentBlue));

        // Create the Model Matrix (Rotate over time)
        glm::mat4 model = glm::mat4(1.0f);
        //model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.5f, 1.0f, 0.0f));

        // Create the View Matrix (The Camera)
        glm::mat4 view = glm::mat4(1.0f);
        view = glm::translate(view, glm::vec3(0.0f, -0.5f, -7.0f)); 
        view = glm::rotate(view, glm::radians(10.0f), glm::vec3(1.0f, 0.0f, 0.0f));

        // Create the Projection Matrix (Perspective)
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)currentWidth / (float)currentHeight, 0.1f, 100.0f);

        // Send these matrices to the Shaders
        mainShader.setMat4("model", model);
        mainShader.setMat4("view", view);
        mainShader.setMat4("projection", projection);

        // Finally, Draw the Cube
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, planeIndices.size(), GL_UNSIGNED_INT, 0);

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}