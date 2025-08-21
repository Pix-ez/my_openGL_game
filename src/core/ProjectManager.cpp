// ProjectManager.cpp
#include "ProjectManager.h"
#include "SceneSerializer.h" // Include the (currently empty) headers
#include "AssetProcessor.h"
#include "project_generated.h" // Our generated FlatBuffers header
#include "ResourceManager.h"

#include <iostream>
#include <fstream>
#include <vector>

ProjectManager::ProjectManager() {
    // Initialize the delegates
    m_sceneSerializer = std::make_unique<SceneSerializer>(nullptr);
    m_assetProcessor = std::make_unique<AssetProcessor>();
}

ProjectManager::~ProjectManager() {
    // Destructor is needed for unique_ptr with forward-declared types
}

bool ProjectManager::CreateNewProject(const std::filesystem::path& projectPath) {
    if (std::filesystem::exists(projectPath) && !std::filesystem::is_empty(projectPath)) {
        std::cerr << "Error: Project path already exists and is not empty." << std::endl;
        return false;
    }
    
    CloseProject();

    m_projectRootPath = projectPath;
    m_projectName = projectPath.stem().string();
    m_projectFilePath = m_projectRootPath / (m_projectName + ".myproj");

    if (!CreateProjectStructure()) {
        return false;
    }
    m_resourceManager = std::make_unique<ResourceManager>(m_projectRootPath);
    m_sceneSerializer = std::make_unique<SceneSerializer>(m_resourceManager.get());

    // --- Create and save the .myproj file using FlatBuffers ---
    flatbuffers::FlatBufferBuilder builder;

    auto nameOffset = builder.CreateString(m_projectName);
    auto versionOffset = builder.CreateString("1.0.0");
    auto sceneFileOffset = builder.CreateString("scenes/main.bin");

    auto projectData = MyEngine::Project::CreateProject(builder, nameOffset, versionOffset, sceneFileOffset);
    builder.Finish(projectData);

    std::ofstream outfile(m_projectFilePath, std::ios::binary);
    if (!outfile) {
        std::cerr << "Error: Could not create project file." << std::endl;
        return false;
    }
    outfile.write(reinterpret_cast<const char*>(builder.GetBufferPointer()), builder.GetSize());
    outfile.close();

    m_isProjectLoaded = true;
    std::cout << "Successfully created project '" << m_projectName << "' at " << m_projectRootPath << std::endl;
    return true;
}

bool ProjectManager::LoadProject(const std::filesystem::path& projectFilePath) {
    if (!std::filesystem::exists(projectFilePath)) {
        std::cerr << "Error: Project file not found." << std::endl;
        return false;
    }
    
    CloseProject();

    // Read the file into a buffer
    std::ifstream infile(projectFilePath, std::ios::binary | std::ios::ate);
    auto size = infile.tellg();
    infile.seekg(0, std::ios::beg);
    std::vector<char> buffer(size);
    if (!infile.read(buffer.data(), size)) {
        std::cerr << "Error: Could not read project file." << std::endl;
        return false;
    }

    // Verify and parse the FlatBuffer
    auto verifier = flatbuffers::Verifier(reinterpret_cast<uint8_t*>(buffer.data()), buffer.size());
    if (!MyEngine::Project::VerifyProjectBuffer(verifier)) {
        std::cerr << "Error: Invalid project file format." << std::endl;
        return false;
    }

    auto projectData = MyEngine::Project::GetProject(buffer.data());

    m_projectFilePath = projectFilePath;
    m_projectRootPath = projectFilePath.parent_path();
    m_resourceManager = std::make_unique<ResourceManager>(m_projectRootPath);
    m_sceneSerializer = std::make_unique<SceneSerializer>(m_resourceManager.get());
    
    m_isProjectLoaded = true;
    m_projectName = projectData->name()->str();
    
    // Set up paths
    m_assetsPath = m_projectRootPath / "assets";
    m_cachePath = m_projectRootPath / "cache";
    m_scenesPath = m_projectRootPath / "scenes";

    m_isProjectLoaded = true;
    std::cout << "Successfully loaded project '" << m_projectName << "'" << std::endl;
    return true;
}

bool ProjectManager::SaveProject() {
    // For now, saving the project just means saving the current scene.
    // In the future, it could save other project-level settings.
    // NOTE: This assumes you have a way to get the current scene's GameObjects
    // from your main application, which we'll handle with SaveScene.
    std::cout << "Project saved (scene state)." << std::endl;
    return true;
}

void ProjectManager::CloseProject() {
    m_projectName.clear();
    m_projectRootPath.clear();
    m_projectFilePath.clear();
    m_assetsPath.clear();
    m_cachePath.clear();
    m_scenesPath.clear();
    m_isProjectLoaded = false;
}

bool ProjectManager::CreateProjectStructure() {
    try {
        std::filesystem::create_directories(m_projectRootPath);
        m_assetsPath = m_projectRootPath / "assets";
        m_cachePath = m_projectRootPath / "cache";
        m_scenesPath = m_projectRootPath / "scenes";
        std::filesystem::create_directories(m_assetsPath);
        std::filesystem::create_directories(m_cachePath);
        std::filesystem::create_directories(m_scenesPath);
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error creating project directories: " << e.what() << std::endl;
        return false;
    }
    return true;
}


// --- Delegation Functions ---
// These are just stubs for now. They call the (currently empty) serializer classes.
bool ProjectManager::SaveScene(const std::vector<std::shared_ptr<GameObject>>& sceneRoots) {
    if (!m_isProjectLoaded) return false;
    
    // We'll read the scene file path from the project data later. For now, hardcode it.
    auto scenePath = m_scenesPath / "main.bin";
    return m_sceneSerializer->SaveScene(scenePath, sceneRoots);
}

// In LoadScene, pass the now-valid resource manager to the serializer
bool ProjectManager::LoadScene(std::vector<std::shared_ptr<GameObject>>& sceneRoots) {
    if (!m_resourceManager) {
        std::cerr << "Error: SceneSerializer has no ResourceManager. Load a project first." << std::endl;
        return false;
    }
    if (!m_isProjectLoaded) return false;
    auto scenePath = m_scenesPath / "main.bin";
    return m_sceneSerializer->LoadScene(scenePath, sceneRoots);
}

std::string ProjectManager::ProcessAndImportAsset(const std::filesystem::path& sourceAssetPath) {
    if (!m_isProjectLoaded) return "";
    
    std::string filename = sourceAssetPath.stem().string();
    std::string relativeCachePath = "cache/" + filename + ".mesh";
    auto destinationPath = m_projectRootPath / relativeCachePath;
    
    if (m_assetProcessor->ProcessModel(sourceAssetPath, m_assetsPath, destinationPath)) {
        return relativeCachePath;
    }
    return "";
}