#pragma once
#include "shader.h"
#include "texture.h"
#include <glm/glm.hpp>
#include <string>
#include <map>
#include <memory>

// Forward declaration of Texture class
class Texture;

class Material{

    public:
    //using smart pointer to auto mangement of memory
    std::shared_ptr<Shader> shader;
    std::map<std::string, std::shared_ptr<Texture>> textures;

    //Material properties
    glm::vec3 color = glm::vec3{1.0f}; //fallback color
    float shininess = 32.0f;
    Material(std::shared_ptr<Shader> sh) : shader(sh) {}

    // determines the uniform name based on the texture's type.
    void addTexture(const std::shared_ptr<Texture>& texture) {
        if (!texture) return;

        switch (texture->getType()) {
            case Texture::TextureType::Diffuse:
                textures["material.diffuse"] = texture;
                break;
            case Texture::TextureType::Specular:
                textures["material.specular"] = texture;
                break;
            case Texture::TextureType::Normal:
                textures["material.normal"] = texture;
                break;
            // Add other cases here if you support more texture types
        }
    }

    // Texture class's own bind method for better encapsulation.
    void apply() const {
        if (!shader) return; // Safety check

        shader->use();

        shader->setVec3("material.color", color);
        shader->setFloat("material.shininess", shininess);

        int textureUnit = 0;
        for (const auto& [name, tex] : textures) {
            // Tell the texture to bind itself to the correct unit
            tex->bind(textureUnit);
            // Tell the shader which texture unit the sampler 'name' should use
            shader->setInt(name, textureUnit);
            textureUnit++;
        }
    }



};