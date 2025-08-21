// AssetProcessor.h
#pragma once

#include <filesystem>
#include <string>

class AssetProcessor {
public:
    // Takes a source model file (e.g., .obj) and converts it to our custom .mesh format.
    // It also finds, copies, and processes associated textures.
    bool ProcessModel(const std::filesystem::path& sourceModelPath, 
                      const std::filesystem::path& projectAssetsPath, 
                      const std::filesystem::path& destinationMeshPath);

    // In a real engine, this would convert textures to an optimal format like DDS.
    // For now, it just copies the texture to the project's assets folder if it's not already there.
    // Returns the new relative asset path.
    std::string ProcessTexture(const std::filesystem::path& sourceTexturePath, 
                               const std::filesystem::path& sourceModelDirectory,
                               const std::filesystem::path& projectAssetsPath);
};