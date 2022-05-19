#version 310 es

#extension GL_GOOGLE_include_directive : enable

#include "constants.h"

layout(input_attachment_index = 0, set = 0, binding = 0) uniform highp subpassInput in_color;
layout(set=0, binding = 1) uniform highp sampler2D in_sampler_texture;
layout(location = 0) in highp vec2 in_texcoord;
layout(location = 0) out highp vec4 out_color;

highp float pixel_size = 0.002f;

layout(push_constant) uniform push_cons
{
	highp vec4 screen_resolution;
    highp vec4 editor_screen_resolution;
} resolution_data;

highp vec2 GetCorrectUV(highp vec2 uv)
{
    highp vec2 ratio = resolution_data.editor_screen_resolution.zw / resolution_data.screen_resolution.zw;
    highp vec2 offset = resolution_data.editor_screen_resolution.xy / resolution_data.screen_resolution.zw;

    return offset.xy + uv.xy * ratio;
}

highp vec2 PixelizeUV(highp vec2 uv)
{
    highp vec2 correct_uv = GetCorrectUV(uv);
    highp float pixel_scale = 1.0f / pixel_size;
    highp float pixel_ratio = resolution_data.screen_resolution.z / resolution_data.screen_resolution.w;
    //return correctUV;
    return vec2(floor(correct_uv.x / pixel_size) / pixel_scale + pixel_size / 2.0f, floor(correct_uv.y / pixel_size / pixel_ratio) / pixel_scale * pixel_ratio + pixel_size * pixel_ratio / 2.0f);
}

void main()
{
    //highp vec3 color = subpassLoad(in_color).rgb;
    
    out_color = texture(in_sampler_texture, PixelizeUV(in_texcoord));
    //out_color = vec4(PixelizeUV(in_texcoord), 0.0f, 1.0f);
}


