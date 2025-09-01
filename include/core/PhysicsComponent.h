#pragma once 
#include <glm/glm.hpp>

class GameObject;

class PhysicsComponent{
public:
    //physic properties
    glm::vec3 velocity{0.0f};
    glm::vec3 acceleration{0.0f};
    float mass = 1.0f;
    // Control flags
    bool isStatic = false;       // If true, the object will not move
    bool applyGravity = true;    // If true, gravity will be applied
    float gravityOverride = 0.0f; // Use a custom gravity value for this object

    // Constructor
    PhysicsComponent(GameObject* owner);

    // Update physics simulation for a single time step
    void Update(float deltaTime);

private:
    GameObject* m_owner; // Pointer to the owning GameObject
};