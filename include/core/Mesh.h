// #ifndef MESH_H
// #define MESH_H

// #include <glad/glad.h> // holds all OpenGL type declarations
#pragma once
#include <glm/glm.hpp>

#include "Shader.h"
#include "Material.h"
#include "Vertex.h"

#include <string>
#include <vector>
#include <memory>
using namespace std;

#define MAX_BONE_INFLUENCE 4

class Mesh {
public:
  

    // Mesh Data
    std::vector<Vertex>       m_vertices;
    std::vector<unsigned int> m_indices;
    std::shared_ptr<Material> m_material;

  
    
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::shared_ptr<Material> material);
    //  Mesh(int dummy, std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::shared_ptr<Material> material);
    // Destructor to free GPU resources
    ~Mesh();

    // Delete copy operations to prevent this error from ever happening again
    Mesh(const Mesh&) = delete;
    Mesh& operator = (const Mesh&) = delete;

    // Add move operations to allow for efficient transfers
    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;

    

    // The Draw function is now much simpler.
    // void Draw(Shader& shader,int& textureUnit);
    void DrawWithMaterial(Shader& shader, int& textureUnit);
    void DrawGeometryOnly();

    private:
    // Render data
    unsigned int m_VAO, m_VBO, m_EBO;

    // Initializes all the buffer objects/arrays from the vertex data.
    void setupMesh();
};
   