#version 430
/*
    Background Vertex Shader
    For rendering stars sphere with orthographic projection
*/

#define IN_POS          layout(location = 0)
#define IN_NORMAL       layout(location = 1)
#define IN_UV           layout(location = 2)

#define OUT_UV          layout(location = 0)

#define U_MODEL         layout(location = 0)
#define U_VIEW          layout(location = 1)
#define U_PROJ          layout(location = 2)

// Input
in IN_POS       vec3 vPos;
in IN_NORMAL    vec3 vNormal;
in IN_UV        vec2 vUV;

// Output
out gl_PerVertex {vec4 gl_Position;};
out OUT_UV vec2 fUV;

// Uniforms
U_MODEL uniform mat4 uModel;
U_VIEW  uniform mat4 uView;
U_PROJ  uniform mat4 uProjection;

void main(void)
{
    gl_Position = uProjection * uView * uModel * vec4(vPos, 1.0);
    fUV = vUV;
}
