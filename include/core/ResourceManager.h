// ResourceManager.h
#pragma once
#include "Texture.h"
#include <string>
#include <filesystem>
#include <map>
#include <memory>

class Model;
class Texture;

class ResourceManager {
public:
    // Takes the root path of the project to know where the 'cache' and 'assets' folders are
    ResourceManager(const std::filesystem::path& projectRootPath);

    // The main loading function. Handles primitives and file-based models.
    std::shared_ptr<Model> LoadModel(const std::string& assetPath);
    
    // Loads textures
    std::shared_ptr<Texture> LoadTexture(const std::string& assetPath, Texture::TextureType type);

private:
    std::shared_ptr<Model> LoadModelFromFile(const std::string& assetPath);
    std::shared_ptr<Model> GeneratePrimitive(const std::string& primitiveType);

    std::filesystem::path m_projectRootPath;

    // Caches for loaded assets
    std::map<std::string, std::shared_ptr<Model>> m_modelCache;
    std::map<std::string, std::shared_ptr<Texture>> m_textureCache;
};