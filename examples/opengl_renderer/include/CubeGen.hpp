
#include <vector>

#include <glad/glad.h>
#include <WaveLogic.h>

struct CubeMesh {
    std::vector<float> vertices;    // interleaved: position (3) + normal (3)
    std::vector<unsigned int> indices;
};

struct CubeObject {
    unsigned int VAO, VBO, EBO;
    int indexCount;
    float x, z;
    float size;
};


CubeMesh generateCube(float size)
{
    float h = size;

    // Each face: 4 vertices with a shared flat normal
    // Order: position (x,y,z), normal (x,y,z)
    CubeMesh mesh;
    mesh.vertices = {
        // +Y top
        -h,  h, -h,  0,  1,  0,
         h,  h, -h,  0,  1,  0,
         h,  h,  h,  0,  1,  0,
        -h,  h,  h,  0,  1,  0,

        // -Y bottom
        -h, -h, -h,  0, -1,  0,
         h, -h, -h,  0, -1,  0,
         h, -h,  h,  0, -1,  0,
        -h, -h,  h,  0, -1,  0,

        // +X right
         h, -h, -h,  1,  0,  0,
         h,  h, -h,  1,  0,  0,
         h,  h,  h,  1,  0,  0,
         h, -h,  h,  1,  0,  0,

        // -X left
        -h, -h, -h, -1,  0,  0,
        -h,  h, -h, -1,  0,  0,
        -h,  h,  h, -1,  0,  0,
        -h, -h,  h, -1,  0,  0,

        // +Z front
        -h, -h,  h,  0,  0,  1,
         h, -h,  h,  0,  0,  1,
         h,  h,  h,  0,  0,  1,
        -h,  h,  h,  0,  0,  1,

        // -Z back
        -h, -h, -h,  0,  0, -1,
         h, -h, -h,  0,  0, -1,
         h,  h, -h,  0,  0, -1,
        -h,  h, -h,  0,  0, -1,
    };

    // Two triangles per face, six faces
    for (unsigned int face = 0; face < 6; ++face)
    {
        unsigned int base = face * 4;
        mesh.indices.push_back(base + 0);
        mesh.indices.push_back(base + 1);
        mesh.indices.push_back(base + 2);
        mesh.indices.push_back(base + 2);
        mesh.indices.push_back(base + 3);
        mesh.indices.push_back(base + 0);
    }

    return mesh;
}


CubeObject createCubeObject(float size, float x, float z)
{
    CubeMesh mesh = generateCube(size);

    CubeObject obj;
    obj.size       = size;
    obj.x          = x;
    obj.z          = z;
    obj.indexCount = static_cast<int>(mesh.indices.size());

    glGenVertexArrays(1, &obj.VAO);
    glGenBuffers(1, &obj.VBO);
    glGenBuffers(1, &obj.EBO);

    glBindVertexArray(obj.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, obj.VBO);
    glBufferData(GL_ARRAY_BUFFER,
                 mesh.vertices.size() * sizeof(float),
                 mesh.vertices.data(),
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 mesh.indices.size() * sizeof(unsigned int),
                 mesh.indices.data(),
                 GL_STATIC_DRAW);

    // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                          6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    return obj;
}


void updateAndDrawCube(CubeObject& cube,
                       const std::vector<WaveLogic::WaveParameters>& waves,
                       unsigned int shaderProgram,
                       const glm::mat4& view,
                       const glm::mat4& projection)
{
    WaveLogic::GerstnerResult surface = WaveLogic::Simulator::EvaluateWaveAt(cube.x, cube.z, waves);

    // Sit the bottom face on the surface
    float worldY = surface.position.y - cube.size * 0.5f;

    // Translation
    glm::mat4 model = glm::translate(glm::mat4(1.0f),
                      glm::vec3(cube.x, worldY, cube.z));

    // Tilt to match wave normal
    glm::vec3 up         = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 waveNormal = glm::vec3(surface.normal.x,
                                     surface.normal.y,
                                     surface.normal.z);

    glm::quat tilt;
    float d = glm::dot(up, waveNormal);
    if (d >= 0.9999f) {
        tilt = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    } else if (d <= -0.9999f) {
        tilt = glm::angleAxis(glm::pi<float>(), glm::vec3(1.0f, 0.0f, 0.0f));
    } else {
        glm::vec3 axis = glm::normalize(glm::cross(up, waveNormal));
        float     angle = std::acos(d);
        tilt = glm::angleAxis(angle, axis);
    }

    model = model * glm::mat4_cast(tilt);

    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));

    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"),
                       1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"),
                       1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"),
                       1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix3fv(glGetUniformLocation(shaderProgram, "normalMatrix"),
                       1, GL_FALSE, glm::value_ptr(normalMatrix));

    glBindVertexArray(cube.VAO);
    glDrawElements(GL_TRIANGLES, cube.indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}