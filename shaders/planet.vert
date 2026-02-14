#version 430
/*
    Planet Vertex Shader
    Transforms vertices and passes data to fragment shader
*/

// Definitions
#define IN_POS          layout(location = 0)
#define IN_NORMAL       layout(location = 1)
#define IN_UV           layout(location = 2)

#define OUT_UV          layout(location = 0)
#define OUT_NORMAL      layout(location = 1)
#define OUT_WORLD_POS   layout(location = 2)

#define U_MODEL         layout(location = 0)
#define U_VIEW          layout(location = 1)
#define U_PROJ          layout(location = 2)
#define U_NORMAL_MAT    layout(location = 3)

// Input
in IN_POS       vec3 vPos;
in IN_NORMAL    vec3 vNormal;
in IN_UV        vec2 vUV;

// Output
out gl_PerVertex {vec4 gl_Position;};
out OUT_UV          vec2 fUV;
out OUT_NORMAL      vec3 fNormal;
out OUT_WORLD_POS   vec3 fWorldPos;

// Uniforms
U_MODEL         uniform mat4 uModel;
U_VIEW          uniform mat4 uView;
U_PROJ          uniform mat4 uProjection;
U_NORMAL_MAT    uniform mat3 uNormalMatrix;

void main(void)
{
    // Transform position to world space
    vec4 worldPos = uModel * vec4(vPos, 1.0);
    fWorldPos = worldPos.xyz;
    
    // Transform to clip space
    gl_Position = uProjection * uView * worldPos;
    
    // Transform normal to world space
    fNormal = normalize(uNormalMatrix * vNormal);
    
    // Pass UV coordinates
    fUV = vUV;
}
