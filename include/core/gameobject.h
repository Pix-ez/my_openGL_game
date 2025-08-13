#pragma once

#include "model.h"
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
        : model(model), name(std::move(name)) {}

    virtual ~GameObject() = default;

    // --- HIERARCHY METHODS ---
    void AddChild(std::shared_ptr<GameObject> child);
    glm::mat4 GetWorldTransformMatrix() const;

    // --- GAME LOOP METHODS ---
    // These are now recursive
    virtual void Update(float deltaTime);
    void Draw(Shader& shader, const glm::mat4& view, const glm::mat4& projection);
    // void Draw(Shader& shader);

    // --- UI METHOD (for Part 2) ---
    virtual void OnImGui();

    // Public access to children for UI iteration
    const std::vector<std::shared_ptr<GameObject>>& GetChildren() const { return m_children; }

private:
    // Hierarchy relationships
    std::weak_ptr<GameObject> m_parent;
    std::vector<std::shared_ptr<GameObject>> m_children;

    glm::mat4 GetLocalTransformMatrix() const;
};