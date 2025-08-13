#version 420 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;
layout (location = 7) in vec4 aColor;           // The material color from the vertex
layout (location = 8) in float aUseTexture; 

out vec2 TexCoords;  
out vec3 FragPos;
out vec3 Normal;
out vec4 vertexColor; 
out float useTextureFlag;
out mat3 TBN; 

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


void main()
{
 
   FragPos = vec3(model * vec4(aPos, 1.0));
   Normal = mat3(transpose(inverse(model))) * aNormal;
   gl_Position = projection * view * vec4(FragPos, 1.0);
   TexCoords = aTexCoords;
   vertexColor = aColor;
   useTextureFlag = aUseTexture;   

   // Create the TBN matrix to transform from tangent space to world space
   // We use the normal matrix to correctly handle non-uniform scaling
   mat3 normalMatrix = mat3(transpose(inverse(model)));
   vec3 T = normalize(normalMatrix * aTangent);
   vec3 B = normalize(normalMatrix * aBitangent);
   vec3 N = normalize(normalMatrix * aNormal);

   // This matrix will transform a tangent-space normal to a world-space normal
   TBN = mat3(T, B, N); 
}

