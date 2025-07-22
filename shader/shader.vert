#version 420 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 7) in vec4 aColor;           // The material color from the vertex
layout (location = 8) in float aUseTexture; 

out vec2 TexCoords;  
out vec3 FragPos;
out vec3 Normal;
out vec4 vertexColor; 
out float useTextureFlag;
// out vec3 LightPos;

// uniform vec3 lightPos; // we now define the uniform in the vertex shader and pass the 'view space' lightpos to the fragment shader. lightPos is currently in world space.

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


void main()
{
   // vertexPosition = aPos;  // Pass through
   // FragPos = vec3(view * model * vec4(aPos, 1.0));
   // Normal = mat3(transpose(inverse(view * model))) * aNormal;
   // gl_Position = projection * view * model * vec4(aPos, 1.0);
   // LightPos = vec3(view * vec4(lightPos, 1.0)); // Transform world-space light position to view-space light position

   FragPos = vec3(model * vec4(aPos, 1.0));
   Normal = mat3(transpose(inverse(model))) * aNormal;
   gl_Position = projection * view * vec4(FragPos, 1.0);
   TexCoords = aTexCoords;
   vertexColor = aColor;
   useTextureFlag = aUseTexture;    
}

