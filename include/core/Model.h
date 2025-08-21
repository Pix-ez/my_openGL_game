
#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "stb_image.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Shader.h"
#include "Texture.h"
#include "Mesh.h"
#include "Material.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include <tuple> 

class Model {
public:
    
    //simple empty constuctor for loading from our mesh file and adding all mesh 
    Model() = default;
    //for loading from 3d model filepath
    Model(const std::string& path, bool gamma = false);
    //for adding in-engine primitive models
    Model(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);

   
   

    //defualt draw for lit
    void DrawWithMaterial(Shader& shader, int& textureUnit);
    // for rendering only geometry for special pass (shadow, depth, etc)
    void DrawGeometryOnly();

    std::vector<Mesh>& GetMeshes() { return m_meshes; }

    void setAssetPath(const std::string& path) { m_assetPath = path; }
    const std::string& getAssetPath() const { return m_assetPath; }
   


    
private:
   
     // The model owns its meshes.
    std::vector<Mesh> m_meshes;
    std::string m_assetPath; //path of model in folders
    // The texture cache now uses the texture path as a key and a shared_ptr for automatic memory management.
    std::map<std::string, std::shared_ptr<Texture>> m_texturesLoaded;
    // The default shader to use if a material doesn't specify one.
    // std::shared_ptr<Shader> m_defaultShader;
    std::string m_directory;
    bool gammaCorrection;
    
    // void addMesh(const Mesh&& mesh);
    
    void loadModel(const std::string& path);

    void processNode(aiNode* node, const aiScene* scene);

    // Mesh processMesh(aiMesh* mesh, const aiScene* scene);
    std::tuple<std::vector<Vertex>, std::vector<unsigned int>, std::shared_ptr<Material>>
    processMesh(aiMesh* mesh, const aiScene* scene);

    // This creates our own Material object from Assimp's material
    std::shared_ptr<Material> loadMaterial(aiMaterial* mat);

    std::shared_ptr<Texture> loadMaterialTexture(aiMaterial* mat, aiTextureType type, int texIndex, Texture::TextureType texType);

};

#endif
