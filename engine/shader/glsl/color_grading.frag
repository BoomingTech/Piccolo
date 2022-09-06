#version 310 es

#extension GL_GOOGLE_include_directive : enable

// debugging defines
// #define DEBUG
// #define VERTIAL_SPLIT

#include "constants.h"

layout(input_attachment_index = 0, set = 0, binding = 0) uniform highp subpassInput in_color;

layout(set = 0, binding = 1) uniform sampler2D color_grading_lut_texture_sampler;

#ifdef DEBUG
layout(location = 0) in highp vec2 in_uv;
#endif

layout(location = 0) out highp vec4 out_color;

highp vec2 lut_uv(highp float red, highp float green, highp float blue_slice, highp float slice_size) {
    // u = (u_Red + u_Blue) / size, v = u_green
    // `min` fix special case for color.b = 1.0F
    return vec2((red + min(blue_slice, slice_size - 1.0F)) / slice_size, green);
}

highp float get_slice_size() {
    highp ivec2 lut_tex_size = textureSize(color_grading_lut_texture_sampler, 0);
    return float(lut_tex_size.y);
    // return 16.0F; // can be hard cored as 16 for color_grading_lut_01.png
}

highp vec4 get_lut_color(highp vec4 color, highp float slice_size) {
    // scale blue by slice_size, and get integral and fractional part
    // example: with a sampler of 16 slices
    // 5.6(scaled blue) => slice 5, slice_weight 0.6
    highp float slice;
    // max blue = 15.0, then rgb (0.6, g, 1.0) will be sampled from (15.6/16.0, g)
    highp float slice_weight = modf(color.b * (slice_size - 1.0F), slice);

    highp vec4 color_left = textureLod(color_grading_lut_texture_sampler, lut_uv(color.r, color.g, slice, slice_size), 0.0);
    highp vec4 color_right = textureLod(color_grading_lut_texture_sampler, lut_uv(color.r, color.g, slice + 1.0F, slice_size), 0.0);

    return vec4(
        // interpolate 1D using weight as fractional part of scaled blue
        mix(color_left, color_right, slice_weight).rgb,
        // using alpha from original color
        color.a);
}

void main() {
    highp vec4 color = subpassLoad(in_color).rgba;
    highp float slice_size = get_slice_size();

#ifdef DEBUG
#ifdef VERTIAL_SPLIT
    if (in_uv.x > 0.5) {
#else
    if (in_uv.x > in_uv.y) {
#endif
        out_color = color;
    } else {
#endif
    // shader out_color as following:
    out_color = get_lut_color(color, slice_size);
#ifdef DEBUG
    }
#endif
}
