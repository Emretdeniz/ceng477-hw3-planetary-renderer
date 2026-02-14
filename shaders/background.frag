#version 430
/*
    Background Fragment Shader
    Simply samples the stars texture
*/

#define IN_UV       layout(location = 0)
#define OUT_COLOR   layout(location = 0)
#define T_ALBEDO    layout(binding = 0)

// Input
in IN_UV vec2 fUV;

// Output
out OUT_COLOR vec4 fragColor;

// Textures
uniform T_ALBEDO sampler2D tAlbedo;

void main(void)
{
    fragColor = texture(tAlbedo, fUV);
}
