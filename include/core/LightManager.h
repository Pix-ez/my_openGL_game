#pragma once

#include <vector>
#include <memory>
#include "Shader.h"

// Forward declare to avoid including full headers
class DirectionalLight;
class PointLight;
class Light;
class SpotLight;

class LightManager {
public:
    // Called from Light's constructor/destructor
    void RegisterLight(Light* light);
    void UnregisterLight(Light* light);

    // The main function called by the render loop
    void UploadLightsToShader(Shader& shader);

private:
    // Separate lists for each type of light for easy iteration
    std::vector<DirectionalLight*> m_dirLights;
    std::vector<PointLight*> m_pointLights;
    std::vector<SpotLight*> m_spotLights; 
    // You can add m_spotLights here later
};