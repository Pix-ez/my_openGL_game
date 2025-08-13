#include "light.h"
#include "light_manager.h"
#include "imgui.h"


// Define the static member
LightManager* Light::s_lightManager = nullptr;

Light::Light(std::shared_ptr<Model> model, const std::string& name) : GameObject(model, name) {
    // if (s_lightManager) {
    //     s_lightManager->RegisterLight(this);
    // }
}

Light::~Light() {
    // if (s_lightManager) {
    //     s_lightManager->UnregisterLight(this);
    // }
}


void Light::OnImGui() {
    // 1. Call the parent's OnImGui function first to draw the Transform section.
    GameObject::OnImGui(); 

    // 2. Add a separator for visual clarity.
    ImGui::Separator();
    
    // 3. Add the UI widgets for properties common to ALL lights.
    ImGui::Text("Light Properties");
    ImGui::ColorEdit3("Ambient Color", &ambient.x);
    ImGui::ColorEdit3("Diffuse Color", &diffuse.x);
    ImGui::ColorEdit3("Specular Color", &specular.x);
    ImGui::DragFloat("Intensity", &intensity, 0.05f, 0.0f, 100.0f);
}