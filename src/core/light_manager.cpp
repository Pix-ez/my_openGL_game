#include "light_manager.h"
#include "directional_light.h" // Need full definitions here
#include "point_light.h"
#include "spot_light.h"
#include <algorithm>

void LightManager::RegisterLight(Light* light) {
    // Use dynamic_cast to check the type and add to the correct list
    // if (auto dirLight = dynamic_cast<DirectionalLight*>(light)) {
    //     m_dirLights.push_back(dirLight);
    // } else if (auto pointLight = dynamic_cast<PointLight*>(light)) {
    //     m_pointLights.push_back(pointLight);
    // }else if (auto spotLight = dynamic_cast<SpotLight*>(light)) { // <-- ADD THIS CASE
    //     m_spotLights.push_back(spotLight);
    // }


    if (!light) return;

    if (auto dirLight = dynamic_cast<DirectionalLight*>(light)) {
        m_dirLights.push_back(dirLight);
        // std::cout << "[DEBUG] DirectionalLight registered. Total: " << m_dirLights.size() << std::endl;
    } else if (auto pointLight = dynamic_cast<PointLight*>(light)) {
        m_pointLights.push_back(pointLight);
        // std::cout << "[DEBUG] PointLight registered. Total: " << m_pointLights.size() << std::endl;
    } else if (auto spotLight = dynamic_cast<SpotLight*>(light)) {
        m_spotLights.push_back(spotLight);
        // std::cout << "[DEBUG] SpotLight registered. Total: " << m_spotLights.size() << std::endl;
    } else {
        // std::cout << "[DEBUG] ERROR: Unknown light type failed to register." << std::endl;
    }
}

void LightManager::UnregisterLight(Light* light) {
    // This is more complex, but remove_if is a good way to do it
    if (auto dirLight = dynamic_cast<DirectionalLight*>(light)) {
        m_dirLights.erase(std::remove(m_dirLights.begin(), m_dirLights.end(), dirLight), m_dirLights.end());
    } else if (auto pointLight = dynamic_cast<PointLight*>(light)) {
        m_pointLights.erase(std::remove(m_pointLights.begin(), m_pointLights.end(), pointLight), m_pointLights.end());
    } else if (auto spotLight = dynamic_cast<SpotLight*>(light)) { // <-- ADD THIS CASE
        m_spotLights.erase(std::remove(m_spotLights.begin(), m_spotLights.end(), spotLight), m_spotLights.end());
    }
}

void LightManager::UploadLightsToShader(Shader& shader) {

    //  std::cout << "[DEBUG] UploadLightsToShader called. DirLights: " << m_dirLights.size() 
    //           << ", PointLights: " << m_pointLights.size() << std::endl;
    shader.use();

    // Upload the first directional light found (usually there's only one)
    // if (!m_dirLights.empty()) {
    //     // std::cout << "[DEBUG] Uploading directional light data..." << std::endl;
    //     m_dirLights[0]->UploadToShader(shader);
    // }


    if (!m_dirLights.empty()) {
        shader.setBool("u_hasDirLight", true);
        // std::cout << "[DEBUG] direction light adeed" << std::endl;
        m_dirLights[0]->UploadToShader(shader); // Upload its data
    } else {
        shader.setBool("u_hasDirLight", false);
    }

    // Upload all point lights, keeping track of the array index
    int pointLightCount = m_pointLights.size();
    shader.setInt("u_numPointLights", pointLightCount);
    if(!m_pointLights.empty()){
        for (int i = 0; i < pointLightCount; ++i) {
            // Stop if we exceed the max number of lights supported by the shader
            if (i >= 4) break; // Assuming shader supports max 4 point lights (NR_POINT_LIGHTS)
            m_pointLights[i]->UploadToShader(shader, i);
            }
    }
    
    int spotLightCount = m_spotLights.size();
    shader.setInt("u_numSpotLights", spotLightCount);
    if (!m_spotLights.empty()) {
        for (int i = 0; i < spotLightCount; ++i) {
            if (i >= 4) break; // Assuming shader supports max 4 spotlights (NR_SPOT_LIGHTS)
            m_spotLights[i]->UploadToShader(shader, i);
        }
    }

    // Tell the shader how many point lights are active
    // shader.setInt("numPointLights", pointLightCount);
}