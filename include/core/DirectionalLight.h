#pragma once

#include "Light.h"

class DirectionalLight : public Light {
public:
    DirectionalLight() 
        : Light(nullptr, "Directional Light") {}

    // Implement the upload function
    void UploadToShader(Shader& shader, int index = 0) const override;

    // Helper to get direction from the GameObject's rotation
    glm::vec3 GetDirection() const;

    virtual void OnImGui(bool &isSceneDirty) override;

    glm::vec3 direction = glm::vec3{-0.2f, -1.0f, -0.3f};
};