// ResourceManager.cpp
#include "ResourceManager.h"
#include "Model.h"
#include "Texture.h"
#include "Primitives.h" // For generating primitives
#include "scene_generated.h" // For Vec3 struct

#include <iostream>
#include <fstream>

// Re-use the binary format definition
namespace MeshFile {
    struct Header { char magic[4]; uint32_t version; uint32_t meshCount; };
    struct MeshChunk {
        uint32_t vertexCount; uint32_t indexCount;
        MyEngine::Scene::Vec3 baseColor; float shininess;
        uint32_t diffusePathLength; uint32_t specularPathLength; uint32_t normalPathLength;
    };
}


ResourceManager::ResourceManager(const std::filesystem::path& projectRootPath)
    : m_projectRootPath(projectRootPath) {}

std::shared_ptr<Model> ResourceManager::LoadModel(const std::string& assetPath) {
    // Check cache first
    if (m_modelCache.count(assetPath)) {
        return m_modelCache[assetPath];
    }

    // Check for primitives
    if (assetPath.rfind("primitive::", 0) == 0) {
        std::string type = assetPath.substr(11);
        auto primitiveModel = GeneratePrimitive(type);
        m_modelCache[assetPath] = primitiveModel;
        return primitiveModel;
    }
    
    // Load from file
    auto fileModel = LoadModelFromFile(assetPath);
    if (fileModel) {
        m_modelCache[assetPath] = fileModel;
    }
    return fileModel;
}


std::shared_ptr<Model> ResourceManager::LoadModelFromFile(const std::string& assetPath) {
    auto fullPath = m_projectRootPath / assetPath;
    
    std::ifstream infile(fullPath, std::ios::binary);
    if (!infile) {
        std::cerr << "Error: Could not open mesh file: " << fullPath << std::endl;
        return nullptr;
    }

    MeshFile::Header fileHeader;
    infile.read(reinterpret_cast<char*>(&fileHeader), sizeof(fileHeader));
    
    if (std::string(fileHeader.magic, 4) != "MESH") {
        std::cerr << "Error: Invalid mesh file format." << std::endl;
        return nullptr;
    }
    if (fileHeader.meshCount == 0) {
        std::cerr << "Warning: Mesh file has no meshes: " << fullPath << std::endl;
        return nullptr;
    }

    // --- Create a new EMPTY model ---
    // We will manually add meshes to it.
    auto model = std::make_shared<Model>();
    model->setAssetPath(assetPath); // IMPORTANT: Store the path for serialization

    auto readString = [&](uint32_t len) {
        if (len == 0) return std::string();
        std::string s(len, '\0');
        infile.read(&s[0], len);
        return s;
    };

    // --- Loop for each mesh chunk in the file ---
    for (uint32_t i = 0; i < fileHeader.meshCount; ++i) {
        // 1. Read the chunk header for this mesh
        MeshFile::MeshChunk chunkHeader;
        infile.read(reinterpret_cast<char*>(&chunkHeader), sizeof(chunkHeader));

        // 2. Read the material texture paths
        std::string diffusePath = readString(chunkHeader.diffusePathLength);
        std::string specularPath = readString(chunkHeader.specularPathLength);
        std::string normalPath = readString(chunkHeader.normalPathLength);

        // 3. Read the vertex and index data for this mesh
        std::vector<Vertex> vertices(chunkHeader.vertexCount);
        infile.read(reinterpret_cast<char*>(vertices.data()), vertices.size() * sizeof(Vertex));

        std::vector<unsigned int> indices(chunkHeader.indexCount);
        infile.read(reinterpret_cast<char*>(indices.data()), indices.size() * sizeof(unsigned int));
        
        // 4. Create the material for this mesh
        auto material = std::make_shared<Material>();
        material->BaseColor = {chunkHeader.baseColor.x(), chunkHeader.baseColor.y(), chunkHeader.baseColor.z()};
        material->shininess = chunkHeader.shininess;
        
        if (!diffusePath.empty()) material->addTexture(LoadTexture(diffusePath, Texture::TextureType::Diffuse));
        if (!specularPath.empty()) material->addTexture(LoadTexture(specularPath, Texture::TextureType::Specular));
        if (!normalPath.empty()) material->addTexture(LoadTexture(normalPath, Texture::TextureType::Normal));

        // 5. Add the new mesh to the model's list of meshes
        // This is the key part that relies on the "Rule of Five" fixes we did earlier.
        model->GetMeshes().emplace_back(std::move(vertices), std::move(indices), material);
    }

    return model;
}

std::shared_ptr<Model> ResourceManager::GeneratePrimitive(const std::string& primitiveType) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    if (primitiveType == "cube") {
        Primitives::GenerateCube(vertices, indices);
    } else if (primitiveType == "plane") {
        Primitives::GeneratePlane(vertices, indices);
    } // ... else other primitives
    
    auto model = std::make_shared<Model>(vertices, indices);
    model->setAssetPath("primitive::" + primitiveType); // Store its type as its "path"
    return model;
}


std::shared_ptr<Texture> ResourceManager::LoadTexture(const std::string& assetPath, Texture::TextureType type) {
    if (m_textureCache.count(assetPath)) {
        return m_textureCache[assetPath];
    }
    
    auto fullPath = m_projectRootPath / "assets" / assetPath;
    
    // Your existing texture loading logic goes here
    // For example: auto texture = std::make_shared<Texture>(fullPath.string(), type);
    // You will need to determine the texture 'type' (Diffuse, Specular) from the context.
    // A simple way is to pass it in or infer from the filename.
    
    // Placeholder:
    auto texture = std::make_shared<Texture>(fullPath.string(), type);
    
    if (texture->isLoaded()) { // Assuming your Texture class has a way to check this
        m_textureCache[assetPath] = texture;
        return texture;
    }
    
    return nullptr;
}