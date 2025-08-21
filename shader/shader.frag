#version 420 core


struct Material {
   sampler2D diffuseSampler;
   sampler2D specularSampler;
   sampler2D emissionSampler;    
   sampler2D normalSampler;
   sampler2D height;
   sampler2D tnormals;
   

   // Flags to control logic
   bool useDiffuseTexture;
   bool useSpecularTexture;
   bool useNormalTexture;

   // Fallback colors
   vec3 diffuseColor;
   vec3 specularColor;
   vec3 emissionColor;
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

// in vec2 TexCoords;
// in vec3 FragPos;
// in vec3 Normal;in vec4 vertexColor; 
// in float useTextureFlag;
// in mat3 TBN;

in VS_OUT {
   vec4 FragPosLightSpace;
   vec2 TexCoords;  
   vec3 FragPos;  
   vec3 Normal;
   vec4 vertexColor; 
   float useTextureFlag;
   mat3 TBN; 

} fs_in;
out vec4 FragColor;

uniform vec3 viewPos;
uniform sampler2D shadowMap;
uniform Material material;
uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLights[NR_SPOT_LIGHTS];
uniform float emission_strength;
uniform float time;


// function prototypes
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 diffuseColor, vec3 specularColor, float shininess);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 diffuseColor, vec3 specularColor, float shininess);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 diffuseColor, vec3 specularColor, float shininess);

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);  
   //  float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;  
   float shadow = 0.0;
   vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
   for(int x = -1; x <= 1; ++x)
   {
      for(int y = -1; y <= 1; ++y)
      {
         float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
         shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
      }    
   }
   shadow /= 9.0;

   if(projCoords.z > 1.0)
      shadow = 0.0;
   
    return shadow;
}
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
   vec4 diffuseBaseColor;
    if (material.useDiffuseTexture) {
        diffuseBaseColor = texture(material.diffuseSampler, fs_in.TexCoords);
    } else {
        diffuseBaseColor = vec4(material.diffuseColor, 1.0);
    }

    vec3 specularComponent;
    if (material.useSpecularTexture) {
        specularComponent = texture(material.specularSampler, fs_in.TexCoords).rgb;
    } else {
        specularComponent = material.specularColor;
    }

     vec3 norm;
    if (material.useNormalTexture) {
        vec3 tangentNormal = texture(material.normalSampler, fs_in.TexCoords).rgb;
        tangentNormal = normalize(tangentNormal * 2.0 - 1.0);
        norm = normalize(fs_in.TBN * tangentNormal);
    } else {
        // Fallback to vertex normals (make sure they are passed from VS)
        norm = normalize(fs_in.Normal); 
    }

    float shininess = material.shininess;

      // vec4 textureColour = texture(material.diffuse, TexCoords);
         if(diffuseBaseColor.a < 0.5) {
         discard;
         }
      
      //   vec3 norm = normalize(Normal); //using vertex normals
     // 1. Sample the normal from the normal map. It's in Tangent Space.
   //  vec3 tangentNormal = texture(material.normal, TexCoords).rgb;


    // 2. Unpack the normal from the [0, 1] color range to the [-1, 1] vector range.
   //  tangentNormal = normalize(tangentNormal * 2.0 - 1.0);

    // 3. Transform the normal from Tangent Space to World Space using the TBN matrix.
    // THIS IS THE KEY STEP!
   //  vec3 norm = normalize(TBN * tangentNormal);
    
        vec3 viewDir = normalize(viewPos - fs_in.FragPos); 
        
      

        // phase 1: directional lighting
        vec3 result = vec3(0);
        if (u_hasDirLight) {
        result += CalcDirLight(dirLight, norm, viewDir, diffuseBaseColor.rgb, specularComponent.rgb, shininess);
          }
         

        // phase 2: point lights
        for(int i = 0; i < u_numPointLights; i++)
            result += CalcPointLight(pointLights[i], norm, fs_in.FragPos, viewDir, diffuseBaseColor.rgb, specularComponent.rgb, shininess);

        // phase 3: spot light
    
      for(int i = 0; i < u_numSpotLights; i++) {
        result += CalcSpotLight(spotLights[i], norm, fs_in.FragPos, viewDir, diffuseBaseColor.rgb, specularComponent.rgb, shininess);
    }
      // Add emission last
    result += texture(material.emissionSampler, fs_in.TexCoords).rgb * emission_strength;
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
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 diffuseColor, vec3 specularColor, float shininess)
{
   vec3 lightDir = normalize(-light.direction);
   // diffuse shading
   float diff = max(dot(normal, lightDir), 0.0);
   // specular shading
   // vec3 reflectDir = reflect(-lightDir, normal);
   vec3 halfwayDir = normalize(lightDir + viewDir);
   float spec = pow(max(dot(viewDir, halfwayDir), 0.0), shininess);
   // combine results
   vec3 ambient = light.ambient * diffuseColor;
   vec3 diffuse = light.diffuse * diff * diffuseColor;
   vec3 specular = light.specular * spec * diffuseColor;
   // calculate shadow
   float shadow = ShadowCalculation(fs_in.FragPosLightSpace, normal , lightDir);                      
   vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * diffuseColor; 
   lighting *= light.intensity;
   // return (ambient + diffuse + specular) * light.intensity;
   return lighting;
}

// calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 diffuseColor, vec3 specularColor, float shininess)
{
   vec3 lightDir = normalize(light.position - fragPos);
   // diffuse shading
   float diff = max(dot(normal, lightDir), 0.0);
   // specular shading
   vec3 reflectDir = reflect(-lightDir, normal);
   // vec3 reflectDir = reflect(-lightDir, normal);
   vec3 halfwayDir = normalize(lightDir + viewDir);
   float spec = pow(max(dot(viewDir, halfwayDir), 0.0), shininess);

   // attenuation
   float distance = length(light.position - fragPos);
   float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
   // combine results
   vec3 ambient = light.ambient * diffuseColor;
   vec3 diffuse = light.diffuse * diff * diffuseColor;
   vec3 specular = light.specular * spec * specularColor;
   ambient *= attenuation;
   diffuse *= attenuation;
   specular *= attenuation;
   return (ambient + diffuse + specular) * light.intensity;
}

// calculates the color when using a spot light.
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 diffuseColor, vec3 specularColor, float shininess)
{
   vec3 lightDir = normalize(light.position - fragPos);
   // diffuse shading
   float diff = max(dot(normal, lightDir), 0.0);
   // specular shading
   // vec3 reflectDir = reflect(-lightDir, normal);
   vec3 halfwayDir = normalize(lightDir + viewDir);
   float spec = pow(max(dot(viewDir, halfwayDir), 0.0), shininess);
   // attenuation
   float distance = length(light.position - fragPos);
   float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
   // spotlight intensity
   float theta = dot(lightDir, normalize(-light.direction)); 
   float epsilon = light.cutOff - light.outerCutOff;
   float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
   // combine results
   vec3 ambient = light.ambient * diffuseColor;
   vec3 diffuse = light.diffuse * diff * diffuseColor;
   vec3 specular = light.specular * spec * specularColor;
   ambient *= attenuation * intensity;
   diffuse *= attenuation * intensity;
   specular *= attenuation * intensity;
   return (ambient + diffuse + specular) * light.intensity;
}
