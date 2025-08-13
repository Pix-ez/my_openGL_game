// #ifndef MODEL_H
// #define MODEL_H

// #include <glad/glad.h> 

// #include <glm/glm.hpp>
// #include <glm/gtc/matrix_transform.hpp>
// #include <stb_image.h>
// #include <assimp/Importer.hpp>
// #include <assimp/scene.h>
// #include <assimp/postprocess.h>

// #include <core/mesh.h>
// #include "shader.h"

// #include <string>
// #include <fstream>
// #include <sstream>
// #include <iostream>
// #include <map>
// #include <vector>
// using namespace std;

// unsigned int TextureFromFile(const char *path, const string &directory, bool gamma = false);

// class Model 
// {
// public:
//     // model data 
//     vector<Texture> textures_loaded;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
//     vector<Mesh>    meshes;
//     string directory;
//     bool gammaCorrection;

//     // constructor, expects a filepath to a 3D model.
//     Model(string const &path, bool gamma = false) : gammaCorrection(gamma)
//     {
//         loadModel(path);
//     }

//     // draws the model, and thus all its meshes
//     void Draw(Shader &shader)
//     {
//         for(unsigned int i = 0; i < meshes.size(); i++)
//             meshes[i].Draw(shader);
//     }
    
// private:
//     // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
//     void loadModel(string const &path)
//     {
//         // read file via ASSIMP
//         Assimp::Importer importer;
//         const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
//         // check for errors
//         if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
//         {
//             cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
//             return;
//         }
//         // retrieve the directory path of the filepath
//         directory = path.substr(0, path.find_last_of('/'));

//         // process ASSIMP's root node recursively
//         processNode(scene->mRootNode, scene);
//     }

//     // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
//     void processNode(aiNode *node, const aiScene *scene)
//     {
//         // process each mesh located at the current node
//         for(unsigned int i = 0; i < node->mNumMeshes; i++)
//         {
//             // the node object only contains indices to index the actual objects in the scene. 
//             // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
//             aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
//             meshes.push_back(processMesh(mesh, scene));
//         }
//         // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
//         for(unsigned int i = 0; i < node->mNumChildren; i++)
//         {
//             processNode(node->mChildren[i], scene);
//         }

//     }

//     Mesh processMesh(aiMesh *mesh, const aiScene *scene)
//     {
//         // data to fill
//         vector<Vertex> vertices;
//         vector<unsigned int> indices;
//         vector<Texture> textures;

//         // walk through each of the mesh's vertices
//         for(unsigned int i = 0; i < mesh->mNumVertices; i++)
//         {
//             Vertex vertex;
//             glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
//             // positions
//             vector.x = mesh->mVertices[i].x;
//             vector.y = mesh->mVertices[i].y;
//             vector.z = mesh->mVertices[i].z;
//             vertex.Position = vector;
//             // normals
//             if (mesh->HasNormals())
//             {
//                 vector.x = mesh->mNormals[i].x;
//                 vector.y = mesh->mNormals[i].y;
//                 vector.z = mesh->mNormals[i].z;
//                 vertex.Normal = vector;
//             }
//             // texture coordinates
//             if(mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
//             {
//                 glm::vec2 vec;
//                 // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
//                 // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
//                 vec.x = mesh->mTextureCoords[0][i].x; 
//                 vec.y = mesh->mTextureCoords[0][i].y;
//                 vertex.TexCoords = vec;
//                 // tangent
//                 vector.x = mesh->mTangents[i].x;
//                 vector.y = mesh->mTangents[i].y;
//                 vector.z = mesh->mTangents[i].z;
//                 vertex.Tangent = vector;
//                 // bitangent
//                 vector.x = mesh->mBitangents[i].x;
//                 vector.y = mesh->mBitangents[i].y;
//                 vector.z = mesh->mBitangents[i].z;
//                 vertex.Bitangent = vector;
//             }
//             else
//                 vertex.TexCoords = glm::vec2(0.0f, 0.0f);

