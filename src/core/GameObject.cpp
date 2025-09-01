#include "GameObject.h"

// --- HIERARCHY ---
uint32_t GameObject::s_nextID = 1;

GameObject::GameObject(std::shared_ptr<Model> model, std::string name, bool enablePhysics, bool enableCollider) 
    : model(model), name(std::move(name)) // Removed m_id(0) from here
{
    // Assign a new, unique ID and then increment the static counter
    // for the next object.
    m_id = s_nextID++;
    if (enablePhysics) {
        physics = std::make_unique<PhysicsComponent>(this);
    }
   

    if (enableCollider && model) {
        collider = std::make_unique<ColliderComponent>(this, model);
        std::cout<<"colider added"<<std::endl;
    } else {
        // --- ADD THIS DEBUG MESSAGE ---
        if (model) { // Only print if a model was provided but collider was not created
            std::cout << "INFO: GameObject '" << name << "' created WITHOUT a collider component." << std::endl;
        }
        // --- END DEBUG MESSAGE ---
    }
}

void GameObject::Update(float deltaTime) {
    // Update the physics component if it exists
    if (physics ) {
        physics->Update(deltaTime);
    }

    if (collider) {
        collider->Update(); // <-- ADD THIS
    }

    // Recursively update children
    for (const auto& child : m_children) {
        child->Update(deltaTime);
    }
}

void GameObject::AddChild(std::shared_ptr<GameObject> child) {
    if (child) {
        child->m_parent = shared_from_this(); // Use shared_from_this() to get a weak_ptr
        m_children.push_back(child);
    }
}

void GameObject::RemoveChild(std::shared_ptr<GameObject> child) {
    if (child) {
        // Detach the child from this parent
        child->m_parent.reset();
        // Remove the child from our list of children
        m_children.erase(std::remove(m_children.begin(), m_children.end(), child), m_children.end());
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

// void GameObject::Update(float deltaTime) {
//     // Base logic (can be overridden)

//     // Update all children recursively
//     for (const auto& child : m_children) {
//         child->Update(deltaTime);
//     }
// }

// void GameObject::Draw(Shader& shader, const glm::mat4& view, const glm::mat4& projection) {
//     if (!model) {
//         return;
//     }
//     glm::mat4 worldMatrix = GetWorldTransformMatrix();
//     shader.setMat4("model", worldMatrix);
//     // printf("model draw called");
//     model->Draw(shader);

//     // Draw all children recursively
//     for (const auto& child : m_children) {
//         child->Draw(shader, view, projection);
//     }
// }


// void GameObject::Draw(Shader& shader, int& textureUnit){
//     if (!model) {
//         return;
//     }
//     glm::mat4 worldMatrix = GetWorldTransformMatrix();
//     shader.setMat4("model", worldMatrix);
//     // printf("model draw called");
//     model->Draw(shader, textureUnit);

//     // Draw all children recursively
//     for (const auto& child : m_children) {
//         child->Draw(shader,  textureUnit);
//     }
// }

void GameObject::DrawWithMaterial(Shader& shader, int& textureUnit) {
    if (!model) return;
    shader.setMat4("model", GetWorldTransformMatrix());
    model->DrawWithMaterial(shader, textureUnit);
    for (const auto& child : m_children) {
        child->DrawWithMaterial(shader, textureUnit);
    }
}
void GameObject::DrawGeometryOnly(Shader& shader) {
    if (!model) return;
    shader.setMat4("model", GetWorldTransformMatrix());
    model->DrawGeometryOnly();
    for (const auto& child : m_children) {
        child->DrawGeometryOnly(shader);
    }
}

// --- UI (for Part 2) ---
#include "imgui.h" // Include ImGui here or in a precompiled header
#include "Model.h" // Make sure to include model.h to access GetMeshes()
#include "Mesh.h"  // And mesh.h to access the material

// void GameObject::OnImGui() {
//     ImGui::Text("Transform");
//     ImGui::DragFloat3("Position", &position.x, 0.1f);
//     ImGui::DragFloat3("Rotation", &rotation.x, 1.0f);
//     ImGui::DragFloat3("Scale", &scale.x, 0.05f);
// }

void GameObject::OnImGui(bool& isDirty) {
    // --- Part 1: Keep the existing Transform editor ---
    ImGui::Text("Transform");
    if (ImGui::DragFloat3("Position", &position.x, 0.1f)) { isDirty = true; }
    if (ImGui::DragFloat3("Rotation", &rotation.x, 1.0f)) { isDirty = true; }
    if (ImGui::DragFloat3("Scale", &scale.x, 0.05f))    { isDirty = true; }
    
    ImGui::Separator();

    // --- Part 2: Add the Material editor ---
    // Check if this GameObject has a model to inspect
    if (model) {
        // Get the meshes from the model
        auto& meshes = model->GetMeshes();

        // Loop through each mesh (for primitives, there's only one)
        for (int i = 0; i < meshes.size(); ++i) {
            Mesh& mesh = meshes[i];

            // Check if the mesh has a material
            if (mesh.m_material) {
                // Use a TreeNode to keep the UI organized, especially for models with many meshes
                std::string label = "Mesh " + std::to_string(i) + " Material";
                if (ImGui::TreeNode(label.c_str())) {
                    
                    // --- This is the color editor! ---
                    // It directly modifies the diffuseColor in the Material object.
                    if(ImGui::ColorEdit3("Diffuse Color", &mesh.m_material->BaseColor.x)){ isDirty = true; }

                    // We can also edit other material properties here!
                   if( ImGui::DragFloat("Shininess", &mesh.m_material->shininess, 1.0f, 0.0f, 256.0f)){ isDirty = true; }

                    // You could add buttons to assign textures here in the future

                    ImGui::TreePop();
                }
            }
        }
    }
    
    if (ImGui::CollapsingHeader("Physics")) {
            if (physics) {
                ImGui::Checkbox("Is Static", &physics->isStatic);
                ImGui::Checkbox("Apply Gravity", &physics->applyGravity);
                ImGui::DragFloat("Gravity Override", &physics->gravityOverride, 0.1f);
                ImGui::InputFloat3("Velocity", &physics->velocity.x);
                ImGui::InputFloat("Mass", &physics->mass);
            } else {
                if (ImGui::Button("Add Physics Component")) {
                    physics = std::make_unique<PhysicsComponent>(this);
                }
            }
        }
}