#version 420 core


struct Material {
   sampler2D diffuse;
   sampler2D specular;
   sampler2D emission;    
   sampler2D normal;
   sampler2D height;
   sampler2D tnormals;
   float shininess;
};

struct DirLight {
   vec3 direction;

   vec3 ambient;
   vec3 diffuse;
   vec3 specular;
   float intensity;
};
struct PointLight {
   vec3 position;
   
   float constant;
   float linear;
   float quadratic;

   vec3 ambient;
   vec3 diffuse;
   vec3 specular;
   float intensity;
};

struct SpotLight {
   vec3 position;
   vec3 direction;
   float cutOff;
   float outerCutOff;

   float constant;
   float linear;
   float quadratic;

   vec3 ambient;
   vec3 diffuse;
   vec3 specular;  
   float intensity;     
};

#define NR_POINT_LIGHTS 1
#define NR_SPOT_LIGHTS 4 

uniform bool u_hasDirLight;
uniform int u_numPointLights;
uniform int u_numSpotLights;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;in vec4 vertexColor; 
in float useTextureFlag;
in mat3 TBN;

out vec4 FragColor;

uniform vec3 viewPos;
uniform Material material;
uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLights[NR_SPOT_LIGHTS];
uniform float emission_strength;
uniform float time;


// function prototypes
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

float near = 0.1; 
float far  = 100.0; 
  
float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));	
}

void main()
{ 
   
   
   // if (int(useTextureFlag) == 0)
   //  {
   //      FragColor = vertexColor;
   //  }
   //  else
   //  {
      vec4 textureColour = texture(material.diffuse, TexCoords);
         if(textureColour.a < 0.5) {
         discard;
         }
      
      //   vec3 norm = normalize(Normal); //using vertex normals
     // 1. Sample the normal from the normal map. It's in Tangent Space.
    vec3 tangentNormal = texture(material.normal, TexCoords).rgb;


    // 2. Unpack the normal from the [0, 1] color range to the [-1, 1] vector range.
    tangentNormal = normalize(tangentNormal * 2.0 - 1.0);

    // 3. Transform the normal from Tangent Space to World Space using the TBN matrix.
    // THIS IS THE KEY STEP!
    vec3 norm = normalize(TBN * tangentNormal);
    
        vec3 viewDir = normalize(viewPos - FragPos); 
        
      

        // phase 1: directional lighting
        vec3 result = vec3(0);
        if (u_hasDirLight) {
        result += CalcDirLight(dirLight, norm, viewDir);
          }
         

        // phase 2: point lights
        for(int i = 0; i < u_numPointLights; i++)
            result += CalcPointLight(pointLights[i], norm, FragPos, viewDir); 

        // phase 3: spot light
    
      for(int i = 0; i < u_numSpotLights; i++) {
        result += CalcSpotLight(spotLights[i], norm, FragPos, viewDir);
    }
      // Add emission last
    result += texture(material.emission, TexCoords).rgb * emission_strength;
      FragColor = vec4(result, 1.0);
        
      // Output the normal's color. Ignore all textures and lighting.
      //  FragColor = vec4(norm, 1.0);

        //DEBUG
        //show only normals
      //   FragColor = vec4(texture(material.specular, TexCoords).rgb, 1.0);
      //   FragColor = vec4(1.0, 0.0, 0.0 ,1.0);
      
   //    float depth = LinearizeDepth(gl_FragCoord.z) / far; // divide by far for demonstration
   //  FragColor = vec4(vec3(depth), 1.0);
   //  }




}

// calculates the color when using a directional light.
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
   vec3 lightDir = normalize(-light.direction);
   // diffuse shading
   float diff = max(dot(normal, lightDir), 0.0);
   // specular shading
   // vec3 reflectDir = reflect(-lightDir, normal);
   vec3 halfwayDir = normalize(lightDir + viewDir);
   float spec = pow(max(dot(viewDir, halfwayDir), 0.0), material.shininess);
   // combine results
   vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
   vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
   vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
   return (ambient + diffuse + specular) * light.intensity;
}

// calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
   vec3 lightDir = normalize(light.position - fragPos);
   // diffuse shading
   float diff = max(dot(normal, lightDir), 0.0);
   // specular shading
   vec3 reflectDir = reflect(-lightDir, normal);
   // vec3 reflectDir = reflect(-lightDir, normal);
   vec3 halfwayDir = normalize(lightDir + viewDir);
   float spec = pow(max(dot(viewDir, halfwayDir), 0.0), material.shininess);

   // attenuation
   float distance = length(light.position - fragPos);
   float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
   // combine results
   vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
   vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
   vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
   ambient *= attenuation;
   diffuse *= attenuation;
   specular *= attenuation;
   return (ambient + diffuse + specular) * light.intensity;
}

// calculates the color when using a spot light.
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
   vec3 lightDir = normalize(light.position - fragPos);
   // diffuse shading
   float diff = max(dot(normal, lightDir), 0.0);
   // specular shading
   // vec3 reflectDir = reflect(-lightDir, normal);
   vec3 halfwayDir = normalize(lightDir + viewDir);
   float spec = pow(max(dot(viewDir, halfwayDir), 0.0), material.shininess);
   // attenuation
   float distance = length(light.position - fragPos);
   float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
   // spotlight intensity
   float theta = dot(lightDir, normalize(-light.direction)); 
   float epsilon = light.cutOff - light.outerCutOff;
   float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
   // combine results
   vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
   vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
   vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
   ambient *= attenuation * intensity;
   diffuse *= attenuation * intensity;
   specular *= attenuation * intensity;
   return (ambient + diffuse + specular) * light.intensity;
}
