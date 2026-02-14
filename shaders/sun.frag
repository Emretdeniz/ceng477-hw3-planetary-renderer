#version 430
/*
    Sun Fragment Shader
    Renders the sun with a bright color
*/

#define OUT_COLOR layout(location = 0)

// Output
out OUT_COLOR vec4 fragColor;

void main(void)
{
    fragColor = vec4(1.0, 0.95, 0.8, 1.0);

    // Put the sun almost at the far depth so planets drawn later occlude it.
    // Must be < 1.0 because your cleared depth is 1.0 and depth func is GL_LESS.
    gl_FragDepth = 0.999999;
}
