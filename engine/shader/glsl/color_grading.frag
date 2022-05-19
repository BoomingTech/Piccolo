#version 310 es

#extension GL_GOOGLE_include_directive : enable

#include "constants.h"

layout(input_attachment_index = 0, set = 0, binding = 0) uniform highp subpassInput in_color;

layout(set = 0, binding = 1) uniform sampler2D color_grading_lut_texture_sampler;

layout(location = 0) out highp vec4 out_color;

void main()
{
    highp ivec2 lut_tex_size = textureSize(color_grading_lut_texture_sampler, 0);
    highp float _COLORS      = float(lut_tex_size.y);

    highp vec4 color       = subpassLoad(in_color).rgba;
    highp float ratio = float(lut_tex_size.x / lut_tex_size.y);
    highp float chunk = floor(color.b * ratio);

    highp vec2 uv = vec2(color.r / ratio + chunk / ratio, color.g);
    highp vec4 color_sampled = texture(color_grading_lut_texture_sampler, uv);

    highp vec2 uv2 = vec2(color.r / ratio + clamp(chunk + 1.0, 1.0, ratio - 1.0) / ratio, color.g);
    highp vec4 color_sampled2 = texture(color_grading_lut_texture_sampler, uv2);

    out_color = mix(color_sampled, color_sampled2, (color.b - chunk / ratio)*ratio);
}
