#version 310 es

#extension GL_GOOGLE_include_directive : enable

#include "constants.h"
#include "gbuffer.h"

layout(set = 0, binding = 1) uniform samplerCube specular_sampler;

layout(location = 0) in highp vec3 in_UVW;

layout(location = 0) out highp vec4 out_scene_color;

void main()
{
    highp vec3 origin_sample_UVW = vec3(in_UVW.x, in_UVW.z, in_UVW.y);
    highp vec3 color             = textureLod(specular_sampler, origin_sample_UVW, 0.0).rgb;

    out_scene_color = vec4(color, 1.0);
}