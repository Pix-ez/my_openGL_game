#pragma once

#include "Shader.h"
#include "GameObject.h"


// Forward declaration to avoid circular dependency
class LightManager;

class Light: public GameObject {

public:
    //common light properties
    glm::vec3 ambient{0.05f, 0.05f, 0.05f};
    glm::vec3 diffuse{1.0f, 1.0f, 1.0f};
    glm::vec3 specular{1.0f, 1.0f, 1.0f};
    float intensity = 1.0f;

    //Constructure
    Light(std::shared_ptr<Model> model, const std::string& name = "Light");
    // Destructor unregisters it
    virtual ~Light() override;


    // Pure virtual function that all derived lights MUST implement.
    // This is how a light sends its unique data to a shader.
    // The 'index' is for arrays of lights (e.g., pointLights[i]).
    virtual void UploadToShader(Shader& shader, int index = 0) const = 0;

    protected:
    // Static manager instance
    static LightManager* s_lightManager;

    virtual void OnImGui(bool& isDirty) override;

    public:
        // Method to set the manager for all lights to use
        static void SetLightManager(LightManager* manager) {
            s_lightManager = manager;

            if (s_lightManager) {
        std::cout << "[DEBUG] LightManager was SET successfully." << std::endl;
    } else {
        std::cout << "[DEBUG] ERROR: LightManager was set to NULL." << std::endl;
    }
        }
};
