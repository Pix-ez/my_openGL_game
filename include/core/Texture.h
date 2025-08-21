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
        BaseColor,
        Unknown
    
    };
    

    Texture(const std::string& filepath, TextureType type);
    ~Texture();

    void bind(unsigned int uint = 0) const;
    void unbind() const;

    bool isLoaded() const { return m_isLoaded; }

    inline GLuint getID() const {return m_id;}
    inline TextureType getType() const { return m_type; }
    inline const std::string& getPath() const { return m_path; }
   
    private:
    
    GLuint m_id;
    TextureType m_type;
    std::string m_path;
    bool m_isLoaded = false;

    void loadFromFile(const std::string& filepath, bool gammaCorrection);
};