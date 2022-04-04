#version 310 es

#extension GL_GOOGLE_include_directive : enable

#include "constants.h"

void main()
{
    vec3 fullscreen_triangle_positions[3];
    fullscreen_triangle_positions[0] = vec3(3.0, 1.0, 0.5);
    fullscreen_triangle_positions[1] = vec3(-1.0, 1.0, 0.5);
    fullscreen_triangle_positions[2] = vec3(-1.0, -3.0, 0.5);

    gl_Position = vec4(fullscreen_triangle_positions[gl_VertexIndex], 1.0);
}