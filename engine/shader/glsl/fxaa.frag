#version 310 es

#extension GL_GOOGLE_include_directive : enable

#include "constants.h"

layout(set = 0, binding = 0) uniform sampler2D in_color;

layout(location = 0) in highp vec2 in_uv;

layout(location = 0) out highp vec4 out_color;

void main()
{
    out_color = texture(in_color, in_uv);
}
