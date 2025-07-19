#version 420 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aNormal;

out vec2 TexCoords;  // Pass to fragment shader
out vec3 ourColor;  // Pass to fragment shader
out vec3 FragPos;
out vec3 Normal;
out vec3 LightPos;

uniform vec3 lightPos; // we now define the uniform in the vertex shader and pass the 'view space' lightpos to the fragment shader. lightPos is currently in world space.

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


void main()
{
   // vertexPosition = aPos;  // Pass through
   gl_Position = projection * view * model * vec4(aPos, 1.0);
   ourColor = aColor;
   FragPos = vec3(view * model * vec4(aPos, 1.0));
   Normal = mat3(transpose(inverse(view * model))) * aNormal;
   LightPos = vec3(view * vec4(lightPos, 1.0)); // Transform world-space light position to view-space light position
   TexCoords = vec2(aTexCoord.x, 1.0 - aTexCoord.y);
}

