// ProjectManager.h
#pragma once

#include <string>
#include <filesystem>
#include <vector>
#include <memory>

// Forward-declare the classes it will interact with
class GameObject;
class SceneSerializer; // We will build this next
class AssetProcessor;  // We will build this later
class ResourceManager; // Forward declare

class ProjectManager {
public:
    ProjectManager();
    ~ProjectManager();

    // --- Core Project Operations ---
    bool CreateNewProject(const std::filesystem::path& projectPath);
    bool LoadProject(const std::filesystem::path& projectFilePath);
    bool SaveProject(); // Saves the current project state
    void CloseProject();
    
    // --- Accessors ---
    bool IsProjectLoaded() const { return m_isProjectLoaded; }
    const std::filesystem::path& GetProjectPath() const { return m_projectRootPath; }
    const std::filesystem::path& GetAssetsPath() const { return m_assetsPath; }
    const std::filesystem::path& GetCachePath() const { return m_cachePath; }
    const std::string& GetProjectName() const { return m_projectName; }
    ResourceManager* GetResourceManager() { return m_resourceManager.get(); }
    
    // --- Scene & Asset Delegation ---
    // These functions will be called by the UI
    bool SaveScene(const std::vector<std::shared_ptr<GameObject>>& sceneRoots);
    bool LoadScene(std::vector<std::shared_ptr<GameObject>>& sceneRoots);
    // Returns the path to the cached asset, or an empty string on failure
    std::string ProcessAndImportAsset(const std::filesystem::path& sourceAssetPath);

private:
    bool CreateProjectStructure();
    std::unique_ptr<ResourceManager> m_resourceManager;
    
    std::string m_projectName;
    std::filesystem::path m_projectRootPath;
    std::filesystem::path m_projectFilePath;
    
    // Sub-directories
    std::filesystem::path m_assetsPath;
    std::filesystem::path m_cachePath;
    std::filesystem::path m_scenesPath;

    // Delegate objects
    std::unique_ptr<SceneSerializer> m_sceneSerializer;
    std::unique_ptr<AssetProcessor> m_assetProcessor;

    bool m_isProjectLoaded = false;
};