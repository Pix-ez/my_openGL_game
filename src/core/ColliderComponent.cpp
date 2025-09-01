#include "ColliderComponent.h"
#include "GameObject.h" // We need the full definition here
#include "Model.h"
#include <iostream>


ColliderComponent::ColliderComponent(GameObject* owner, std::shared_ptr<Model> model)
    : m_owner(owner)
{
    
    if (model) {
        // In a real engine, you'd get this from the model data
        // e.g., localMin = model->GetMinBounds();
        localMin = model->GetMinBounds();
        localMax = model->GetMaxBounds();
    } else {
        localMin = glm::vec3(0.0f);
        localMax = glm::vec3(0.0f);
    }

    // --- DEBUG PRINT ---
    std::cout << "Collider created for '" << m_owner->name 
              << "': Min(" << localMin.x << ", " << localMin.y << ", " << localMin.z 
              << "), Max(" << localMax.x << ", " << localMax.y << ", " << localMax.z << ")" << std::endl;
    // --- END DEBUG ---

    Update(); // Initial update
}

void ColliderComponent::Update() {
    if (!m_owner) return;

    // Update world bounds based on the owner's position and scale
    // This simple version doesn't account for rotation.
    worldMin = m_owner->position + (localMin * m_owner->scale);
    worldMax = m_owner->position + (localMax * m_owner->scale);

}

// AABB-AABB intersection test
bool ColliderComponent::CheckCollision(const ColliderComponent& a, const ColliderComponent& b) {
    // Check for overlap on each axis
    bool overlapX = a.worldMax.x > b.worldMin.x && a.worldMin.x < b.worldMax.x;
    bool overlapY = a.worldMax.y > b.worldMin.y && a.worldMin.y < b.worldMax.y;
    bool overlapZ = a.worldMax.z > b.worldMin.z && a.worldMin.z < b.worldMax.z;

    // A collision occurs only if there is overlap on all three axes
    return overlapX && overlapY && overlapZ;
}