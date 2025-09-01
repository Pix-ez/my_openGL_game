#include "DirectionalLight.h"
#include "imgui.h"

void DirectionalLight::UploadToShader(Shader& shader, int index) const {
    // Directional light uses a single struct name in the shader
    // shader.setVec3("dirLight.direction", GetDirection());
    // shader.setVec3("dirLight.ambient", ambient);
    // shader.setVec3("dirLight.diffuse", diffuse);
    // shader.setVec3("dirLight.specular", specular);
    glm::vec3 worldPosition = glm::vec3(GetWorldTransformMatrix()[3]);
    glm::vec3 worldDirection = GetDirection(); // Use the helper function!
    shader.setVec3("dirLight.direction", worldDirection);
    shader.setVec3("dirLight.ambient", ambient);
    shader.setVec3("dirLight.diffuse", diffuse);
    shader.setVec3("dirLight.specular", specular);
    shader.setFloat("dirLight.intensity", intensity);
}

glm::vec3 DirectionalLight::GetDirection() const {
    // Calculate direction from rotation (same as camera front vector)
    glm::vec3 front;
    front.x = cos(glm::radians(rotation.y)) * cos(glm::radians(rotation.x));
    front.y = sin(glm::radians(rotation.x));
    front.z = sin(glm::radians(rotation.y)) * cos(glm::radians(rotation.x));
    return glm::normalize(front);
}

void DirectionalLight::OnImGui(bool &isDirty)
{
    Light::OnImGui(isDirty);
    ImGui::Separator();


    ImGui::Text("Direction is controlled by Rotation.");
    if(ImGui::SliderFloat3("Rotation (deg)", &direction.x, 0.0f, 360.0f)) { isDirty = true; }
    
}