//             //check if there is no material texture we  use color
//             if (scene->mNumMaterials > mesh->mMaterialIndex)
//             {
//                 const auto& mat = scene->mMaterials[mesh->mMaterialIndex];
//                 aiColor4D diffuse;
//                 if (AI_SUCCESS == aiGetMaterialColor(mat, AI_MATKEY_COLOR_DIFFUSE, &diffuse))
//                     {
//                     vertex.color = glm::vec4(diffuse.r, diffuse.g, diffuse.b, diffuse.a);
//                     }
//                 if (mat->GetTextureCount(aiTextureType_DIFFUSE) > 0)
//                     {
//                     vertex.useDiffuseTexture = 1.f;
//                     }
//                 else
//                     {
//                     vertex.useDiffuseTexture = 0.f;
//                     }
//             }
//             //push vertex with its data to vector
//             vertices.push_back(vertex);
//         }
//         // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
//         for(unsigned int i = 0; i < mesh->mNumFaces; i++)
//         {
//             aiFace face = mesh->mFaces[i];
//             // retrieve all indices of the face and store them in the indices vector
//             for(unsigned int j = 0; j < face.mNumIndices; j++)
//                 indices.push_back(face.mIndices[j]);        
//         }
//         // process materials
//         aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];    
//         // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
//         // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
//         // Same applies to other texture as the following list summarizes:
//         // diffuse: texture_diffuseN
//         // specular: texture_specularN
//         // normal: texture_normalN

//         // 1. diffuse maps
//         vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
//         textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
//         // 2. specular maps
//         vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
//         textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
//         // 3. normal maps
//         std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT , "texture_normal");
//         textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
//         // 4. height maps
//         std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT , "texture_height");
//         textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
        
//         // 5. roughness maps
//         std::vector<Texture> roughnessMaps = loadMaterialTextures(material, aiTextureType_SHININESS , "texture_shininess");
//         textures.insert(textures.end(), roughnessMaps.begin(), roughnessMaps.end());
        
//         //testing texture
//         std::vector<Texture> normals = loadMaterialTextures(material, aiTextureType_NORMALS , "texture_tnormals");
//         textures.insert(textures.end(), normals.begin(), normals.end());
        
//         // return a mesh object created from the extracted mesh data
//         return Mesh(vertices, indices, textures);
//     }

//     // checks all material textures of a given type and loads the textures if they're not loaded yet.
//     // the required info is returned as a Texture struct.
//     vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName)
//     {
//         vector<Texture> textures;
//         for(unsigned int i = 0; i < mat->GetTextureCount(type); i++)
//         {
//             aiString str;
//             mat->GetTexture(type, i, &str);
//             // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
//             bool skip = false;
//             for(unsigned int j = 0; j < textures_loaded.size(); j++)
//             {
//                 if(std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
//                 {
//                     textures.push_back(textures_loaded[j]);
//                     skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
//                     break;
//                 }
//             }
//             if(!skip)
//             {   // if texture hasn't been loaded already, load it
//                 Texture texture;
//                 texture.id = TextureFromFile(str.C_Str(), this->directory);
//                 texture.type = typeName;
//                 texture.path = str.C_Str();
//                 textures.push_back(texture);
//                 textures_loaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecessary load duplicate textures.
//             }
//         }
//         return textures;
//     }

//     Material loadMaterial(aiMaterial* mat) {
//         Material material;
//         aiColor3D color(0.f, 0.f, 0.f);
//         float shininess;

//         mat->Get(AI_MATKEY_COLOR_DIFFUSE, color);
//         material.Diffuse = glm::vec3(color.r, color.b, color.g);

//         mat->Get(AI_MATKEY_COLOR_AMBIENT, color);
//         material.Ambient = glm::vec3(color.r, color.b, color.g);

//         mat->Get(AI_MATKEY_COLOR_SPECULAR, color);
//         material.Specular = glm::vec3(color.r, color.b, color.g);

//         mat->Get(AI_MATKEY_SHININESS, shininess);
//         material.Shininess = shininess;

//         return material;
//     }
// };


// unsigned int TextureFromFile(const char *path, const string &directory, bool gamma)
// {
//     stbi_set_flip_vertically_on_load(true);
//     string filename = string(path);
//     filename = directory + '/' + filename;

