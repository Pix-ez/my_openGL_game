// SceneSerializer.h
#pragma once

#include <filesystem>
#include <vector>
#include <memory>
#include <map>

#include "scene_generated.h"

// Forward declarations
class GameObject;
class ResourceManager; // You will need to create this class

class SceneSerializer {
public:
    // We pass the resource manager to the constructor so we can load models/primitives
    SceneSerializer(ResourceManager* resourceManager);

    bool SaveScene(const std::filesystem::path& filePath, const std::vector<std::shared_ptr<GameObject>>& sceneRoots);
    bool LoadScene(const std::filesystem::path& filePath, std::vector<std::shared_ptr<GameObject>>& sceneRoots);

private:
    // You need to add a simple ID system to your GameObject class
    // e.g., `uint32_t GetID() const;`
    // We will use a map to keep track of objects during serialization
    std::map<uint32_t, const MyEngine::Scene::GameObject*> m_fbGameObjectMap;

    // Helper function for recursive saving
    void SerializeObjectAndChildren(
        std::shared_ptr<GameObject> object,
        flatbuffers::FlatBufferBuilder& builder,
        std::vector<flatbuffers::Offset<MyEngine::Scene::GameObject>>& gameObjectOffsets
    );

    // Main deserialization function
    std::shared_ptr<GameObject> DeserializeGameObject(const MyEngine::Scene::GameObject* fbGameObject);

    ResourceManager* m_resourceManager;
};