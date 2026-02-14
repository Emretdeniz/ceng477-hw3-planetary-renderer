#version 430
/*
    Shadow Vertex Shader
    Transforms vertices to light space for shadow mapping
*/

#define IN_POS          layout(location = 0)

#define U_MODEL         layout(location = 0)
#define U_VIEW          layout(location = 1)
#define U_PROJ          layout(location = 2)

// Input
IN_POS in vec3 vPos;

// Output
out gl_PerVertex {vec4 gl_Position;};

// Uniforms
U_MODEL uniform mat4 uModel;
U_VIEW  uniform mat4 uView;
U_PROJ  uniform mat4 uProjection;

void main(void)
{
    gl_Position = uProjection * uView * uModel * vec4(vPos, 1.0);
}
