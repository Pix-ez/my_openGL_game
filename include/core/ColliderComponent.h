#pragma once

#include <glm/glm.hpp>
#include <memory>
#include "Model.h"

class GameObject; // Forward declaration

class ColliderComponent {
public:
    // The AABB's boundaries in LOCAL space (relative to the GameObject's origin)
    glm::vec3 localMin;
    glm::vec3 localMax;

    // The AABB's boundaries in WORLD space (calculated each frame)
    glm::vec3 worldMin;
    glm::vec3 worldMax;

    // Pointer to the owner for accessing its transform
    GameObject* m_owner;

    // Constructor
    ColliderComponent(GameObject* owner, std::shared_ptr<Model> model);

    // Updates the worldMin and worldMax based on the owner's transform
    void Update();

    // Static function to check for collision between two colliders
    static bool CheckCollision(const ColliderComponent& a, const ColliderComponent& b);
};