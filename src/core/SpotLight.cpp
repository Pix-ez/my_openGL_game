#include "SpotLight.h"
#include "imgui.h"

void SpotLight::UploadToShader(Shader& shader, int index) const {
    // Note: Your old code used a single "spotLight" struct. If you want to support
    // multiple spotlights later, you would change this to "spotLights[index]".
    // For now, we'll match your existing shader.
    std::string uniformName = "spotLights[" + std::to_string(index) + "]";

    // shader.setVec3(uniformName + ".position", position); // Position comes from GameObject
    // shader.setVec3(uniformName + ".direction", direction); // Direction from rotation
     glm::vec3 worldPosition = glm::vec3(GetWorldTransformMatrix()[3]);
    glm::vec3 worldDirection = GetDirection(); // Use the helper function!

    
    shader.setVec3(uniformName + ".position", worldPosition);
    shader.setVec3(uniformName + ".direction", worldDirection);
    shader.setVec3(uniformName + ".ambient", ambient);
    shader.setVec3(uniformName + ".diffuse", diffuse);
    shader.setVec3(uniformName + ".specular", specular);
    
    shader.setFloat(uniformName + ".constant", constant);
    shader.setFloat(uniformName + ".linear", linear);
    shader.setFloat(uniformName + ".quadratic", quadratic);
    
    // Convert degrees to cosines before uploading
    shader.setFloat(uniformName + ".cutOff", glm::cos(glm::radians(cutOff)));
    shader.setFloat(uniformName + ".outerCutOff", glm::cos(glm::radians(outerCutOff)));
    shader.setFloat(uniformName + ".intensity", intensity);
}

glm::vec3 SpotLight::GetDirection() const {
    // This is the same logic as the DirectionalLight, it calculates a "front"
    // vector based on the GameObject's rotation.
    glm::vec3 front;
    front.x = cos(glm::radians(rotation.y)) * cos(glm::radians(rotation.x));
    front.y = sin(glm::radians(rotation.x));
    front.z = sin(glm::radians(rotation.y)) * cos(glm::radians(rotation.x));
    return glm::normalize(front);
}

void SpotLight::OnImGui(bool &isDirty)
{

    Light::OnImGui(isDirty);
    ImGui::Separator();
    ImGui::Text("SpotLight Properties");
    if(ImGui::SliderFloat3("point-towards (deg)", &direction.x, 0.0f, 360.0f)){ isDirty = true; }
    if(ImGui::SliderFloat("Cut Off", &cutOff, 0.0f, 90.0f, "%.1f deg")){ isDirty = true; }
    if(ImGui::SliderFloat("Outer Cut Off", &outerCutOff, 0.0f, 90.0f, "%.1f deg")){ isDirty = true; }
    if(ImGui::SliderFloat("Constant", &constant, 0.0f, 2.0f, "%.3f")){ isDirty = true; }
    if(ImGui::SliderFloat("Linear", &linear, 0.0f, 1.0f, "%.4f")){ isDirty = true; }
    if(ImGui::SliderFloat("Quadratic", &quadratic, 0.0f, 1.0f, "%.5f")){ isDirty = true; }
    if(ImGui::ColorEdit3("Ambient", &ambient.x)){ isDirty = true; }
    if(ImGui::ColorEdit3("Diffuse", &diffuse.x)){ isDirty = true; }
    if(ImGui::ColorEdit3("Specular", &specular.x)){ isDirty = true; }

}
