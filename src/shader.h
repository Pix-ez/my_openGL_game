#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>


class Shader
{
    public:
    //program ID
    unsigned int ID;

    //contructor for shader
    Shader(const char* vertex_path, const char* fragment_path);
    //use shader
    void use();

    //utility functions
    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;

    private:
    //utility functions
    void checkCompileErrors(unsigned int shader, std::string type);

    
};
#endif