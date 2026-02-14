#version 430
/*
    Shadow Fragment Shader
    Writes depth value to color texture for shadow mapping
*/

// Output
layout(location = 0) out float fragDepth;

void main()
{
    // Write the fragment's depth value to the color texture
    // gl_FragCoord.z is the depth value in [0, 1] range
    fragDepth = gl_FragCoord.z;
}
