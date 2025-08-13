#include "gameobject.h"

// --- HIERARCHY ---

void GameObject::AddChild(std::shared_ptr<GameObject> child) {
    if (child) {
        child->m_parent = shared_from_this(); // Use shared_from_this() to get a weak_ptr
        m_children.push_back(child);
    }
}

glm::mat4 GameObject::GetLocalTransformMatrix() const {
    // Same as the old GetTransformMatrix()
    glm::mat4 transform = glm::mat4(1.0f);
    transform = glm::translate(transform, position);
    transform = glm::rotate(transform, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    transform = glm::rotate(transform, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    transform = glm::rotate(transform, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    transform = glm::scale(transform, scale);
    return transform;
}

glm::mat4 GameObject::GetWorldTransformMatrix() const {
    // Start with this object's local transform
    glm::mat4 worldTransform = GetLocalTransformMatrix();

    // If this object has a parent, multiply by the parent's world transform
    if (auto parent_ptr = m_parent.lock()) { // .lock() safely converts weak_ptr to shared_ptr
        worldTransform = parent_ptr->GetWorldTransformMatrix() * worldTransform;
    }
    
    return worldTransform;
}


// --- GAME LOOP (now recursive) ---

void GameObject::Update(float deltaTime) {
    // Base logic (can be overridden)

    // Update all children recursively
    for (const auto& child : m_children) {
        child->Update(deltaTime);
    }
}

void GameObject::Draw(Shader& shader, const glm::mat4& view, const glm::mat4& projection) {
    if (!model) {
        return;
    }
    glm::mat4 worldMatrix = GetWorldTransformMatrix();
    shader.setMat4("model", worldMatrix);
    // printf("model draw called");
    model->Draw(shader);

    // Draw all children recursively
    for (const auto& child : m_children) {
        child->Draw(shader, view, projection);
    }
}


// void GameObject::Draw(Shader& shader){
//     if (!model) {
//         return;
//     }
//     glm::mat4 worldMatrix = GetWorldTransformMatrix();
//     shader.setMat4("model", worldMatrix);
//     // printf("model draw called");
//     model->Draw(shader);

//     // Draw all children recursively
//     for (const auto& child : m_children) {
//         child->Draw(shader);
//     }
// }


// --- UI (for Part 2) ---
#include "imgui.h" // Include ImGui here or in a precompiled header

void GameObject::OnImGui() {
    ImGui::Text("Transform");
    ImGui::DragFloat3("Position", &position.x, 0.1f);
    ImGui::DragFloat3("Rotation", &rotation.x, 1.0f);
    ImGui::DragFloat3("Scale", &scale.x, 0.05f);
}