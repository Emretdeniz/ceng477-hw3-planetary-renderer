#version 430
/*
    Earth Fragment Shader
    Advanced lighting with specular map, night map, and smooth day/night transition
*/

// Definitions
#define IN_UV           layout(location = 0)
#define IN_NORMAL       layout(location = 1)
#define IN_WORLD_POS    layout(location = 2)

#define OUT_COLOR       layout(location = 0)

#define T_ALBEDO        layout(binding = 0)
#define T_SHADOW        layout(binding = 1)
#define T_SPECULAR      layout(binding = 2)
#define T_NIGHT         layout(binding = 3)

#define U_LIGHT_DIR     layout(location = 4)
#define U_CAMERA_POS    layout(location = 5)
#define U_LIGHT_COLOR   layout(location = 6)
#define U_LIGHT_VP      layout(location = 7)

// Input
IN_UV  in          vec2 fUV;
IN_NORMAL  in      vec3 fNormal;
IN_WORLD_POS  in   vec3 fWorldPos;

// Output
OUT_COLOR out vec4 fragColor;

// Uniforms
U_LIGHT_DIR     uniform vec3 uLightDir;
U_CAMERA_POS    uniform vec3 uCameraPos;
U_LIGHT_COLOR   uniform vec3 uLightColor;
U_LIGHT_VP      uniform mat4 uLightVP;

// Textures
T_ALBEDO  uniform  sampler2D tAlbedo;
T_SHADOW  uniform  sampler2D tShadowMap;
T_SPECULAR uniform sampler2D tSpecularMap;
T_NIGHT  uniform   sampler2D tNightMap;

float calculateShadow(vec3 worldPos)
{
    // Transform world position to light space
    vec4 lightSpacePos = uLightVP * vec4(worldPos, 1.0);
    
    // Perspective divide
    vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
    
    // Transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    
    // Check if outside shadow map bounds
    if(projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 || 
       projCoords.y < 0.0 || projCoords.y > 1.0)
        return 0.0;
    
    // Get depth from shadow map
    float closestDepth = texture(tShadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    
    // Shadow bias to prevent shadow acne
    float bias = 0.005;
    
    // Check if in shadow
    float shadow = (currentDepth - bias) > closestDepth ? 1.0 : 0.0;
    
    return shadow;
}

void main(void)
{
    // Sample textures
    vec3 albedo = texture(tAlbedo, fUV).rgb;
    float specularMask = texture(tSpecularMap, fUV).r;
    vec3 nightLights = texture(tNightMap, fUV).rgb;
    
    // Normalize vectors
    vec3 normal = normalize(fNormal);
    vec3 lightDir = normalize(-uLightDir);
    vec3 viewDir = normalize(uCameraPos - fWorldPos);
    
    // Calculate lighting factors
    float diffuseFactor = max(dot(normal, lightDir), 0.0);
    
    // Ambient component
    vec3 ambient = 0.1 * albedo;
    
    // Calculate shadow
    float shadow = calculateShadow(fWorldPos);
    
    // Diffuse component
    vec3 diffuse = diffuseFactor * albedo * uLightColor;
    
    // Specular component with varying specular power based on specular map
    // Water (high specular mask) = high shininess, Land (low specular mask) = low shininess
    float specularPower = mix(8.0, 64.0, specularMask);  // 8 for land, 64 for water
    vec3 halfDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfDir), 0.0), specularPower);
    vec3 specular = spec * uLightColor * specularMask * 0.8;
    
    // Night lights: visible on dark side of planet
    // Use smooth transition based on diffuse factor
    float nightFactor = 1.0 - smoothstep(0.05, 0.25, diffuseFactor);
    vec3 nightContribution = nightLights * nightFactor * 1.5;
    
    // Combine everything
    // Ambient always present
    // Day lighting affected by shadow
    // Night lights visible on dark side
    vec3 dayColor = ambient + (1.0 - shadow) * (diffuse + specular);
    vec3 color = dayColor + nightContribution;
    
    fragColor = vec4(color, 1.0);
}
