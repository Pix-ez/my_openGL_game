#pragma once
#include "Shader.h"
#include "Texture.h"
#include <glm/glm.hpp>
#include <string>
#include <map>
#include <memory>

// Forward declaration of Texture class
class Texture;

class Material{

    public:
    //using smart pointer to auto mangement of memory
    // std::shared_ptr<Shader> shader;
    std::map<std::string, std::shared_ptr<Texture>> textures;

    //Material properties
    // glm::vec3 BaseColor = glm::vec3{0.0f};
    // std::cout<< "base color found" <<std::endl;
    // Assign a random color to BaseColor and set it in the shader
    glm::vec3 BaseColor = glm::vec3{
            static_cast<float>(rand()) / static_cast<float>(RAND_MAX),
            static_cast<float>(rand()) / static_cast<float>(RAND_MAX),
            static_cast<float>(rand()) / static_cast<float>(RAND_MAX)
        };
    glm::vec3 fallBackColor = glm::vec3{1.0f}; //fallback color
    float shininess = 32.0f;
     Material() = default;

    // Material(std::shared_ptr<Shader> sh) : shader(sh) {}

    // determines the uniform name based on the texture's type.
    void addTexture(const std::shared_ptr<Texture>& texture) {
        if (!texture) return;

        switch (texture->getType()) {
            case Texture::TextureType::Diffuse:
                textures["material.diffuseSampler"] = texture;
                break;
                case Texture::TextureType::Specular:
                textures["material.specularSampler"] = texture;
                break;
                case Texture::TextureType::Normal:
                textures["material.normalSampler"] = texture;
                break;
            // Add other cases here if you support more texture types
        }
    }

    // // Texture class's own bind method for better encapsulation.
    // void apply(int& textureUnit) const {
    //     if (!shader) return; // Safety check

    //     // shader->use();
    //     shader->setBool("material.useDiffuseTexture", textures.count("material.diffuseSampler") > 0);
    //     shader->setBool("material.useSpecularTexture", textures.count("material.specularSampler") > 0);
    //     shader->setBool("material.useNormalTexture", textures.count("material.normalSampler") > 0);

    //     if (BaseColor == glm::vec3(0.0f)){
    //         // std::cout<< "no base color found" <<std::endl;
    //         shader->setVec3("material.diffuseColor", fallBackColor);
    //     }else{
    //         // std::cout<< "base color found" <<std::endl;
    //         shader->setVec3("material.diffuseColor", BaseColor);
    //     }
        
    //     shader->setFloat("material.shininess", shininess);

    //     // int textureUnit = 0;
    //     for (const auto& [name, tex] : textures) {
    //         // Tell the texture to bind itself to the correct unit
    //         tex->bind(textureUnit);
    //         // Tell the shader which texture unit the sampler 'name' should use
    //         shader->setInt(name, textureUnit);
    //         textureUnit++;
    //     }
    // }
    
    void apply(Shader& shader, int& textureUnit) const {
        // if (!shader) return; // Safety check

        // shader->use();
        shader.setBool("material.useDiffuseTexture", textures.count("material.diffuseSampler") > 0);
        shader.setBool("material.useSpecularTexture", textures.count("material.specularSampler") > 0);
        shader.setBool("material.useNormalTexture", textures.count("material.normalSampler") > 0);

        if (BaseColor == glm::vec3(0.0f)){
            // std::cout<< "no base color found" <<std::endl;
            shader.setVec3("material.diffuseColor", fallBackColor);
        }else{
            
            shader.setVec3("material.diffuseColor", BaseColor);
        }
        
        shader.setFloat("material.shininess", shininess);

        // int textureUnit = 0;
        for (const auto& [name, tex] : textures) {
            // Tell the texture to bind itself to the correct unit
            tex->bind(textureUnit);
            // Tell the shader which texture unit the sampler 'name' should use
            shader.setInt(name, textureUnit);
            textureUnit++;
        }
    }


};