// #version 420 core

// in vec2 TexCoord;
// in vec3 ourColor;
// in vec3 FragPos;
// in vec3 Normal;
// in vec3 LightPos;   // extra in variable, since we need the light position in view space we calculate this in the vertex shader

// out vec4 FragColor;

// uniform sampler2D texture1;
// uniform sampler2D texture2;


// uniform vec3 objectColor;
// uniform vec3 lightColor;

// uniform float lightIntensity;

// void main()
// {
//    // Normalize vertex position to [0, 1] range for color
//    //vec3 color = vertexPosition * 0.5 + 0.5;
//    //FragColor = vec4(color, 1.0);
//    // FragColor = ourColor;
//    //FragColor = vec4(ourColor,1.0);

//    //using texture co ordinate
//    // FragColor = texture(texture2, TexCoord);
//    vec4 base = texture(texture1, TexCoord);
//    vec4 overlay = texture(texture2, TexCoord);
//    vec4 texture_color = mix(overlay, base, overlay.b);
//    // vec4 temp_lightcol = vec4(lightColor, 1.0) * lightIntensity;
//    // FragColor = texture_color * temp_lightcol;
//    // FragColor = overlay * overlay.a + base * (1.0 - overlay.a);
//    // FragColor = texture_color;
//    // FragColor = vec4(lightColor * objectColor, 1.0);


//    // FragColor = mix(texture(texture1, TexCoord),texture(texture2, TexCoord), 1.0);
//    // ambient
//    float ambientStrength = 0.1;
//    vec3 ambient = ambientStrength * lightColor;  

//    // diffuse 
//    vec3 norm = normalize(Normal);
//    vec3 lightDir = normalize(LightPos - FragPos);
//    float diff = max(dot(norm, lightDir), 0.0);
//    vec3 diffuse = diff * lightColor;

//    // specular
//    float specularStrength = 0.5;
//    vec3 viewDir = normalize(-FragPos); // the viewer is always at (0,0,0) in view-space, so viewDir is (0,0,0) - Position => -Position
//    vec3 reflectDir = reflect(-lightDir, norm);  
//    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
//    vec3 specular = specularStrength * spec * lightColor;

//    // vec3 result = (ambient + diffuse + specular) * objectColor;
//    // FragColor = vec4(result, 1.0);

//    vec3 lighting = ambient + diffuse + specular;
//    vec3 colorRGB = lighting * texture_color.xyz;
//    float alpha = texture_color.a;//or we can set to fix 1.0 
//    FragColor = vec4(colorRGB, alpha);

// }




#version 420 core

// struct Material {
//     vec3 ambient;
//     vec3 diffuse;
//     vec3 specular;    
//     float shininess;
// }; 

struct Material {
   sampler2D diffuse;
   sampler2D specular;
   sampler2D emission;    
   float shininess;
};

// struct Light {
//    vec3 ambient;
//    vec3 diffuse;
//    vec3 specular;

//    // for dir light
//    // vec3 position;
   
//    //for point light
//    float constant;
//    float linear;
//    float quadratic;

//    //for spotlight
//    vec3  position;
//    vec3  direction;
//    float cutOff;
//    float outerCutOff;
// };

struct DirLight {
   vec3 direction;

   vec3 ambient;
   vec3 diffuse;
   vec3 specular;
};
struct PointLight {
   vec3 position;
   
   float constant;
   float linear;
   float quadratic;

   vec3 ambient;
   vec3 diffuse;
   vec3 specular;
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
};

#define NR_POINT_LIGHTS 4

in vec2 TexCoords;
in vec3 ourColor;
in vec3 FragPos;
in vec3 Normal;
in vec3 LightPos;   // extra in variable, since we need the light position in view space we calculate this in the vertex shader

out vec4 FragColor;

// uniform sampler2D texture1;
// uniform sampler2D texture2;


// uniform vec3 objectColor;
// uniform vec3 lightColor;

