#version 310 es

#extension GL_GOOGLE_include_directive : enable

#include "constants.h"
#include "gbuffer.h"

layout(location = 0) in highp vec4 in_color;
layout(location = 1) in highp vec2 in_uv;

layout(set = 0, binding = 2) uniform sampler2D sparktexture;

layout(location = 0) out highp vec4 out_scene_color;

void main() { 
    out_scene_color.xyz = 4.0f * texture(sparktexture, in_uv).r * in_color.xyz;
    out_scene_color.w = texture(sparktexture, in_uv).r;
}