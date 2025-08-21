#pragma once

#include "Model.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <vector>
#include <string>

// GameObject now inherits from enable_shared_from_this
class GameObject : public std::enable_shared_from_this<GameObject> {
public:
    std::shared_ptr<Model> model;
    std::string name = "GameObject"; // Give objects names for the UI

    // Local transform (relative to the parent)
    glm::vec3 position{0.0f};
    glm::vec3 rotation{0.0f};
    glm::vec3 scale{1.0f};

    // Constructor now takes an optional name
    GameObject(std::shared_ptr<Model> model, std::string name = "GameObject") 
        : model(model), name(std::move(name)), m_id(0) {}

    virtual ~GameObject() = default;

    // --- HIERARCHY METHODS ---
    void AddChild(std::shared_ptr<GameObject> child);
    glm::mat4 GetWorldTransformMatrix() const;

    // --- GAME LOOP METHODS ---
    // These are now recursive
    virtual void Update(float deltaTime);
    // void Draw(Shader& shader, const glm::mat4& view, const glm::mat4& projection);
    // void Draw(Shader& shader, int& textureUnit);
    void DrawWithMaterial(Shader& shader, int& textureUnit);
    void DrawGeometryOnly(Shader& shader); // The shader is needed here for the model matrix

    // --- UI METHOD (for Part 2) ---
    virtual void OnImGui();

    // Public access to children for UI iteration
    const std::vector<std::shared_ptr<GameObject>>& GetChildren() const { return m_children; }

    //getter for id
    uint32_t GetID() const { return m_id; }
    void SetID(uint32_t id) {m_id = id;}

    std::weak_ptr<GameObject> GetParent() const { return m_parent; }

private:
    uint32_t m_id;
    // Hierarchy relationships
    std::weak_ptr<GameObject> m_parent;
    std::vector<std::shared_ptr<GameObject>> m_children;

    glm::mat4 GetLocalTransformMatrix() const;
};