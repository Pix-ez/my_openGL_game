#include "PointLight.h"

void PointLight::UploadToShader(Shader& shader, int index) const {
    // Construct the uniform name string, e.g., "pointLights[0]."
    std::string uniformName = "pointLights[" + std::to_string(index) + "]";
    glm::vec3 worldPosition = glm::vec3(GetWorldTransformMatrix()[3]);

    shader.setVec3(uniformName + ".position", worldPosition); // Position comes from GameObject
    shader.setVec3(uniformName + ".ambient", ambient);
    shader.setVec3(uniformName + ".diffuse", diffuse);
    shader.setVec3(uniformName + ".specular", specular);
    shader.setFloat(uniformName + ".constant", constant);
    shader.setFloat(uniformName + ".linear", linear);
    shader.setFloat(uniformName + ".quadratic", quadratic);
    shader.setFloat(uniformName + ".intensity", intensity);
}