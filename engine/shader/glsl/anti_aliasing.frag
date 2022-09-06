#version 310 es

#extension GL_GOOGLE_include_directive : enable

// debugging defines
// turn off DEBUG to see full screen rendering
#define DEBUG
// #define DEBUG_BLUE_OFF
// #define DISABLED
#define VERTIAL_SPLIT

#include "constants.h"

layout(input_attachment_index = 0, set = 0, binding = 0) uniform highp subpassInput in_color;
layout(set=0, binding = 1) uniform highp sampler2D in_texture_sampler;

layout(location = 0) in highp vec2 in_uv;

struct ResolutionData
{
    highp vec4 screen_resolution;
    highp vec4 editor_screen_resolution;
};

layout(push_constant) uniform constants
{
	ResolutionData resolution_data;
};

layout(location = 0) out highp vec4 out_color;

highp vec2 get_screen_uv(highp vec2 uv)
{
    highp vec4 screen_resolution = resolution_data.screen_resolution;
    highp vec4 editor_screen_resolution = resolution_data.editor_screen_resolution;

    highp vec2 editor_ratio = editor_screen_resolution.zw / screen_resolution.zw;
    highp vec2 offset = editor_screen_resolution.xy / screen_resolution.zw;

    return offset.xy + uv.xy * editor_ratio;
}

// Created by Reinder Nijhoff 2016
// Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// @reindernijhoff
//
// https://www.shadertoy.com/view/ls3GWS
#define FXAA_SPAN_MAX 8.0
#define FXAA_REDUCE_MUL   (1.0/FXAA_SPAN_MAX)
#define FXAA_REDUCE_MIN   (1.0/128.0)
#define FXAA_SUBPIX_SHIFT (1.0/4.0)

highp vec3 FxaaPixelShader( highp vec4 uv, sampler2D tex, highp vec2 rcpFrame) {

    highp vec3 rgbNW = textureLod(tex, uv.zw, 0.0).xyz;
    highp vec3 rgbNE = textureLod(tex, uv.zw + vec2(1,0)*rcpFrame.xy, 0.0).xyz;
    highp vec3 rgbSW = textureLod(tex, uv.zw + vec2(0,1)*rcpFrame.xy, 0.0).xyz;
    highp vec3 rgbSE = textureLod(tex, uv.zw + vec2(1,1)*rcpFrame.xy, 0.0).xyz;
    highp vec3 rgbM  = textureLod(tex, uv.xy, 0.0).xyz;

    highp vec3 luma = vec3(0.299, 0.587, 0.114);
    highp float lumaNW = dot(rgbNW, luma);
    highp float lumaNE = dot(rgbNE, luma);
    highp float lumaSW = dot(rgbSW, luma);
    highp float lumaSE = dot(rgbSE, luma);
    highp float lumaM  = dot(rgbM,  luma);

    highp float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    highp float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

    highp vec2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));

    highp float dirReduce = max(
        (lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL),
        FXAA_REDUCE_MIN);
    highp float rcpDirMin = 1.0/(min(abs(dir.x), abs(dir.y)) + dirReduce);

    dir = min(vec2( FXAA_SPAN_MAX,  FXAA_SPAN_MAX),
          max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX),
          dir * rcpDirMin)) * rcpFrame.xy;

    highp vec3 rgbA = (1.0/2.0) * (
        textureLod(tex, uv.xy + dir * (1.0/3.0 - 0.5), 0.0).xyz +
        textureLod(tex, uv.xy + dir * (2.0/3.0 - 0.5), 0.0).xyz);
    highp vec3 rgbB = rgbA * (1.0/2.0) + (1.0/4.0) * (
        textureLod(tex, uv.xy + dir * (0.0/3.0 - 0.5), 0.0).xyz +
        textureLod(tex, uv.xy + dir * (3.0/3.0 - 0.5), 0.0).xyz);

    highp float lumaB = dot(rgbB, luma);

    if((lumaB < lumaMin) || (lumaB > lumaMax)) return rgbA;

    return rgbB;
}

highp vec4 apply() {
    highp vec2 uv = get_screen_uv(in_uv);

    highp vec2 rcpFrame = 1.0 / resolution_data.screen_resolution.zw;
    highp vec4 uv4 = vec4( uv, uv - (rcpFrame * (0.5 + FXAA_SUBPIX_SHIFT)));

    highp vec3 temp_color = FxaaPixelShader(uv4, in_texture_sampler, rcpFrame);

    // as game display, no need to care color.a, else use color.a
#ifdef DEBUG_BLUE_OFF
    return vec4(temp_color.rg, 0.0, 1.0);
#else
    return vec4(temp_color.rgb, 1.0);
#endif
}


void main() {

    highp vec4 color = subpassLoad(in_color).rgba;
#ifdef DISABLED
    out_color = color;
#else
    // out_color = color;
#ifdef DEBUG
#ifdef VERTIAL_SPLIT
    if (in_uv.x > 0.5) {
        if (in_uv.x <= 0.505) {
            // visually a line
            out_color = vec4(0,0,0,0);
        } else {
            out_color = color;
        }
#else
    if (in_uv.x > in_uv.y) {
        out_color = color;
#endif
    } else {
#endif
    // shader out_color as following:

    out_color = apply();
    // out_color = vec4(color.r, color.g, 0.1, 0.1);
#ifdef DEBUG
    }
#endif
#endif
}