uniform float lightIntensity;
uniform vec3 viewPos;
uniform Material material;
// uniform Light light;
uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLight;

uniform float emission_strength;
uniform float time;

// function prototypes
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
   // vec4 base = texture(texture1, TexCoord);
   // vec4 overlay = texture(texture2, TexCoord);
   // vec4 texture_color = mix(overlay, base, overlay.b);
   // vec3 lightDir = normalize(light.position - FragPos);
    
   // check if lighting is inside the spotlight cone
   // float theta = dot(lightDir, normalize(-light.direction)); 
   // ambient
   //  vec3 ambient = light.ambient * material.ambient;

   // ambient
   // vec3 ambient = light.ambient * texture(material.diffuse, TexCoords).rgb;

   // diffuse 
   vec3 norm = normalize(Normal);
   // vec3 lightDir = normalize(light.position - FragPos);
   // float diff = max(dot(norm, lightDir), 0.0);
   // vec3 diffuse = light.diffuse * (diff * material.diffuse);
   // vec3 diffuse = light.diffuse * diff * texture(material.diffuse, TexCoords).rgb; 

   // specular
   // float specularStrength = 0.5;
   vec3 viewDir = normalize(-FragPos); 
   // vec3 reflectDir = reflect(-lightDir, norm);  
   // float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
   // vec3 specular = light.specular * (spec * material.specular);
   // vec3 specular = light.specular * spec * texture(material.specular, TexCoords).rgb;

   // emission
   vec3 emission = texture(material.emission, TexCoords).rgb;
   emission = texture(material.emission, TexCoords + vec2(0.0,time)).rgb;   /*moving */

   // spotlight (soft edges)
   // float theta = dot(lightDir, normalize(-light.direction)); 
   // float epsilon = (light.cutOff - light.outerCutOff);
   // float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
   // diffuse  *= intensity;
   // specular *= intensity;

   // // attenuation
   // float distance    = length(light.position - FragPos);
   // float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));  

   // ambient  *= attenuation;  // remove attenuation from ambient, as otherwise at large distances the light would be darker inside than outside the spotlight due the ambient term in the else branch
   // diffuse   *= attenuation;
   // specular *= attenuation;  
         
   
   // vec3 ambient  = ambientStrength * lightColor * lightIntensity;
   // vec3 diffuse  = diff             * lightColor * lightIntensity;
   // vec3 specular = specularStrength * spec * lightColor * lightIntensity;

   // vec3 lighting = ambient + diffuse + specular + (emission * emission_strength);
   // phase 1: directional lighting
   vec3 result = CalcDirLight(dirLight, norm, viewDir);
   // phase 2: point lights
   for(int i = 0; i < NR_POINT_LIGHTS; i++)
      result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);    
   // phase 3: spot light
   result += CalcSpotLight(spotLight, norm, FragPos, viewDir);
   result += emission * emission_strength;
   FragColor = vec4(result, 1.0);
   // vec3 finalRGB = lighting * texture_color.rgb;
   // FragColor = vec4(finalRGB, texture_color.a);




}

// calculates the color when using a directional light.
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
   vec3 lightDir = normalize(-light.direction);
   // diffuse shading
   float diff = max(dot(normal, lightDir), 0.0);
   // specular shading
   vec3 reflectDir = reflect(-lightDir, normal);
   float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
   // combine results
   vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
   vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
   vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
   return (ambient + diffuse + specular);
}

// calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
   vec3 lightDir = normalize(light.position - fragPos);
   // diffuse shading
   float diff = max(dot(normal, lightDir), 0.0);
   // specular shading
   vec3 reflectDir = reflect(-lightDir, normal);
   float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
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
   return (ambient + diffuse + specular);
}

// calculates the color when using a spot light.
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
   vec3 lightDir = normalize(light.position - fragPos);
   // diffuse shading
   float diff = max(dot(normal, lightDir), 0.0);
   // specular shading
   vec3 reflectDir = reflect(-lightDir, normal);
   float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
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
   return (ambient + diffuse + specular);
}