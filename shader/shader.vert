#version 420 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;
layout (location = 7) in vec4 aColor;           // The material color from the vertex
layout (location = 8) in float aUseTexture; 


out VS_OUT {
   vec4 FragPosLightSpace;
   vec2 TexCoords;  
   vec3 FragPos;  
   vec3 Normal;
   vec4 vertexColor; 
   float useTextureFlag;
   mat3 TBN; 

} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

void main()
{
 
   vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
   vs_out.Normal = mat3(transpose(inverse(model))) * aNormal;
   gl_Position = projection * view * vec4(vs_out.FragPos, 1.0);
   vs_out.TexCoords = aTexCoords;
   vs_out.vertexColor = aColor;
   vs_out.useTextureFlag = aUseTexture;   
   vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(vs_out.FragPos, 1.0);
   // Create the TBN matrix to transform from tangent space to world space
   // We use the normal matrix to correctly handle non-uniform scaling
   mat3 normalMatrix = mat3(transpose(inverse(model)));
   vec3 T = normalize(normalMatrix * aTangent);
   vec3 B = normalize(normalMatrix * aBitangent);
   vec3 N = normalize(normalMatrix * aNormal);

   // This matrix will transform a tangent-space normal to a world-space normal
   vs_out.TBN = mat3(T, B, N); 
}

