#version 310 es

#extension GL_GOOGLE_include_directive : enable

#include "constants.h"

layout(input_attachment_index = 0, set = 0, binding = 0) uniform highp
subpassInput in_color;

layout(push_constant) uniform Param{
    highp flaot time;
    highp float speed;
    highp float size;
} param;


layout(location = 0) out hignp vec4 out_color;

void main()
{
    highp vec4 color = subpassLoad(in_color).rgba;
    out_color = color;
}