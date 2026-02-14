#version 430
/*
    Cloud Fragment Shader
    Renders clouds with alpha blending
*/

// Definitions
#define IN_UV           layout(location = 0)
#define IN_NORMAL       layout(location = 1)
#define IN_WORLD_POS    layout(location = 2)

#define OUT_COLOR       layout(location = 0)

#define T_CLOUD         layout(binding = 0)

#define U_LIGHT_DIR     layout(location = 4)
#define U_LIGHT_COLOR   layout(location = 6)

// Input
IN_UV  in          vec2 fUV;
IN_NORMAL  in      vec3 fNormal;
IN_WORLD_POS  in   vec3 fWorldPos;

// Output
OUT_COLOR out vec4 fragColor;

// Uniforms
U_LIGHT_DIR     uniform vec3 uLightDir;
U_LIGHT_COLOR   uniform vec3 uLightColor;

// Textures
T_CLOUD uniform sampler2D tCloudMap;

void main(void)
{
    // Sample cloud texture (has alpha channel)
    vec4 cloudSample = texture(tCloudMap, fUV);
    
    // Normalize vectors
    vec3 normal = normalize(fNormal);
    vec3 lightDir = normalize(-uLightDir);
    
    // Simple diffuse lighting for clouds
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 cloudColor = cloudSample.rgb * (0.3 + 0.7 * diff) * uLightColor;
    
    fragColor = vec4(cloudColor, cloudSample.a);
}
