#pragma once

#include "Light.h"

class SpotLight : public Light {
public:
    // Attenuation properties (shared with PointLight)
    float constant = 1.0f;
    float linear = 0.09f;
    float quadratic = 0.032f;
    glm::vec3 direction = glm::vec3{-0.2f, -1.0f, -0.3f};

    // Spotlight-specific properties
    float cutOff = 12.5f;        // Inner cone angle in degrees
    float outerCutOff = 15.0f;   // Outer cone angle in degrees

    // A spotlight can be visualized with a model (e.g., a cone or flashlight model)
    SpotLight(std::shared_ptr<Model> model = nullptr) 
        : Light(model, "Spot Light") {}

    // Implement the upload function, required by the base Light class
    void UploadToShader(Shader& shader, int index = 0) const override;

    // Helper to get direction from the GameObject's rotation
    glm::vec3 GetDirection() const;

    virtual void OnImGui(bool &isDirty) override;
};