//     unsigned int textureID;
//     glGenTextures(1, &textureID);

//     int width, height, nrComponents;
//     unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
//     if (data)
//     {
//         GLenum format;
//         if (nrComponents == 1)
//             format = GL_RED;
//         else if (nrComponents == 3)
//             format = GL_RGB;
//         else if (nrComponents == 4)
//             format = GL_RGBA;

//         glBindTexture(GL_TEXTURE_2D, textureID);
//         glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
//         glGenerateMipmap(GL_TEXTURE_2D);

//         if(nrComponents ==4){
//             glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//             glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//         }else{
//             glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//             glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//         }

//         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
//         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

//         stbi_image_free(data);
//     }
//     else
//     {
//         std::cout << "Texture failed to load at path: " << path << std::endl;
//         stbi_image_free(data);
//     }

//     return textureID;
// }
// #endif




#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "stb_image.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "shader.h"
#include "texture.h"
#include "mesh.h"
#include "material.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>


class Model {
public:
    Model(const std::string& path, std::shared_ptr<Shader> defaultShader, bool gamma = false)
        : m_defaultShader(defaultShader), gammaCorrection(gamma) {
        loadModel(path);
    }

    // void Draw() {
    //     for (Mesh& mesh : meshes)
    //         mesh.draw();
    // }
    // The Draw function passes the shader to each mesh.
    void Draw(Shader& shader) {
        for (Mesh& mesh : m_meshes) {
            mesh.Draw(shader);
        }
    }
private:
    // std::vector<Texture*> textures_loaded;
    // std::vector<Mesh> m_meshes;
    // std::string directory;
    // bool gammaCorrection;
    // Shader* defaultShader;
     // The model owns its meshes.
    std::vector<Mesh> m_meshes;
    // The texture cache now uses the texture path as a key and a shared_ptr for automatic memory management.
    std::map<std::string, std::shared_ptr<Texture>> m_texturesLoaded;
    // The default shader to use if a material doesn't specify one.
    std::shared_ptr<Shader> m_defaultShader;
    std::string m_directory;
    bool gammaCorrection;
    
