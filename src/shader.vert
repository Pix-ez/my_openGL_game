#version 420 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;
out vec2 TexCoord;  // Pass to fragment shader
out vec3 ourColor;  // Pass to fragment shader

uniform mat4 transform;
void main()
{
   // vertexPosition = aPos;  // Pass through
   gl_Position = transform * vec4(aPos, 1.0f); 
   ourColor = aColor;
   TexCoord = vec2(aTexCoord.x, aTexCoord.y);
}

