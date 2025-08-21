#pragma once


#include "Light.h"

class PointLight : public Light {
public:
    // Attenuation properties are unique to point lights
    float constant = 1.0f;
    float linear = 0.09f;
    float quadratic = 0.032f;

    PointLight(std::shared_ptr<Model> model = nullptr) 
        : Light(model, "Point Light") {}

    // Implement the upload function
    void UploadToShader(Shader& shader, int index) const override;
};