    void loadModel(const std::string& path) {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
            return;
        }
        m_directory = path.substr(0, path.find_last_of('/'));
        processNode(scene->mRootNode, scene);
    }

    void processNode(aiNode* node, const aiScene* scene) {
        for (unsigned int i = 0; i < node->mNumMeshes; i++) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            m_meshes.push_back(processMesh(mesh, scene));
        }
        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            processNode(node->mChildren[i], scene);
        }
    }

    Mesh processMesh(aiMesh* mesh, const aiScene* scene) {
        

        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;

       
        // for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        //     Vertex vertex;
        //     vertex.Position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        //     if (mesh->HasNormals()) {
        //         vertex.Normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
        //     }
        //     if (mesh->mTextureCoords[0]) {
        //         vertex.TexCoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
        //     } else {
        //         vertex.TexCoords = glm::vec2(0.0f, 0.0f);
        //     }
        //     vertices.push_back(vertex);
        // }
        // Walk through each of the mesh's vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex; // Create a temporary vertex to fill

            // --- 1. Process vertex positions, normals, and texture coordinates ---
            glm::vec3 vector; 
            // Positions
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;

            // Normals
            if (mesh->HasNormals())
            {
                vector.x = mesh->mNormals[i].x;
                vector.y = mesh->mNormals[i].y;
                vector.z = mesh->mNormals[i].z;
                vertex.Normal = vector;
            }

            // Texture Coordinates
            if (mesh->mTextureCoords[0]) // Does the mesh contain texture coordinates?
            {
                glm::vec2 vec;
                vec.x = mesh->mTextureCoords[0][i].x; 
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;
            }
            else
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);


            // --- 2. THIS IS WHERE YOU ADD YOUR CODE ---
            //    (It needs to be inside this loop)
            // Tangent and Bitangent
            if (mesh->HasTangentsAndBitangents())
            {
                // Tangent
                vector.x = mesh->mTangents[i].x;
                vector.y = mesh->mTangents[i].y;
                vector.z = mesh->mTangents[i].z;
                vertex.Tangent = vector;

                // Bitangent
                vector.x = mesh->mBitangents[i].x;
                vector.y = mesh->mBitangents[i].y;
                vector.z = mesh->mBitangents[i].z;
                vertex.Bitangent = vector;
            }
            // ---------------------------------------------

            // You would also load bone data here if you have it
            // ...

            vertices.push_back(vertex); // Add the completed vertex to the list
        }

        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }


        aiMaterial* assimpMaterial = scene->mMaterials[mesh->mMaterialIndex];
        std::shared_ptr<Material> meshMaterial = loadMaterial(assimpMaterial);

        return Mesh(vertices, indices, meshMaterial);
    }

    // This creates our own Material object from Assimp's material
    std::shared_ptr<Material> loadMaterial(aiMaterial* mat) {
        // Create a new Material with our default shader
        auto material = std::make_shared<Material>(m_defaultShader);

        // We use a helper lambda to load textures and add them to our material.
        auto addTextureToMaterial = [&](aiTextureType type, Texture::TextureType texType) {
            for (unsigned int i = 0; i < mat->GetTextureCount(type); ++i) {
                auto texture = loadMaterialTexture(mat, type, i, texType);
                if (texture) {
                    material->addTexture(texture);
                }
            }
        };

        // Load all texture types
        addTextureToMaterial(aiTextureType_DIFFUSE, Texture::TextureType::Diffuse);
        addTextureToMaterial(aiTextureType_SPECULAR, Texture::TextureType::Specular);
        addTextureToMaterial(aiTextureType_HEIGHT, Texture::TextureType::Normal); // Assimp calls normal maps "height maps" sometimes
        addTextureToMaterial(aiTextureType_NORMALS, Texture::TextureType::Normal);

        // You can also load material colors here if you want
        // aiColor3D color (0.f,0.f,0.f);
        // mat->Get(AI_MATKEY_COLOR_DIFFUSE, color);
        // material->color = glm::vec3(color.r, color.g, color.b);

        return material;
    }

    std::shared_ptr<Texture> loadMaterialTexture(aiMaterial* mat, aiTextureType type, int texIndex, Texture::TextureType texType) {
        aiString str;
        mat->GetTexture(type, texIndex, &str);
        std::string path = str.C_Str();

        // 1. Check if the texture is already in our cache
        if (m_texturesLoaded.count(path)) {
            return m_texturesLoaded[path];
        }

        // 2. If not, load it
        std::string fullPath = m_directory + '/' + path;
        auto texture = std::make_shared<Texture>(fullPath, texType);

        // 3. Add it to the cache and return it
        m_texturesLoaded[path] = texture;
        return texture;
    }


    // std::vector<Texture*> loadMaterialTextures(aiMaterial* mat, aiTextureType type, Texture::TextureType texType) {
    //     std::vector<Texture*> textures;
    //     for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
    //         aiString str;
    //         mat->GetTexture(type, i, &str);
    //         std::string filename = directory + "/" + str.C_Str();

    //         bool skip = false;
    //         for (Texture* t : textures_loaded) {
    //             if (t->getPath() == filename) {
    //                 textures.push_back(t);
    //                 skip = true;
    //                 break;
    //             }
    //         }
    //         if (!skip) {
    //             Texture* texture = new Texture(filename, texType);
    //             textures_loaded.push_back(texture);
    //             textures.push_back(texture);
    //         }
    //     }
    //     return textures;
    // }

    // Material loadMaterial(aiMaterial* mat) {
    //     Material material;
    //     material.shader = defaultShader;

    //     auto addTex = [&](aiTextureType assimpType, Texture::TextureType myType, const std::string& name) {
    //         std::vector<Texture*> texs = loadMaterialTextures(mat, assimpType, myType);
    //         if (!texs.empty())
    //             material.textures[name] = texs[0];
    //     };

    //     addTex(aiTextureType_DIFFUSE, Texture::Diffuse, "material.diffuse");
    //     addTex(aiTextureType_SPECULAR, Texture::Specular, "material.specular");
    //     addTex(aiTextureType_NORMALS, Texture::Normal, "material.normal");

    //     return material;
    // }
};

#endif
