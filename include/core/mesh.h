// #ifndef MESH_H
// #define MESH_H

// #include <glad/glad.h> // holds all OpenGL type declarations
#pragma once
#include <glm/glm.hpp>

#include "shader.h"
#include "material.h"
#include "vertex.h"

#include <string>
#include <vector>
#include <memory>
using namespace std;

#define MAX_BONE_INFLUENCE 4

// struct Vertex {
//     // position
//     glm::vec3 Position;
//     // normal
//     glm::vec3 Normal;
//     // texCoords
//     glm::vec2 TexCoords;
//     // tangent
//     glm::vec3 Tangent;
//     // bitangent
//     glm::vec3 Bitangent;
//     //color
//     glm::vec4 color;

//     float useDiffuseTexture;

// 	//bone indexes which will influence this vertex
// 	int m_BoneIDs[MAX_BONE_INFLUENCE];
// 	//weights from each bone
// 	float m_Weights[MAX_BONE_INFLUENCE];
// };

// struct Texture {
//     unsigned int id;
//     string type;
//     string path;
// };

// struct Material {
// glm::vec3 Diffuse;
// glm::vec3 Specular;
// glm::vec3 Ambient;
// float Shininess;
// };
class Mesh {
public:
    // // mesh Data
    // vector<Vertex>       vertices;
    // unsigned int VAO;
    // vector<unsigned int> indices;
    // // vector<Texture*>      textures;
    // Material* material;


    // Mesh Data
    std::vector<Vertex>       m_vertices;
    std::vector<unsigned int> m_indices;
    std::shared_ptr<Material> m_material;

    // constructor
    // Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures)
    // {
    //     this->vertices = vertices;
    //     this->indices = indices;
    //     this->textures = textures;

    //     // now that we have all the required data, set the vertex buffers and its attribute pointers.
    //     setupMesh();
    // }
    
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::shared_ptr<Material> material);

    // Destructor to free GPU resources
    ~Mesh();

    // Delete copy operations to prevent this error from ever happening again
    Mesh(const Mesh&) = delete;
    Mesh& operator = (const Mesh&) = delete;

    // Add move operations to allow for efficient transfers
    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;

    // The Draw function is now much simpler.
    void Draw(Shader& shader);

    private:
    // Render data
    unsigned int m_VAO, m_VBO, m_EBO;

    // Initializes all the buffer objects/arrays from the vertex data.
    void setupMesh();
};
    // render the mesh
    // void Draw(Shader &shader) 
    // {
    //     // bind appropriate textures
    //     unsigned int diffuseNr  = 1;
    //     unsigned int specularNr = 1;
    //     unsigned int normalNr   = 1;
    //     unsigned int heightNr   = 1;
    //     for(unsigned int i = 0; i < textures.size(); i++)
    //     {
    //         glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before binding
    //         // retrieve texture number (the N in diffuse_textureN)
    //         // string number;
    //         string name = textures[i]->textureType;
    //         string shader_propertie_name;
    //         if(name == "texture_diffuse")
    //             // number = std::to_string(diffuseNr++);
    //             shader_propertie_name = "material.diffuse";
    //         else if(name == "texture_specular")
    //             shader_propertie_name = "material.specular";
    //         else if(name == "texture_normal")
    //             shader_propertie_name = "material.normal";
    //         else if(name == "texture_height")
    //             shader_propertie_name = "material.height";
    //         else if(name == "texture_emmision")
    //             shader_propertie_name = "material.emmision";
            
    //         else if(name == "texture_shininess")
    //             shader_propertie_name = "material.shininess";
                
    //         else if(name == "texture_tnormals")
    //             shader_propertie_name = "material.tnormals";

    //         // now set the sampler to the correct texture unit
    //         glUniform1i(glGetUniformLocation(shader.ID, shader_propertie_name.c_str()), i);
    //         // and finally bind the texture
    //         glBindTexture(GL_TEXTURE_2D, textures[i].id);
    //     }
        
    //     // draw mesh
    //     glBindVertexArray(VAO);
    //     glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
    //     glBindVertexArray(0);

    //     // always good practice to set everything back to defaults once configured.
    //     glActiveTexture(GL_TEXTURE0);
    // }

//     void draw() const {
//         material->apply();
//         glBindVertexArray(VAO);
//         glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()) , GL_UNSIGNED_INT, 0);
//         glBindVertexArray(0);
//         glActiveTexture(GL_TEXTURE0); // reset
//     }
// private:
//     // render data 
//     unsigned int VBO, EBO;

//     // initializes all the buffer objects/arrays
//     void setupMesh()
//     {
//         // create buffers/arrays
//         glGenVertexArrays(1, &VAO);
//         glGenBuffers(1, &VBO);
//         glGenBuffers(1, &EBO);

//         glBindVertexArray(VAO);
//         // load data into vertex buffers
//         glBindBuffer(GL_ARRAY_BUFFER, VBO);
//         // A great thing about structs is that their memory layout is sequential for all its items.
//         // The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
//         // again translates to 3/2 floats which translates to a byte array.
//         glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);  

//         glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
//         glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

//         // set the vertex attribute pointers
//         // vertex Positions
//         glEnableVertexAttribArray(0);	
//         glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
//         // vertex normals
//         glEnableVertexAttribArray(1);	
//         glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
//         // vertex texture coords
//         glEnableVertexAttribArray(2);	
//         glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
//         // vertex tangent
//         glEnableVertexAttribArray(3);
//         glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
//         // vertex bitangent
//         glEnableVertexAttribArray(4);
//         glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));
// 		// Bone ids
// 		glEnableVertexAttribArray(5);
// 		glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, m_BoneIDs));

// 		// weights
// 		glEnableVertexAttribArray(6);
// 		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_Weights));

//         // vertex material color
//         glEnableVertexAttribArray(7);
//         glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));

//         // vertex use texture flag
//         glEnableVertexAttribArray(8);
//         glVertexAttribPointer(8, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, useDiffuseTexture));
        
//         glBindVertexArray(0);
//     }
// };
// #endif