#include "Mesh.h"

// Constructor
Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::shared_ptr<Material> material)
    : m_vertices(vertices), m_indices(indices), m_material(material)
{
    // Now that we have all the required data, set the vertex buffers and its attribute pointers.
    setupMesh();
}
// Mesh::Mesh(int dummy, std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::shared_ptr<Material> material)
//     : m_vertices(std::move(vertices)),
//       m_indices(std::move(indices)),
//       m_material(std::move(material))
// {
//     // The rest of the constructor body is the same
//     setupMesh();
// }


// Destructor
Mesh::~Mesh() {
    // Clean up the GPU resources when the mesh object is destroyed
    glDeleteVertexArrays(1, &m_VAO);
    glDeleteBuffers(1, &m_VBO);
    glDeleteBuffers(1, &m_EBO);
}

// Move Constructor
Mesh::Mesh(Mesh&& other) noexcept
    : m_vertices(std::move(other.m_vertices)),
      m_indices(std::move(other.m_indices)),
      m_material(std::move(other.m_material)),
      m_VAO(other.m_VAO),
      m_VBO(other.m_VBO),
      m_EBO(other.m_EBO)
{
    // Invalidate the source object so its destructor does nothing
    other.m_VAO = 0;
    other.m_VBO = 0;
    other.m_EBO = 0;
}

// Move Assignment Operator
Mesh& Mesh::operator=(Mesh&& other) noexcept {
    if (this != &other) {
        // Free our own existing resources
        glDeleteVertexArrays(1, &m_VAO);
        glDeleteBuffers(1, &m_VBO);
        glDeleteBuffers(1, &m_EBO);

        // Steal the data from the source object
        m_vertices = std::move(other.m_vertices);
        m_indices = std::move(other.m_indices);
        m_material = std::move(other.m_material);
        m_VAO = other.m_VAO;
        m_VBO = other.m_VBO;
        m_EBO = other.m_EBO;

        // Invalidate the source object
        other.m_VAO = 0;
        other.m_VBO = 0;
        other.m_EBO = 0;
    }
    return *this;
}

// The new simplified Draw function
// void Mesh::Draw(Shader& shader,int& textureUnit) 
// {
//     // Use the material to set up the shader (bind textures, set uniforms)
//     if (m_material) {
//         m_material->apply(textureUnit); // We'll use the apply() method we defined earlier
//     }
    
//     // Draw the mesh
//     glBindVertexArray(m_VAO);
//     glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(m_indices.size()), GL_UNSIGNED_INT, 0);
//     glBindVertexArray(0);

//     // Always good practice to set everything back to defaults once configured.
//     glActiveTexture(GL_TEXTURE0);
// }

void Mesh::DrawWithMaterial(Shader& shader, int& textureUnit) {
    // This code is already correct. It applies the material and then draws.
    if (m_material) {
        m_material->apply(shader, textureUnit);
    }
    
    glBindVertexArray(m_VAO);
    glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(m_indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // Note: The glActiveTexture(GL_TEXTURE0) at the end is generally not needed
    // and can sometimes cause issues. It's safe to remove it.
}

// 2. Implement the new, simple function
void Mesh::DrawGeometryOnly() {
    // This function doesn't care about shaders or materials.
    // It just binds the vertex data and issues the draw call.
    glBindVertexArray(m_VAO);
    glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(m_indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

// This is your original setupMesh function, which was very well written!
// I've just updated it to use the member variables (e.g., m_VAO).
void Mesh::setupMesh()
{
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);

    glBindVertexArray(m_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), &m_vertices[0], GL_STATIC_DRAW);  

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), &m_indices[0], GL_STATIC_DRAW);

    // Set the vertex attribute pointers
    // Vertex Positions
    glEnableVertexAttribArray(0);	
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));
    // Vertex Normals
    glEnableVertexAttribArray(1);	
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    // Vertex Texture Coords
    glEnableVertexAttribArray(2);	
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
    // Vertex Tangent
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
    // Vertex Bitangent
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));
    // Bone IDs
    glEnableVertexAttribArray(5);
    glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, m_BoneIDs));
    // Bone Weights
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_Weights));
    // Vertex Color
    glEnableVertexAttribArray(7);
    glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    
    glBindVertexArray(0);
}