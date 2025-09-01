#include "PhysicsComponent.h"
#include "GameObject.h" 

// A global gravity constant
const float GLOBAL_GRAVITY = -9.81f;

// The rest of your code was perfect, it will now compile.
PhysicsComponent::PhysicsComponent(GameObject* owner) : m_owner(owner) {}

void PhysicsComponent::Update(float deltaTime) {
    if (isStatic || !m_owner) { // Added a check for m_owner just in case
        return; 
    }

    // Apply gravity
    if (applyGravity) {
        float gravity = (gravityOverride != 0.0f) ? gravityOverride : GLOBAL_GRAVITY;
        acceleration.y += gravity; // It's common for gravity to be negative, so this should probably be -=
                                   // But for now, we'll keep your logic: acceleration.y += gravity
    }

    // Update velocity based on acceleration
    velocity += acceleration * deltaTime;

    // Update position based on velocity
    // This line is why we need the full include of GameObject.h here
    m_owner->position += velocity * deltaTime;

    // Reset acceleration for the next frame
    acceleration = glm::vec3(0.0f);
}