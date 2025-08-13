#pragma once
#include <string>
#include <glad/glad.h>

class Texture{
    public:
    enum TextureType{
        Diffuse,
        Specular,
        Normal,
        Height,
        Unknown
    
    };

    Texture(const std::string& filepath, TextureType type);
    ~Texture();

    void bind(unsigned int uint = 0) const;
    void unbind() const;

    inline GLuint getID() const {return id;}
    inline TextureType getType() const { return textureType; }
    inline const std::string& getPath() const { return path; }
   
    private:
    
    GLuint id;
    TextureType textureType;
    std::string path;

    void loadFromFile(const std::string& filepath, bool gammaCorrection);
};